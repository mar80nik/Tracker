// ImageWnd.cpp : implementation file
//

#include "stdafx.h"
#include "KSVU3.h"
#include "ImageWnd.h"
#include "TchartSeries.h"
#include "MainFrm.h"
#include "dcm800.h"
#include "metricon.h"
#include "my_color.h"
#include "captureWnd.h"
#include "BMPanvas.h"
#include "compressor.h"
IMPLEMENT_DYNAMIC(ImageWnd, CWnd)
ImageWnd::ImageWnd()
{
}

ImageWnd::~ImageWnd() {}


BEGIN_MESSAGE_MAP(ImageWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

// ImageWnd message handlers

int ImageWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect tr; GetClientRect(&tr); 
	CRect CameraOutWnd; CameraOutWnd.UnionRect(CameraWnd.CameraOutWnd, CameraWnd.LevelsScanWnd);
	fiber.Create(0,"FIBER",WS_CHILD | WS_VISIBLE | WS_CAPTION,CRect(CPoint(0,0),CSize(600, 600)),this,111,0);
	CameraWnd.Create(0, "CameraWnd", WS_CHILD | WS_BORDER | WS_VISIBLE, 
		CRect(CPoint(tr.Width() - CameraOutWnd.Width(),0), CameraOutWnd.Size()), 
		this, ID_MV_WND, 0);	
	OnChildMove();
	CameraWnd.Ctrls.Create(IDD_DIALOGBARTAB4,&Ctrls); 
	CameraWnd.Ctrls.SetWindowPos(NULL,500,0,0,0,SWP_NOSIZE | SWP_NOZORDER);
	CameraWnd.Ctrls.ShowWindow(SW_SHOW);
#define TEST1
#ifdef DEBUG
 	fiber.LoadPic(CString("..\\exe\\fiber.png"));
#endif
	Ctrls.Parent=this;
	SetMarker(OrgPoint(CPoint(282, 243)), BGN);
	SetMarker(OrgPoint(CPoint(507, 784)), END);

	return 0;
}

void ImageWnd::OnDestroy()
{
	CameraWnd.DestroyWindow();
	CameraWnd.Ctrls.DestroyWindow();
	Ctrls.DestroyWindow();
	CWnd::OnDestroy();

}

typedef CArray<HWND> HWNDArray;

BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
{
	HWNDArray &childs=*((HWNDArray*)lParam); 
	HWND parent=childs[1], hwndParent=GetParent(hwnd);
	int n=(int)(childs[0]);
	CWnd* wnd=CWnd::FromHandle(hwnd);
	CString T; wnd->GetWindowText(T);
	if( hwndParent==parent ) childs.Add(hwnd); 
	if((childs.GetCount()-2)==n) return 0;
	else return 1;
}

void ImageWnd::OnChildMove()
{
	int n=GetWindowedChildCount();
	HWNDArray childs; childs.Add((HWND)n); childs.Add(m_hWnd);
	CRect result,tr,cr; GetClientRect(&cr);

	while(EnumChildWindows(m_hWnd, EnumChildProc,(LPARAM)&childs));
	int i=2; CWnd* twnd=NULL;
	if( (childs.GetCount()-2)>0 )
	{
		twnd=CWnd::FromHandle(childs[i++]);
		twnd->GetWindowRect(&tr); result.UnionRect(tr,tr);
		for(;i<childs.GetCount();i++)
		{
			
			twnd=CWnd::FromHandle(childs[i]);
			CString T; twnd->GetWindowText(T);
			twnd->GetWindowRect(&tr); 
			result.UnionRect(result,tr);
		}
	}
	CPoint tl=result.TopLeft(), br=result.BottomRight();
	ScreenToClient(&tl); ScreenToClient(&br);
	result=CRect(tl,br);
	if(cr.top!=result.top || cr.bottom!=result.bottom || cr.left!=result.left || cr.right!=result.right)
	{
		SetWindowPos(NULL,result.left,result.top,result.Width(),result.Height(), SWP_NOZORDER );
	}
}

