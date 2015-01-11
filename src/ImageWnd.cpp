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
//	fiber.LoadPic(CString("..\\exe\\strips.png"));
 	fiber.LoadPic(CString("..\\exe\\fiber.png"));
#endif
	Ctrls.Parent=this;
	SetMarker(CPoint(282, 243), BGN);
	SetMarker(CPoint(507, 784), END);

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
	ON_COMMAND(ID_PICWNDMENU_SCANLINE, OnPicWndScanArbitaryLine)
	ON_COMMAND(ID_PICWNDMENU_MULTICROSS, OnPicWndMultiCross)

	
	ON_WM_MOVE()
END_MESSAGE_MAP()

HRESULT ImageWnd::PicWnd::ConvertAvaToOrg( CPoint& pnt ) const
{
	CRect AvaRgn, OrgRgn; GetClientRect(&AvaRgn);
	if (SUCCEEDED(accum.GetPicRgn(OrgRgn)))
	{
		CSize Ava = AvaRgn.Size(), Org = OrgRgn.Size();
		pnt.x = (LONG)((double)pnt.x*Org.cx/Ava.cx); 
		pnt.y = (LONG)((double)pnt.y*Org.cy/Ava.cy);
		return S_OK;
	}
	return E_FAIL;
}

HRESULT ImageWnd::PicWnd::ConvertOrgToAva( CPoint& pnt ) const
{
	CRect AvaRgn, OrgRgn; GetClientRect(&AvaRgn);
	if (SUCCEEDED(accum.GetPicRgn(OrgRgn)))
	{
		CSize Ava = AvaRgn.Size(), Org = OrgRgn.Size();
		pnt.x = (LONG)((double)pnt.x*Ava.cx/Org.cx); 
		pnt.y = (LONG)((double)pnt.y*Ava.cy/Org.cy); 
		return S_OK;
	}
	return E_FAIL;
}

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
	CRect r; GetClientRect(&r); MarkerAvaBGN.Erase(NULL); MarkerAvaEND.Erase(NULL);
	ava.PatBlt(WHITENESS); ava.Rectangle(r);
	ava.TextOut(10,10,T);
}

void ImageWnd::PicWnd::OnPaint()
{
	CPaintDC dc(this); CPoint marker;
	HDC hdc=dc.GetSafeHdc();
 	if (accum.bmp != NULL)
	{
		if(accum.bmp->HasImage())
		{
			marker = Parent->MarkerBGN;
			if (SUCCEEDED(ConvertOrgToAva(marker)))
			{
				MarkerAvaBGN.Draw(&ava, marker);
			}
			marker = Parent->MarkerEND;
			if (SUCCEEDED(ConvertOrgToAva(marker)))
			{
				MarkerAvaEND.Draw(&ava, marker);
			}
		}
	}
	ava.CopyTo(hdc,TOP_LEFT);
}