//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(ImageWnd::CtrlsTab, BarTemplate)
	//{{AFX_MSG_MAP(DialogBarTab1)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedScan)
	ON_EN_KILLFOCUS(IDC_EDIT1, &ImageWnd::CtrlsTab::OnEnKillfocusEdit1)	
	ON_MESSAGE(UM_BUTTON_ITERCEPTED,&ImageWnd::CtrlsTab::OnButtonIntercepted)
	ON_BN_CLICKED(IDC_BUTTON5, &ImageWnd::CtrlsTab::OnBnClickedButton5)	
	//}}AFX_MSG_MAP	
END_MESSAGE_MAP()

BOOL ImageWnd::CtrlsTab::OnInitDialog() 
{
	CDialog::OnInitDialog();
	NofScans.SetCurSel(1);
	return TRUE; 	
}

ImageWnd::CtrlsTab::CtrlsTab( CWnd* pParent /*= NULL*/ ): BarTemplate(pParent),
#if defined DEBUG
	stroka(220), Xmin(100), Xmax(6000)
#else
	stroka(1224), Xmin(2), Xmax(3263)
#endif

{
}

void ImageWnd::CtrlsTab::DoDataExchange(CDataExchange* pDX)
{
	BarTemplate::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogBarTab1)
	DDX_Text(pDX, IDC_EDIT1, stroka);
	DDX_Text(pDX, IDC_EDIT3, Xmin);
	DDX_Text(pDX, IDC_EDIT4, Xmax);
	DDX_Control(pDX, IDC_EDIT3, XminCtrl);
	DDX_Control(pDX, IDC_EDIT4, XmaxCtrl);
	DDX_Control(pDX, IDC_EDIT1, strokaCtrl);
	DDX_Control(pDX, IDC_COMBO1, NofScans);
	//}}AFX_DATA_MAP

}

void Interpolate(CPoint Beg, CPoint End, double dr, BYTE *arr, int wbyte, SimplePointArray* Points)
{
	int x1, y1, z1, z2, z3; double z, r, cosfi, sinfi, dx, dy; int A, B, D, N; 
	SimplePoint pnt; pnt.type.Set(GenericPnt); 
	double rmax = sqrt((double)(End.x - Beg.x)*(End.x - Beg.x) + (End.y - Beg.y)*(End.y - Beg.y));
	cosfi = (End.x - Beg.x)/rmax; sinfi = (End.y - Beg.y)/rmax;
	N = (int)(1 + rmax/dr); dx = dr*cosfi; dy = dr*sinfi; r = 0.; pnt.x = Beg.x; pnt.y = Beg.y;

	for (int i = 0; i < N; i++)
	{
		x1 = (int)pnt.x; y1 = (int)pnt.y; 
		z1 = arr[y1*wbyte + x1]; z3 = arr[(y1 + 1)*wbyte + (x1 + 1)];
		if ((double)(pnt.x - x1 + pnt.y - y1) > 1.)
		{
			z2 = arr[y1*wbyte + (x1 + 1)];
			A = z1 - z2; B = z2 - z3;
		}
		else
		{
			z2 = arr[(y1 + 1)*wbyte + x1];
			A = z2 - z3; B = z1 - z2;
		}
		D = -z1 - x1*A - y1*B; z = -D - A*pnt.x - B*pnt.y;
		Points->Add(pnt);	
		r += dr; pnt.x += dx; pnt.y += dy;
	}
}

void ImageWnd::CtrlsTab::OnBnClickedScan()
{
	/*UpdateData(); ImageWnd* parent=(ImageWnd*)Parent; void* x; CString T,T1;
	MyTimer Timer1,Timer2; sec time; 
	void* x; CString T; BOOL exit = FALSE;		
	ConrtoledLogMessage log; log << _T("Speed tests results");
	ImagesAccumulator &dark = parent->dark.accum, &cupol = parent->cupol.accum, &strips = parent->strips.accum;

	if (dark.bmp == NULL) 	{log << _T("There is no DARK image"); exit = TRUE;}
	CMainFrame* MainWnd=(CMainFrame*)AfxGetMainWnd(); 
	MainWnd->TabCtrl1.ChangeTab(MainWnd->TabCtrl1.FindTab("Main control"));	

	if(!(parent->fiber.org.HasImage())) return;

			PointVsErrorArray dark_points, cupol_points, strips_points, result; Timer1.Start();
			for (int i = 0; i < dark_points.GetSize(); i++)
			int diff;
			if ((diff = dark_points.GetSize() - result.GetSize()) != 0)
			{
				log.T.Format("%d points were excluded because of division error", diff); log << log.T;
				log.SetPriority(lmprHIGH);
			if (result.GetSize() != 0)
			{
				CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); TChart& chrt=mf->Chart1; 			
				if((x = chrt.Series.GainAcsess(WRITE))!=0)
				{
					SeriesProtector Protector(x); TSeriesArray& Series(Protector);
		if((t2=new TSimplePointSeries(T))!=0)	
					if((t2 = new TPointVsErrorSeries(T)) != NULL)	
					{
						t2->_SymbolStyle::Set(NO_SYMBOL); t2->_ErrorBarStyle::Set(POINTvsERROR_BAR);				
						t2->PointType.Set(GenericPnt); 										
						for(int i=0;i<Series.GetSize();i++) Series[i]->SetStatus(SER_INACTIVE);
						Series.Add(t2); 
			ChartMsg = t2->CreateDataImportMsg(); 
						t2->AssignColors(ColorsStyle(clRED,Series.GetRandomColor()));
						t2->SetStatus(SER_ACTIVE); t2->SetVisible(true);
						TPointVsErrorSeries::DataImportMsg *ChartMsg = t2->CreateDataImportMsg(); 
						for (int i = 0; i < result.GetSize(); i++) 
						{
							ChartMsg->Points.Add(result[i]);
						}
						ChartMsg->Dispatch(); 
						mf->TabCtrl1.ChangeTab(mf->TabCtrl1.FindTab("Main control"));	
					}		

	Xmin = parent->MarkerBGN.x - CrossWidth; Xmax = parent->MarkerBGN.x + CrossWidth;
	Ymin = parent->MarkerBGN.y - CrossWidth; Ymax = parent->MarkerBGN.y + CrossWidth;
    Timer1.Start();	parent->fiber.org.LoadBitmapArray(); 
	Timer1.Stop(); time=Timer1.GetValue();
	logT.Format("D C S %d lines load = %s", Xmax - Xmin + 1,ConvTimeToStr(time)); log->Msgs.Add(logT);
	fiber = parent->fiber.org.arr;

	Interpolate(CPoint(parent->MarkerBGN.x, parent->MarkerBGN.y - CrossWidth), 
				CPoint(parent->MarkerBGN.x, parent->MarkerBGN.y + CrossWidth), 1.0, 
				parent->fiber.org.arr, parent->fiber.org.wbyte, &ChartMsg->Points);
	Timer1.Stop(); 
	logT.Format("Interpolating %d lines time=%s", ChartMsg->Points.GetSize(), ConvTimeToStr(Timer1.GetValue())); log->Msgs.Add(logT);
	ChartMsg->Dispatch();

	if((x=chrt.Series.GainAcsess(WRITE))!=0)
		SeriesProtector Protector(x); TSeriesArray& Series(Protector);
		if((t2=new TSimplePointSeries(T))!=0)	
			for(int i=0;i<Series.GetSize();i++) Series[i]->SetStatus(SER_INACTIVE);
			Series.Add(t2); 
			t2->_SymbolStyle::Set(NO_SYMBOL); 
			ChartMsg = t2->CreateDataImportMsg(); 
			t2->AssignColors(ColorsStyle(clRED,Series.GetRandomColor()));
			t2->PointType.Set(GenericPnt); 
			t2->SetStatus(SER_ACTIVE); t2->SetVisible(true);
		}		
			}

	Interpolate(CPoint(parent->MarkerBGN.x - CrossWidth, parent->MarkerBGN.y), 
		CPoint(parent->MarkerBGN.x + CrossWidth, parent->MarkerBGN.y), 1.0, 
		parent->fiber.org.arr, parent->fiber.org.wbyte, &ChartMsg->Points);
	Timer1.Stop(); 
	logT.Format("Interpolating %d lines time=%s", ChartMsg->Points.GetSize(), ConvTimeToStr(Timer1.GetValue())); log->Msgs.Add(logT);
	parent->fiber.org.UnloadBitmapArray();*/
}