void ImageWnd::PicWnd::AvaMarker::Draw( BMPanvas* canvas, const CPoint& pnt )
{
	Draw( canvas, pnt, DRAW ); 
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

void ImageWnd::PicWnd::AvaMarker::Draw( BMPanvas* canvas, const CPoint& pnt, DrawModes mode )
{
	int lastMode;
	if ( canvas == NULL) return;
	switch ( mode)
	{
	case DRAW: Erase( canvas ); lastMode = canvas->SetROP2(R2_NOT); last = pnt; ToErase=TRUE; break;
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

HRESULT ImageWnd::PicWnd::TryLoadBitmap(CString T, BMPanvas &bmp)
{
	HRESULT ret;
	if (SUCCEEDED(ret = bmp.LoadImage(T)))
	{
		Parent->CameraWnd.Ctrls.UpdateData();	
		CaptureWnd::CtrlsTab::ColorTransformModes ColorTransformModes = 
			Parent->CameraWnd.Ctrls.ColorTransformSelector;
		if (bmp.ColorType != BMPanvas::GRAY_PAL)
		{
			if (ColorTransformModes == CaptureWnd::CtrlsTab::ColorTransformModes::TrueColor)
			{
				ControledLogMessage log(lmprHIGH);
				log.T.Format("Error: Image you are trying to load which is no GRAYSCALE."); log << log.T;
				log.T.Format("*****: In order to use bult-in convertor please select"); log << log.T;
				log.T.Format("*****: convert method: NativeGDI, HSL or HSV."); log << log.T;
				log.Dispatch(); 
				return E_FAIL;
			}
			BMPanvas temp_replica; 
			temp_replica.Create(&bmp, bmp.Rgn); bmp.CopyTo(&temp_replica, TOP_LEFT);
			bmp.Destroy(); bmp.Create(this,temp_replica.w,temp_replica.h,8);
			bmp.CreateGrayPallete(); 
			ColorTransform(&temp_replica, &bmp, ColorTransformModes);
		}
	}
	return ret;
}

HRESULT ImageWnd::PicWnd::LoadPic(CString T)
{	
	HRESULT ret; ControledLogMessage log;
	if(FAILED(ret = accum.LoadFrom(T)))
	{
		BMPanvas org;
		if (FAILED(ret = TryLoadBitmap(T, org)))
		{
			log.T.Format("Failed to Load as BITMAP %s", T); 
			log << log.T; log.SetPriority(lmprHIGH);	
			return E_FAIL;
		}
		else
		{
			if (FAILED(ret = accum.FillAccum(&org)))
			{
				log.T.Format("Failed to INIT accumulator from %s", T); 
				log << log.T; log.SetPriority(lmprHIGH);	
				return E_FAIL;				
			}			
		}
	}

	accum.ConvertToBitmap(this); BMPanvas &org = *(accum.bmp);

	FileName=T;
	EraseAva(); MakeAva();
	HGDIOBJ tfont=ava.SelectObject(font1); ava.SetBkMode(TRANSPARENT); ava.SetTextColor(clRED);
	ava.TextOut(0,0,T);
	T.Format("%dx%d", org.w, org.h); ava.TextOut(0,10,T);
	ava.SelectObject(tfont); 
	CaptureButton.ShowWindow(SW_HIDE); DragAcceptFiles(FALSE);
	UpdateNow();
	Parent->Ctrls.Xmax=org.w; Parent->Ctrls.UpdateData();

	log.Dispatch();
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
		BMPanvas &org = *(accum.bmp); ControledLogMessage log;
		Timer1.Stop(); 
		time=Timer1.GetValue(); 
		log.T.Format("%s org (%.2f Mpix) load time=%s", T2, org.w*org.h/1e6, ConvTimeToStr(time)); log << log.T;		
		log.Dispatch();
	}	
	CWnd::OnDropFiles(hDropInfo);
}

void ImageWnd::PicWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	CPoint OrgPnt(point); 
	if (SUCCEEDED(ConvertAvaToOrg(OrgPnt)))
	{
		switch( nFlags )
		{
		case MK_SHIFT: Parent->SetMarker(OrgPnt, BGN); break;
		case MK_CONTROL: Parent->SetMarker(OrgPnt, END); break;
		case 0: break;	
		}
	}
	CWnd::OnLButtonUp(nFlags, point);
}

void ImageWnd::PicWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
	CWnd::OnRButtonUp(nFlags, point);
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

void ImageWnd::SetMarker( const CPoint& _mark, MarkerNames pos )
{
	CPoint mark = _mark;
	if (fiber.accum.HasImage())
	{
		if (SUCCEEDED(fiber.ValidatePnt(mark)))
		{
			switch (pos)
			{
			case BGN: MarkerBGN = mark; break;
			case END: MarkerEND = mark; break;
			}
			fiber.UpdateNow();
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

HRESULT ImageWnd::PicWnd::ValidatePnt( CPoint& pnt )
{
	CPoint ret = pnt; 
	if (accum.bmp != NULL)
	{
		if (ret.x < 0) ret.x = 0;
		if (ret.x >= accum.w)	 ret.x = accum.w - 1;
		if (ret.y < 0) ret.y = 0;
		if (ret.y >= accum.h)	 ret.y = accum.h - 1;
		pnt = ret;
		return S_OK;
	}
	else
	{
		return E_FAIL;	
	}
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
	//ControledLogMessage log; log << _T("Speed tests results");

	//if (accum.bmp != NULL)
	//{
	//	CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); TChart& chrt=mf->Chart1; 
	//	mf->TabCtrl1.ChangeTab(mf->TabCtrl1.FindTab("Main control"));	

	//	//OrgPicRgn scan_rgn = Parent->ScanRgn; CPoint cntr = scan_rgn.CenterPoint();
	//	//T.Format("Scan line y = %d N = %d", cntr.y, accum.n);

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
	//	//accum.ScanLine(&(ChartMsg->Points), cntr.y, scan_rgn.left, scan_rgn.right);
	//	log.T.Format("Scan of %d points took %g ms", ChartMsg->Points.GetSize(), accum.fillTime.val()); 
	//	log << log.T;
	//	ChartMsg->Dispatch();
	//	log.Dispatch();
	//}
}

HRESULT ImageWnd::PicWnd::ScanArbitaryLine(void * const buf, CPoint beg, CPoint end, MyTimer &Timer)
{
	ScanLineData line; HRESULT ret;
	beg.y = accum.h -1 - beg.y; end.y = accum.h -1 - end.y;	
	if (SUCCEEDED(ret = line.Init(beg, end)))
	{
		accum.ScanArbitaryLine(buf, line, &Timer);
	}
	return ret;
}

void ImageWnd::PicWnd::OnPicWndScanArbitaryLine()
{
	if (accum.HasImage())
	{
		void* x; CString T; MyTimer Timer1;
		TPointVsErrorSeries::DataImportMsg *ChartMsg = NULL; 
		ControledLogMessage log; log << _T("Speed tests results");

		CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); TChart& chrt=mf->Chart1; 
		mf->TabCtrl1.ChangeTab(mf->TabCtrl1.FindTab("Main control"));	

		if((x = chrt.Series.GainAcsess(WRITE))!=0)
		{
			SeriesProtector Protector(x); TSeriesArray& Series(Protector);
			TPointVsErrorSeries *t2;
			if((t2 = new TPointVsErrorSeries(T))!=0)	
			{
				for(int i=0;i<Series.GetSize();i++) Series[i]->SetStatus(SER_INACTIVE);
				Series.Add(t2); 
				t2->_SymbolStyle::Set(NO_SYMBOL); t2->_ErrorBarStyle::Set(NO_BARS);
				ChartMsg = t2->CreateDataImportMsg(); 
				t2->AssignColors(ColorsStyle(clRED,Series.GetRandomColor()));
				t2->PointType.Set(GenericPnt); 
				t2->SetStatus(SER_ACTIVE); t2->SetVisible(true);
			}		
		}

		if (SUCCEEDED(ScanArbitaryLine(&(ChartMsg->Points), Parent->MarkerBGN, Parent->MarkerEND, Timer1)))
		{			
			log.T.Format("Scan of %d points from (%d,%d) to (%d,%d) took %g ms", 
				ChartMsg->Points.GetSize(), 
				Parent->MarkerBGN.x, Parent->MarkerBGN.y, Parent->MarkerEND.x, Parent->MarkerEND.y,
				Timer1.GetValue().val()); 
			log << log.T;
			ChartMsg->Dispatch();
			log.Dispatch();
		}
	}
}

void ImageWnd::PicWnd::OnPicWndMultiCrossHelper(ScanLineData tmp_line, GaussFitFunc &GaussFit)
{
	Parent->MarkerBGN = tmp_line.beg; Parent->MarkerEND = tmp_line.end; /*OnPicWndScanArbitaryLine();*/
	PointVsErrorArray buf; buf.RemoveAll(); MyTimer Timer;
	if (SUCCEEDED(ScanArbitaryLine(&buf, tmp_line.beg, tmp_line.end, Timer)))
	{		
		ControledLogMessage log;
		//DoubleArray init; init << 10 << 1 << 1 << .1;
		DoubleArray init; init << 10 << 100 << 50 << 20;
		GaussFit.CalculateFrom(buf.x, buf.y, buf.dy, init);
		//log.T.Format("************ GaussFit **********************"); log << log.T;
		//GaussFit.GetReport(log);
		//double width = GaussFit.GetWidth(); log.T.Format("width = %g", width); log << log.T;
		//log.Dispatch();
		
		/*DoubleArray X, Y;
		if (SUCCEEDED(GaussFit.MakeGraph(X, Y)))
		{
			void *x; TSimplePointSeries::DataImportMsg *ChartMsg = NULL; 
			CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); TChart& chrt=mf->Chart1; 
			mf->TabCtrl1.ChangeTab(mf->TabCtrl1.FindTab("Main control"));	
			if((x = chrt.Series.GainAcsess(WRITE))!=0)
			{
				SeriesProtector Protector(x); TSeriesArray& Series(Protector);
				TSimplePointSeries *t2;
				if((t2 = new TSimplePointSeries(_T("GaussFit graph")))!=0)	
				{
					for(int i=0;i<Series.GetSize();i++) Series[i]->SetStatus(SER_INACTIVE);
					Series.Add(t2); 
					t2->_SymbolStyle::Set(NO_SYMBOL); t2->_ErrorBarStyle::Set(NO_BARS);
					ChartMsg = t2->CreateDataImportMsg(); 
					t2->AssignColors(ColorsStyle(clRED,Series.GetRandomColor()));
					t2->PointType.Set(GenericPnt); 
					t2->SetStatus(SER_ACTIVE); t2->SetVisible(true);
				}		
			}
			SimplePoint pnt;
			for (int i = 0; i < X.GetSize(); i++)
			{
				pnt.x = X[i]; pnt.y = Y[i];
				ChartMsg->Points.Add(pnt);
			}
			ChartMsg->Dispatch();
		}*/
	}
}

void ImageWnd::PicWnd::OnPicWndMultiCross()
{	
	const int dd = 20; MyTimer Timer;
	CPoint ParentBgn = Parent->MarkerBGN, ParentEnd = Parent->MarkerEND;

	CPoint pnt = Parent->MarkerBGN;
	CPoint beg = pnt - CPoint(dd, 0), end = pnt + CPoint(dd, 0);
	ScanLineData line, tmp_line;
	line.Init(beg, end);
	GaussFitFunc GaussFit; 

	//tmp_line = line; OnPicWndMultiCrossHelper(tmp_line, TODO);	
	//tmp_line = line; tmp_line.RotateByAngle(45*DEGREE, ScanLineRotationMode::CNTR); OnPicWndMultiCrossHelper(tmp_line, TODO);
	//tmp_line = line; tmp_line.RotateByAngle(90*DEGREE, ScanLineRotationMode::CNTR); OnPicWndMultiCrossHelper(tmp_line, TODO);
	//tmp_line = line; tmp_line.RotateByAngle(135*DEGREE, ScanLineRotationMode::CNTR); OnPicWndMultiCrossHelper(tmp_line, TODO);

	void *x; TSimplePointSeries::DataImportMsg *ChartMsg = NULL; 
	CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); TChart& chrt=mf->Chart1; 
	mf->TabCtrl1.ChangeTab(mf->TabCtrl1.FindTab("Main control"));	
	if((x = chrt.Series.GainAcsess(WRITE))!=0)
	{
		SeriesProtector Protector(x); TSeriesArray& Series(Protector);
		TSimplePointSeries *t2;
		if((t2 = new TSimplePointSeries(_T("GaussFit graph")))!=0)	
		{
			for(int i=0;i<Series.GetSize();i++) Series[i]->SetStatus(SER_INACTIVE);
			Series.Add(t2); 
			t2->_SymbolStyle::Set(NO_SYMBOL); t2->_ErrorBarStyle::Set(NO_BARS);
			ChartMsg = t2->CreateDataImportMsg(); 
			t2->AssignColors(ColorsStyle(clRED,Series.GetRandomColor()));
			t2->PointType.Set(GenericPnt); 
			t2->SetStatus(SER_ACTIVE); t2->SetVisible(true);
		}		
	}
	SimplePoint spnt;
	for (spnt.x = -90; spnt.x <= 260; spnt.x += 5)
	{
		tmp_line = line; tmp_line.RotateByAngle(spnt.x*DEGREE, ScanLineRotationMode::CNTR); 
		OnPicWndMultiCrossHelper(tmp_line, GaussFit);		
		if (GaussFit.a[1] > 0 && GaussFit.a[2] > 0)
		{
			spnt.y = 1/GaussFit.GetWidth();
			ChartMsg->Points.Add(spnt);
		}
		else
		{
			spnt.y = 0;
		}
		
	}
	ChartMsg->Dispatch();

	Parent->MarkerBGN = ParentBgn; Parent->MarkerEND = ParentEnd;
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

//======================================
void ImagesAccumulator::ResetSums()
{
	if (sums != NULL)	{ free(sums); sums = NULL; OldSumsSize = 0;}
}

void ImagesAccumulator::Reset()
{
	ResetSums(); 
	if (bmp != NULL)	{ delete bmp; bmp = NULL; }
	n = 0; w = h = 0; HasErrors = FALSE;
}

HRESULT ImagesAccumulator::Initialize(int _w, int _h, BOOL _HasErrors/* = TRUE*/)
{
	if (w != _w || h != _h)
	{
		ResetSums();
		w = _w; h = _h; HasErrors = _HasErrors;
		sums = (BYTE*)malloc((OldSumsSize = GetSumsSize())); 
		if (sums == 0) return E_FAIL;
		memset(sums, 0, GetSumsSize());		
	}
	else
	{
		if (HasErrors != _HasErrors)
		{
			w = _w; h = _h; HasErrors = _HasErrors;
			size_t NewSumsSize = GetSumsSize();
			BYTE *new_sums = (BYTE*)realloc(sums, NewSumsSize); 
			if (new_sums == NULL) return E_FAIL;
			sums = new_sums; 
			if(NewSumsSize > OldSumsSize)
			{
				BYTE *tmp_sums = sums; tmp_sums += OldSumsSize;
				memset(tmp_sums, 0, NewSumsSize - OldSumsSize);				
			}
			for (int i = OldSumsSize - 1; i > 0; i--)
			{
				new_sums[2*i] = new_sums[i];
				new_sums[i] = 0;
			}
			USHORT *tmp_sums = GetSum(); UINT *tmp_sums2 = GetSums2();
			for (size_t i = 0; i < OldSumsSize; i++, tmp_sums++, tmp_sums2++)
			{
				*tmp_sums2 = (*tmp_sums)*(*tmp_sums);				
			}
			OldSumsSize = NewSumsSize;
		}
	}
	return S_OK;
}

HRESULT ImagesAccumulator::FillAccum(BMPanvas *src, MyTimer *Timer1)
{
	HRESULT ret;
	if (src == NULL) return E_FAIL;
	if (src->HasImage() == FALSE) return E_FAIL;	

	if (w != src->w || h != src->h)
	{
		if (n > 0)
		{
			ret = E_FAIL;			
		}
		else
		{
			ret = Initialize(src->w, src->h, FALSE);
		}
		if (FAILED(ret))
		{
			Reset();
			ControledLogMessage log(lmprHIGH);
			log.T.Format("ImagesAccumualtor error: %d != %d or %d != %d", w, src->w, h, src->h); log << log.T;
			log.Dispatch(); 
			return ret;
		}
	}
	if (n == 1)
	{		
		ret = Initialize(src->w, src->h);
	}
	if (sums == NULL) return E_FAIL;
	if (FAILED(ret)) 
	{
		Reset();
		return E_FAIL;
	}		
	if (Timer1 != NULL) Timer1->Start(); 
	src->LoadBitmapArray(); 
	if (HasErrors)
	{		
		BYTE *src_pxl; USHORT *accum_pxl; UINT* accum_pxl2;
		accum_pxl = GetSum(); accum_pxl2 = GetSums2();

		for (int y = 0; y < h; y++)
		{			
			src_pxl = src->arr + src->wbyte*y;
			for (int x = 0; x < w; x++)
			{
				*accum_pxl += *src_pxl; *accum_pxl2 += (*src_pxl)*(*src_pxl);
				src_pxl++; accum_pxl++; accum_pxl2++;
			}
		}		
	}
	else
	{
		BYTE *accum_pxl = (BYTE*)GetSum(), *bmp_pxl = src->arr;
		size_t line_size =  w*sizeof(BYTE);
		for (int y = 0; y < h; y++)
		{
			memcpy(accum_pxl, bmp_pxl, line_size);
			accum_pxl += line_size; bmp_pxl += src->wbyte;
		}
	}
	src->UnloadBitmapArray();		
	n++; 
	if (Timer1 != NULL) Timer1->Stop();
	return S_OK;
}

void ImagesAccumulator::ConvertToBitmap(CWnd *ref, MyTimer *Timer1)
{
	if (sums != NULL)
	{
		BYTE *dst_pxl; 
		if (Timer1 != NULL) Timer1->Start();
		if (bmp == NULL)
		{
			bmp = new BMPanvas(); 
		}
		bmp->Create(ref, w, h, 8); bmp->CreateGrayPallete(); bmp->LoadBitmapArray(); 
		if (HasErrors)
		{
			USHORT *accum_pxl = GetSum(); 
			for (int y = 0; y < h; y++)
			{			
				dst_pxl = bmp->arr + bmp->wbyte*y;
				for (int x = 0; x < w; x++)
				{
					*dst_pxl = (*accum_pxl)/n; 				
					dst_pxl++; accum_pxl++; 
				}
			}
		}
		else
		{
			BYTE *accum_pxl = (BYTE*)GetSum(); 
			for (int y = 0; y < h; y++)
			{			
				dst_pxl = bmp->arr + bmp->wbyte*y;
				for (int x = 0; x < w; x++)
				{
					*dst_pxl = *accum_pxl; 				
					dst_pxl++; accum_pxl++; 
				}
			}
		}
		bmp->SetBitmapArray(); 
		if (Timer1 != NULL) Timer1->Stop(); 
	}
}

HRESULT ImagesAccumulator::SaveTo( const CString &file )
{
	HRESULT ret; ControledLogMessage log; MyTimer Timer1;

	if (sums != NULL)
	{
		Compressor cmpr(Compressor::ZIP, 9); CFile dst0;
		TRY
		{
			CFileException Ex;
			if (dst0.Open(file, CFile::modeCreate | CFile::modeWrite| CFile::typeBinary) == FALSE)
			{
				Ex.ReportError(); ret = E_FAIL; return ret;
			}			
			CArchive ar(&dst0, CArchive::store); Serialize(ar); ar.Close();
		}
		AND_CATCH(CArchiveException, pEx)
		{
			pEx->ReportError(); ret = E_FAIL; return ret;
		}
		END_CATCH

			CMemFile src0; 
		src0.Attach(sums, GetSumsSize(), GetCompressorBufferSize()); src0.SetLength(GetSumsSize());				
		if ((ret = cmpr.Process(&src0, &dst0)) != Z_OK) 
		{
			dst0.Abort();
			log.T.Format("Failed to save %s", file); 
			log << log.T; log.SetPriority(lmprHIGH);	
		}		
		else 
		{			
			dst0.Close(); 
			log.T.Format("Time to save %s - %s Ratio = %.2f", 
				file, ConvTimeToStr(cmpr.LastSession.dt), cmpr.LastSession.ratio); 			
			log << log.T;
		}
		src0.Detach(); src0.Close();
	}
	log.Dispatch();	
	return ret;
}

HRESULT ImagesAccumulator::LoadFrom( const CString &file )
{	
	HRESULT ret = E_FAIL; ControledLogMessage log;
	if (bmp == NULL) bmp = new BMPanvas();		
	bmp->Destroy();

	ResetSums();
	ULONGLONG buf_size = 0; CFile src0;
	Compressor cmpr(Compressor::UNZIP);			
	TRY
	{
		CFileException Ex;
		if (src0.Open(file, CFile::modeRead| CFile::typeBinary, &Ex) == FALSE)
		{
			Ex.ReportError(); 
			return E_FAIL;
		}			
		CArchive ar(&src0, CArchive::load); 
		Serialize(ar); 
		ar.Close();
	}
	AND_CATCH(CArchiveException, pEx)
	{
		pEx->ReportError(); 
		return E_FAIL;
	}
	END_CATCH
		CMemFile dst0(GetCompressorBufferSize());	

	if ((ret = cmpr.Process(&src0, &dst0)) == Z_OK)
	{
		src0.Close(); sums = dst0.Detach();
		log.T.Format("Time to load %s - %s Ratio = %.2f", 
			file, ConvTimeToStr(cmpr.LastSession.dt), cmpr.LastSession.ratio); 			
		log << log.T;
	}	
	else
	{
		Reset();
		log.T.Format("Failed to UNZIP %s", file); 
		log << log.T; log.SetPriority(lmprHIGH);	
	}
	log.Dispatch();		
	return ret;
}

void ImagesAccumulator::ScanLine( void *_buf, const ScanRgnData &data, MyTimer *Timer1 ) const
{	
	PointVsErrorArray *buf = (PointVsErrorArray*)_buf;
	if (HasImage())
	{
		PointVsError pnte; pnte.type.Set(GenericPnt);
		if (Timer1 != NULL) Timer1->Start(); 
		CSize size(data.Xmax - data.Xmin, 2*data.AvrRange + 1); int N = n*size.cy;	
		UINT *avr_accum = new UINT[size.cx];
		ULONG *avr_accum2 = new ULONG[size.cx];
		memset(avr_accum, 0, size.cx*sizeof(UINT));
		memset(avr_accum2, 0, size.cx*sizeof(ULONG));

		if (HasErrors)
		{			
			for (int y = data.stroka - data.AvrRange; y <= data.stroka + data.AvrRange; y++)
			{
				USHORT *accum_pxl = GetSum(); UINT *accum_pxl2 = GetSums2(); 
				accum_pxl += y*w + data.Xmin; accum_pxl2 += y*w + data.Xmin;

				for (int x = 0; x < size.cx; x++, accum_pxl++, accum_pxl2++)
				{
					avr_accum[x] += *accum_pxl;
					avr_accum2[x] += *accum_pxl2;
				}
			}
		}
		else
		{
			for (int y = data.stroka - data.AvrRange; y <= data.stroka + data.AvrRange; y++)
			{
				BYTE *accum_pxl = (BYTE*)GetSum();
				accum_pxl += y*w + data.Xmin;

				for (int x = 0; x < size.cx; x++, accum_pxl++)
				{
					avr_accum[x] += *accum_pxl;
					avr_accum2[x] += (*accum_pxl)*(*accum_pxl);
				}
			}
		}

		for (int x = 0; x < size.cx; x++)
		{
			pnte.x = data.Xmin + x; pnte.y = (float)(avr_accum[x])/N; 
			if (N > 1)
			{				
				ULONGLONG val = N*avr_accum2[x] - avr_accum[x]*avr_accum[x];
				pnte.dy = sqrt(((float)val)/((N - 1)*N));
			}
			else
			{
				pnte.dy = 0;
			}				
			buf->Add(pnte);
		}
		if (Timer1 != NULL) Timer1->Stop();
		delete[] avr_accum; delete[] avr_accum2;
	}
}

USHORT* ImagesAccumulator::GetSum() const
{
	return (USHORT*)(sums != NULL ? sums:NULL);
}

UINT* ImagesAccumulator::GetSums2() const
{
	return (UINT*)((sums != NULL && HasErrors == TRUE) ? sums + w*h*sizeof(USHORT):NULL);
}

ImagesAccumulator::ImagesAccumulator() : sums(NULL), bmp(NULL)
{
	Reset();	
}

HRESULT ImagesAccumulator::GetPicRgn( CRect& PicRgn ) const
{
	if (GetSum() != NULL)
	{
		PicRgn = CRect(CPoint(0, 0),CSize(w, h));
		return S_OK;
	}
	return E_FAIL;
}

PointVsError3D ImagesAccumulator::GetPoint( const CPoint &pnt ) const
{
	PointVsError3D pnte; USHORT avr_accum;	UINT avr_accum2;
	const int N = n; const int shift = pnt.y*w + pnt.x;	
	
	if (HasErrors)
	{	
		USHORT *accum_pxl = GetSum(); UINT *accum_pxl2 = GetSums2(); 
		accum_pxl += shift; accum_pxl2 += shift;
		avr_accum = *accum_pxl;
		avr_accum2 = *accum_pxl2;
	}
	else
	{
		BYTE *accum_pxl = (BYTE*)GetSum();
		accum_pxl += shift;
		avr_accum = *accum_pxl;
		avr_accum2 = avr_accum*avr_accum;
	}

	if (N > 1)
	{				
		ULONGLONG val = N*avr_accum2 - avr_accum*avr_accum;
		pnte.dz = sqrt(((float)val)/((N - 1)*N));
	}
	else
	{
		pnte.dz = 0;
	}		
	pnte.x = pnt.x; pnte.y = pnt.y; pnte.z = (float)(avr_accum)/N; 
	return pnte;
}

void ImagesAccumulator::ScanArbitaryLine( void * const _buf, const ScanLineData &data, MyTimer *Timer1 /*= NULL*/ ) const
{
	PointVsErrorArray *buf = (PointVsErrorArray*)_buf;
	if (data.IsInited())
	{
		int N, X, Y; double A, B; 
		double r, x, y, dr, dx, dy; 
		PointVsError3D pnt[3];
		PointVsError pnte; pnte.type.Set(GenericPnt);

		if (Timer1 != NULL) Timer1->Start(); 
		r = 0;		x = data.beg.x;			y = data.beg.y;		
		dr = 0.5;	dx = dr*data.cosfi;		dy = dr*data.sinfi; 
		N = (int)(1 + data.len/dr);
		
		for (int i = 0; i < N; i++)
		{
			X = (int)x; Y = (int)y; 
			pnt[0] = GetPoint(CPoint(X, Y)); pnt[2] = GetPoint(CPoint(X + 1, Y + 1));	
			if ((double)(x - X + y - Y) > 1.)
			{
				pnt[1] = GetPoint(CPoint(X + 1, Y));
				A = pnt[0].z - pnt[1].z; B = pnt[1].z - pnt[2].z;
			}
			else
			{
				pnt[1] = GetPoint(CPoint(X, Y + 1));
				A = pnt[1].z - pnt[2].z; B = pnt[0].z - pnt[1].z;
			}

			pnte.x = r; pnte.y = pnt[0].z - (x - X)*A - (y - Y)*B; pnte.dy = 0.;
			buf->Add(pnte);	
			r = i*dr; x = data.beg.x + i*dx; y = data.beg.y + i*dy;
		}
		if (Timer1 != NULL) Timer1->Stop();
	}
}

size_t AccumInfo::GetSumsSize() const
{
	size_t ret;
	if (HasErrors)
	{
		ret = w*h*(sizeof(USHORT) + sizeof(UINT));
	}
	else
	{
		ret = w*h*sizeof(BYTE);
	}
	return ret;
}

//======================================

void AccumInfo::Serialize( CArchive &ar )
{
	if(ar.IsStoring())
	{
		ar << n << w << h << HasErrors;
	}
	else
	{
		ar >> n >> w >> h >> HasErrors;
	}

}

size_t AccumInfo::GetCompressorBufferSize() const
{
	size_t ret;
	if (HasErrors)
	{
		ret = (w*10*(sizeof(UINT) + sizeof(USHORT)));
	}
	else
	{
		ret = w*10*sizeof(BYTE);				
	}
	return ret;	
}
//////////////////////////////////////////////////
//////////////////////////////////////////////////
double ScanLineData::Get_Length() const
{
	double dy = Get_dY(), dx = Get_dX();
	return sqrt(dy*dy + dx*dx);
}

double ScanLineData::Get_dX() const { return (double)(end.x - beg.x); }
double ScanLineData::Get_dY() const { return (double)(end.y - beg.y); }
HRESULT ScanLineData::Init( const CPoint &_beg, const CPoint &_end ) 
{ 
	beg = _beg; end = _end; 
	dX = Get_dX(); dY = Get_dY();
	if ((len = Get_Length()) != 0) { Get_cosfi(cosfi); Get_sinfi(sinfi); return (status = S_OK); }
	else { cosfi = 1.; sinfi = 0.; return (status = E_FAIL);	}
}

HRESULT ScanLineData::Get_cosfi( double &cosfi ) const
{
	double len;;
	if ((len = Get_Length()) != 0) { cosfi = Get_dX()/len; return S_OK; }
	return E_FAIL;
}

HRESULT ScanLineData::Get_sinfi( double &sinfi ) const
{
	double len;;
	if ((len = Get_Length()) != 0) { sinfi = Get_dY()/len; return S_OK; }
	return E_FAIL;
}

CPoint ShiftRotateShift(const CPoint pnt, const double shift_x, const double shift_y, const double Cos, const double Sin)
{
	CPoint ret; 
	double x = pnt.x - shift_x,		y = pnt.y - shift_y;
	double x1 = x*Cos + y*Sin,		y1 = -x*Sin + y*Cos;
	x1 += shift_x;			y1 += shift_y;
	ret.x = (LONG)x1;	ret.y = (LONG)y1;	
	return ret;
}

void ScanLineData::RotateByAngle( const double RadiansAngle, const ScanLineRotationMode mode )
{
	if (IsInited())
	{
		CPoint tmp; double cx, cy;
		double Cos = cos(RadiansAngle), Sin = sin(RadiansAngle);
		switch (mode)
		{
			case BEG: 
				Init(beg, ShiftRotateShift(end, beg.x, beg.y, Cos, Sin));
				break;
			case CNTR: 
				cx = beg.x + len*cosfi/2; cy = beg.y + len*sinfi/2;
				Init(ShiftRotateShift(beg, cx, cy, Cos, Sin), ShiftRotateShift(end, cx, cy, Cos, Sin));
				break;
			case END: 
				Init(ShiftRotateShift(beg, end.x, end.y, Cos, Sin), end);
				break;
		}
	}
}