ImageWnd::PicWnd::PicWnd()
{
	Parent=0;
}

ImageWnd::PicWnd::~PicWnd()
{
	POSITION pos = helpers.GetHeadPosition();
	while ( pos != NULL)
	{
		delete helpers.GetNext(pos);
	}
	helpers.RemoveAll();
}
BEGIN_MESSAGE_MAP(ImageWnd::PicWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_DROPFILES()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_COMMAND(CaptureBtnID, OnCaptureButton)
	ON_MESSAGE(UM_CAPTURE_READY,OnCaptureReady)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_PICWNDMENU_ERASE, OnPicWndErase)
	ON_COMMAND(ID_PICWNDMENU_SAVE, OnPicWndSave)
	ON_COMMAND(ID_PICWNDMENU_SCANLINE, OnPicWndScanLine)
	ON_WM_MOVE()
END_MESSAGE_MAP()

int ImageWnd::PicWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
    Parent=(ImageWnd*)GetParent();
	CRect r; GetClientRect(&r);
	ava.Create(this,r.Width(),r.Height(),24); 
	EraseAva();
	DragAcceptFiles(true);
	font1.CreatePointFont(80,"Arial"); 
	CRect t(CPoint(0,0),CSize(150,20)); t.OffsetRect(r.CenterPoint()-t.CenterPoint());
	CString T; GetWindowText(T); T="Capture "+T+" image";
	CaptureButton.Create(T,BS_CENTER | BS_TEXT | WS_VISIBLE | BS_PUSHBUTTON, t,this,CaptureBtnID);
	menu1.LoadMenu(IDR_MENU3);
	return 0;
}

void ImageWnd::PicWnd::EraseAva()
{
	CString T,T1;  GetWindowText(T1); T.Format("Drag or Capture %s image",T1);
	CRect r; GetClientRect(&r); 
	ava.PatBlt(WHITENESS); ava.Rectangle(r);
	ava.TextOut(10,10,T);
}

void ImageWnd::PicWnd::OnPaint()
{
	CPaintDC dc(this); 	
	HDC hdc=dc.GetSafeHdc();
	if (accum.bmp != NULL)
	{
		if(accum.bmp->HasImage())
		{
		MarkerAvaBGN.Draw(&ava);
		MarkerAvaEND.Draw(&ava);			
		}
	}
	ava.CopyTo(hdc,TOP_LEFT);
}

void ImageWnd::PicWnd::AvaMarker::Draw( BMPanvas* canvas )
{
	Draw( canvas, *this, DRAW ); 
}

void ImageWnd::PicWnd::AvaMarker::Erase( BMPanvas * canvas )
{
	if (canvas == NULL)
	{
		ToErase = FALSE;
	}
	else
	{
		if ( ToErase == TRUE ) Draw( canvas, last, ERASE );
		ToErase = FALSE;
	}
}

void ImageWnd::PicWnd::AvaMarker::Draw( BMPanvas* canvas, const AvaPoint& pnt, DrawModes mode )
{
	int lastMode;
	if ( canvas == NULL) return;
	switch ( mode)
	{
	case DRAW: Erase( canvas ); lastMode = canvas->SetROP2(R2_NOT); last = *this; ToErase=TRUE; break;
	case ERASE: 
	default:		
		lastMode = canvas->SetROP2(R2_NOT); 
	}	
	canvas->MoveTo(pnt.x - CrossWidth, pnt.y); canvas->LineTo(pnt.x + CrossWidth, pnt.y); 
	canvas->MoveTo(pnt.x, pnt.y - CrossWidth); canvas->LineTo(pnt.x, pnt.y + CrossWidth); 
	canvas->SetROP2(lastMode);
}

void ImageWnd::PicWnd::UpdateNow(void)
{
	RedrawWindow(0,0,RDW_INVALIDATE | RDW_FRAME | RDW_NOERASE | RDW_ALLCHILDREN);					
}

HRESULT ImageWnd::PicWnd::LoadPic(CString T)
{	
	HRESULT ret;
	if(SUCCEEDED(ret = accum.LoadFrom(T)))
	{
		accum.ConvertToBitmap(this); BMPanvas &org = *(accum.bmp);

		//Parent->CameraWnd.Ctrls.UpdateData(); 
		//if (Parent->CameraWnd.Ctrls.ColorTransformSelector == CaptureWnd::CtrlsTab::TrueColor)
		//{
		//	ConrtoledLogMessage log(MessagePriorities::lmprHIGH); 
		//	log << _T("ERR: Image you are trying to load is no GRAYSCALE.");
		//	log << _T("ERR: In order to use bult-in convertor please select");			
		//	log << _T("ERR: convert method: NativeGDI, HSL or HSV.");			
		//	log.Dispatch();
		//	return E_FAIL;
		//}

		//if (accum.bmp->ColorType != BMPanvas::GRAY_PAL) ConvertOrgToGrayscale();
		FileName=T;
		EraseAva(); MakeAva();
        HGDIOBJ tfont=ava.SelectObject(font1); ava.SetBkMode(TRANSPARENT); ava.SetTextColor(clRED);
		ava.TextOut(0,0,T);
		T.Format("%dx%d", org.w, org.h); ava.TextOut(0,10,T);
		ava.SelectObject(tfont); 
		CaptureButton.ShowWindow(SW_HIDE); DragAcceptFiles(FALSE);
		UpdateNow();
		Parent->Ctrls.Xmax=org.w; Parent->Ctrls.UpdateData();
	}
	else FileName="";		 
	return ret;
}

void ImageWnd::PicWnd::OnDropFiles(HDROP hDropInfo)
{
	char buf[1000]; CString T,T2;  GetWindowText(T2);
	DragQueryFile(hDropInfo,0xFFFFFFFF,buf,1000);
	DragQueryFile(hDropInfo,0,buf,1000); T=CString(buf); 
	MyTimer Timer1; sec time; 	
	
	Timer1.Start(); 
	if (SUCCEEDED(LoadPic(T)))
	{
		BMPanvas &org = *(accum.bmp); ConrtoledLogMessage log;
		//Parent->SetScanRgn(Parent->GetScanRgn());
		Timer1.Stop(); 
		time=Timer1.GetValue(); 
		log.T.Format("%s org (%.2f Mpix) load time=%s", T2, org.w*org.h/1e6, ConvTimeToStr(time)); log << log.T;		
		log.Dispatch();
	}	
	CWnd::OnDropFiles(hDropInfo);
}

void ImageWnd::PicWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	BOOL update = FALSE; 
	if (accum.bmp == NULL) return;
	if (accum.bmp->HasImage() == FALSE ) return;
	switch( nFlags )
	{
	case MK_SHIFT: Parent->SetMarker(Convert(AvaPoint(point)), BGN); update = TRUE; break;
	case MK_CONTROL: Parent->SetMarker(Convert(AvaPoint(point)), END); update = TRUE; break;
	case 0: update=TRUE; break;	
	}
	if (update == TRUE)
	{
		UpdateNow();
		{
			//Parent->SetScanRgn( Convert(tmpRgn) );
		}		
	}
	CWnd::OnLButtonUp(nFlags, point);
}

void ImageWnd::PicWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
	BOOL update = FALSE; 
	if (accum.bmp == NULL) return;
	if (accum.bmp->HasImage() == FALSE ) return;
	switch( nFlags )
	{
	case MK_SHIFT: update=TRUE; break;
	default: ;
	}
	if (update)
	{
	}
	else
	{
		CWnd::OnRButtonUp(nFlags, point);
	}	
}

void ImageWnd::PicWnd::OnCaptureButton()
{
	UpdateHelpers(EvntOnCaptureButton);
}

LRESULT ImageWnd::PicWnd::OnCaptureReady( WPARAM wParam, LPARAM lParam )
{
	UpdateHelpers(EvntOnCaptureReady);
	return 0;
}
void ImageWnd::PicWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (accum.bmp != NULL)
	{
		if (accum.bmp->HasImage())
		{
			CMenu* menu = menu1.GetSubMenu(0);
			menu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
		}
	}
}

void ImageWnd::PicWnd::OnPicWndErase()
{
	accum.Reset(); EraseAva();
	CaptureButton.EnableWindow(TRUE); CaptureButton.ShowWindow(SW_SHOW); DragAcceptFiles(true);
	UpdateNow();	
}

void ImageWnd::PicWnd::OnPicWndSave()
{
	if (accum.bmp != NULL)
	{
		if(accum.bmp->HasImage())
		{
			CFileDialog dlg1(FALSE,"pack");
			if(dlg1.DoModal()==IDOK)
			{
				accum.SaveTo(dlg1.GetPathName());
			}
		}
	}
}

int ImageWnd::CtrlsTab::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if (BarTemplate::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void ImageWnd::OnSize(UINT nType, int cx, int cy)
{
	CRect r,r1;
	CWnd::OnSize(nType, cx, cy);
	
	if (CameraWnd.m_hWnd!=NULL)
	{
		GetClientRect(&r); CameraWnd.GetWindowRect(&r1);
		CameraWnd.SetWindowPos(NULL,cx-r1.Width(),0,0,0,SWP_NOZORDER | SWP_NOSIZE);

		CScrollView* pFirstView=(CScrollView*)GetParent();
		pFirstView->SetScrollSizes(MM_TEXT, CSize(cx, cy));
	}
	
}

BOOL ImageWnd::OnEraseBkgnd(CDC* pDC)
{
	CRect r; GetClientRect(&r);
	pDC->PatBlt(0,0,r.Width(),r.Height(),WHITENESS);

	return CWnd::OnEraseBkgnd(pDC);
}

void ImageWnd::SetMarker( const OrgPoint& mark, MarkerNames pos )
{
	if (fiber.accum.GetSum() != NULL)
	{
		switch (pos)
		{
		case BGN: MarkerBGN = fiber.ValidatePnt(mark); fiber.SetMarker(fiber.Convert(MarkerBGN), BGN); break;
		case END: MarkerEND = fiber.ValidatePnt(mark); fiber.SetMarker(fiber.Convert(MarkerEND), END);break;
		}
	}
}

void ImageWnd::PicWnd::OnMove(int x, int y)
{	
	CRect r,r1,r2; GetWindowRect(&r);
	CPoint tl=r.TopLeft(), br=r.BottomRight(); 
	Parent->ScreenToClient(&tl); Parent->ScreenToClient(&br);	
	r2=CRect(tl,br);
	CPoint pnt=r2.TopLeft(),shift(0,0);

	if(pnt.x<0 || pnt.y<0) 
	{
		if(pnt.x<0) shift.x=-pnt.x;
		if(pnt.y<0) shift.y=-pnt.y;
		r2.OffsetRect(shift);
		SetWindowPos(NULL,r2.left,r2.top,0,0,SWP_NOZORDER | SWP_NOSIZE);
		return;
	}		
	CWnd::OnMove(x, y);	
	RedrawWindow(0,0,RDW_FRAME | RDW_INVALIDATE);
	Parent->OnChildMove();
}

void ImageWnd::PicWnd::ConvertOrgToGrayscale()
{
	if (accum.bmp != NULL)
	{
		BMPanvas temp_replica; BMPanvas &org = *(accum.bmp);
		temp_replica.Create(&org, org.Rgn); org.CopyTo(&temp_replica, TOP_LEFT);
		org.Destroy(); org.Create(this,temp_replica.w,temp_replica.h,8);
		org.CreateGrayPallete(); Parent->CameraWnd.Ctrls.UpdateData();			
		ColorTransform(&temp_replica, &org, Parent->CameraWnd.Ctrls.ColorTransformSelector);
	}
}

void ImageWnd::PicWnd::SetMarker( const AvaPoint& mark, MarkerNames pos )
{
	if (accum.bmp != NULL)
	{
		switch (pos)
		{
		case BGN: MarkerAvaBGN = ValidatePnt(mark); break;
		case END: MarkerAvaEND = ValidatePnt(mark); break;
		}
	}	
}

ImageWnd::OrgPoint ImageWnd::PicWnd::ValidatePnt( const OrgPoint& pnt )
{
	OrgPoint ret = pnt; 
	if (accum.bmp != NULL)
	{
		if (ret.x < 0) ret.x = 0;
		if (ret.x >= accum.w)	 ret.x = accum.w - 1;
		if (ret.y < 0) ret.y = 0;
		if (ret.y >= accum.h)	 ret.y = accum.h - 1;
	}
	return ret;	
}

ImageWnd::OrgPoint ImageWnd::PicWnd::Convert( const AvaPoint& pnt)
{
	OrgPoint ret; 
	if (accum.bmp != NULL)
	{
		BMPanvas &org = *accum.bmp;
		if ( ava.HasImage() )
		{
			struct {double cx, cy;} scale = {(double)org.Rgn.Width() / ava.Rgn.Width(), (double)org.Rgn.Height() / ava.Rgn.Height()}; 
		ret.x = (LONG)(pnt.x * scale.cx); ret.y = (LONG)(pnt.y * scale.cy);
		}
	}	
	return ret;
}

ImageWnd::AvaPoint ImageWnd::PicWnd::Convert( const OrgPoint& pnt)
{
	AvaPoint ret; 
	if (accum.bmp != NULL)
	{
		BMPanvas &org = *accum.bmp;
		if ( org.HasImage() )
		{
			struct {double cx, cy;} scale = {(double)ava.Rgn.Width() / org.Rgn.Width(), (double)ava.Rgn.Height() / org.Rgn.Height()}; 
		ret.x = (LONG)(pnt.x * scale.cx); ret.y = (LONG)(pnt.y * scale.cy); 
		}
	}
	return ret;
}

void ImageWnd::PicWnd::UpdateHelpers( const HelperEvent &event )
{
	BaseForHelper * accumCapture = NULL;
	ImageWnd::CtrlsTab &ctrls = Parent->Ctrls; ctrls.UpdateData();
	switch (event)
	{
	case EvntOnCaptureButton:
		this->accum.Reset();
		accumCapture = new AccumHelper(this, ctrls.GetNofScans());
		helpers.AddTail(accumCapture);
		break;
	default:
		POSITION pos = helpers.GetHeadPosition();
		while ( pos != NULL)
		{
			POSITION prev = pos;
			BaseForHelper* helper = helpers.GetNext(pos); 
			if (helper->Update(event) == RSLT_HELPER_COMPLETE)
			{
				delete helper; helpers.RemoveAt(prev);
			}			
		}
	}
}

HRESULT ImageWnd::PicWnd::MakeAva()
{
	if (accum.bmp != NULL)
	{
		BMPanvas &org = *(accum.bmp);
		SetStretchBltMode(ava.GetDC(),COLORONCOLOR);		
		org.StretchTo(&ava, ava.Rgn, org.Rgn,SRCCOPY);
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

void ImageWnd::PicWnd::OnPicWndScanLine()
{
	//void* x; CString T; 	
	//TPointVsErrorSeries::DataImportMsg *ChartMsg = NULL; 
	//ConrtoledLogMessage log; log << _T("Speed tests results");

	//if (accum.bmp != NULL)
	//{
	//	CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); TChart& chrt=mf->Chart1; 
	//	mf->TabCtrl1.ChangeTab(mf->TabCtrl1.FindTab("Main control"));	

	//	OrgPicRgn scan_rgn = Parent->ScanRgn; CPoint cntr = scan_rgn.CenterPoint();
	//	T.Format("Scan line y = %d N = %d", cntr.y, accum.n);

	//	if((x=chrt.Series.GainAcsess(WRITE))!=0)
	//	{
	//		SeriesProtector Protector(x); TSeriesArray& Series(Protector);
	//		TPointVsErrorSeries *t2;
	//		if((t2 = new TPointVsErrorSeries(T))!=0)	
	//		{
	//			for(int i=0;i<Series.GetSize();i++) Series[i]->SetStatus(SER_INACTIVE);
	//			Series.Add(t2); 
	//			t2->_SymbolStyle::Set(NO_SYMBOL); t2->_ErrorBarStyle::Set(POINTvsERROR_BAR);
	//			ChartMsg = t2->CreateDataImportMsg(); 
	//			t2->AssignColors(ColorsStyle(clRED,Series.GetRandomColor()));
	//			t2->PointType.Set(GenericPnt); 
	//			t2->SetStatus(SER_ACTIVE); t2->SetVisible(true);
	//		}		
	//	}
	//	
	//	accum.ScanLine(&(ChartMsg->Points), cntr.y, scan_rgn.left, scan_rgn.right);
	//	log.T.Format("Scan of %d points took %g ms", ChartMsg->Points.GetSize(), accum.fillTime.val()); 
	//	log << log.T;
	//	ChartMsg->Dispatch();
	//	log.Dispatch();
	//}
}


HelperEvent AccumHelper::Update( const HelperEvent &event )
{
	ImagesAccumulator &accum = parent->accum; CString T;
	switch (event)
	{
	case EvntOnCaptureReady:
		if (accum.n < n_max)
		{
			if (SUCCEEDED(accum.FillAccum(tmp_bmp)))
			{
				accum.ConvertToBitmap(parent);
				//parent->Parent->SetScanRgn(parent->Parent->GetScanRgn());
				
				parent->MakeAva();
				//parent->ScanRgn.Erase(NULL);

				HGDIOBJ tfont = parent->ava.SelectObject(parent->font1);
				parent->ava.SetBkMode(TRANSPARENT); COLORREF old_color = parent->ava.SetTextColor(clRED);
				T.Format("Camera capture %d of %d", accum.n, n_max); parent->ava.TextOut(0,0,T);
				T.Format("%dx%d", accum.w, accum.h); parent->ava.TextOut(0,10,T);
				parent->ava.SelectObject(tfont); parent->ava.SetTextColor(old_color);
				parent->UpdateNow();				

				if (accum.n == n_max)
				{
					return RSLT_HELPER_COMPLETE;					
				}
				else
				{
					parent->Parent->CameraWnd.PostMessage(UM_CAPTURE_REQUEST,(WPARAM)parent, (LPARAM)tmp_bmp);			
				}
			}		
		}
		else ASSERT(0);
		break;
	}
	return RSLT_OK;
}

AccumHelper::AccumHelper( ImageWnd::PicWnd *_parent, const int _n_max ) : parent(_parent), n_max(_n_max)
{
	tmp_bmp = NULL; tmp_bmp = new BMPanvas();
	parent->CaptureButton.EnableWindow(FALSE);
	parent->Parent->CameraWnd.PostMessage(UM_CAPTURE_REQUEST,(WPARAM)parent, (LPARAM)tmp_bmp);
	parent->CaptureButton.ShowWindow(SW_HIDE); parent->DragAcceptFiles(FALSE);
}

AccumHelper::~AccumHelper()
{	
	if (tmp_bmp != NULL)
	{
		delete tmp_bmp; tmp_bmp = NULL;
	}
	BaseForHelper::~BaseForHelper();
}


BOOL ImageWnd::CtrlsTab::DestroyWindow()
{
	return BarTemplate::DestroyWindow();
}

void ImageWnd::CtrlsTab::OnBnClickedButton5()
{
	ImageWnd *parent=(ImageWnd*)Parent;
	parent->fiber.PostMessage(WM_COMMAND,ID_PICWNDMENU_ERASE,0);
}

void ImageWnd::CtrlsTab::OnEnKillfocusEdit1() {}

LRESULT ImageWnd::CtrlsTab::OnButtonIntercepted( WPARAM wParam, LPARAM lParam )
{
	UpdateData(); ImageWnd* parent=(ImageWnd*)Parent;
	return NULL;
}

int ImageWnd::CtrlsTab::GetNofScans()
{
	CString text;
	NofScans.GetLBText(NofScans.GetCurSel(),text);
	return atoi(text);
}

IMPLEMENT_DYNAMIC(CEditInterceptor, CEdit)

BEGIN_MESSAGE_MAP(CEditInterceptor, CEdit)
	ON_WM_CHAR()
END_MESSAGE_MAP()


void CEditInterceptor::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == 13)
	{
		CWnd * parent = GetParent();
		parent->PostMessage(UM_BUTTON_ITERCEPTED, nChar, 0);		
	}
	else
	{
		CEdit::OnChar(nChar, nRepCnt, nFlags);
	}
}

