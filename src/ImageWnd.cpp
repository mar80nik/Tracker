// ImageWnd.cpp : implementation file
//

#include "stdafx.h"
#include "KSVU3.h"
#include "ImageWnd.h"
#include "TchartSeries.h"
#include "MainFrm.h"
#include "dcm800.h"
//#include "monochromator.h"
#include "metricon.h"
#include "my_color.h"
#include "captureWnd.h"
#include "BMPanvas.h"

// ImageWnd

IMPLEMENT_DYNAMIC(ImageWnd, CWnd)
ImageWnd::ImageWnd()
{
	scale=10;
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

	CRect tr; GetClientRect(&tr); CSize wndSize(600,400+200);
	fiber.Create(0,"FIBER",WS_CHILD | WS_VISIBLE | WS_CAPTION,CRect(CPoint(0,0),CSize(600, 600)),this,111,0);
	CameraWnd.Create(0, "CameraWnd", WS_CHILD | WS_BORDER | WS_VISIBLE, CRect(CPoint(tr.Width()-wndSize.cx,0),wndSize), this, ID_MV_WND, 0);	
	OnChildMove();
	CameraWnd.Ctrls.Create(IDD_DIALOGBARTAB4,&Ctrls); 
	CameraWnd.Ctrls.SetWindowPos(NULL,500,0,0,0,SWP_NOSIZE | SWP_NOZORDER);
	CameraWnd.Ctrls.ShowWindow(SW_SHOW);
#define TEST2
#ifdef DEBUG
 	fiber.LoadPic(CString("exe\\fiber.png"));
#endif
	Ctrls.Parent=this;

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
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedScan)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedCalibrate)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_BUTTON4, &ImageWnd::CtrlsTab::OnBnClickedCalcTE)
	ON_BN_CLICKED(IDC_BUTTON5, &ImageWnd::CtrlsTab::OnBnClickedButton5)
	//}}AFX_MSG_MAP	
	ON_EN_KILLFOCUS(IDC_EDIT1, &ImageWnd::CtrlsTab::OnEnKillfocusEdit1)
	ON_BN_CLICKED(IDC_BUTTON11, &ImageWnd::CtrlsTab::OnBnClickedCalcTM)
	ON_MESSAGE(UM_BUTTON_ITERCEPTED,&ImageWnd::CtrlsTab::OnButtonIntercepted)
END_MESSAGE_MAP()

BOOL ImageWnd::CtrlsTab::OnInitDialog() 
{
	CDialog::OnInitDialog();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

ImageWnd::CtrlsTab::CtrlsTab( CWnd* pParent /*= NULL*/ ): BarTemplate(pParent)
, stroka(1220)
, AvrRange(100)
, Xmin(100)
, Xmax(3100)
{
}

void ImageWnd::CtrlsTab::DoDataExchange(CDataExchange* pDX)
{
	BarTemplate::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogBarTab1)
	DDX_Text(pDX, IDC_EDIT1, stroka);
	DDX_Text(pDX, IDC_EDIT2, AvrRange);
	DDX_Text(pDX, IDC_EDIT3, Xmin);
	DDX_Text(pDX, IDC_EDIT4, Xmax);
	DDV_MinMaxInt(pDX, AvrRange, 1, 1000);
	DDX_Control(pDX, IDC_EDIT3, XminCtrl);
	DDX_Control(pDX, IDC_EDIT4, XmaxCtrl);
	DDX_Control(pDX, IDC_EDIT1, strokaCtrl);
	DDX_Control(pDX, IDC_EDIT2, AvrRangeCtrl);
	//}}AFX_DATA_MAP

}

void ImageWnd::CtrlsTab::OnBnClickedScan()
{
	UpdateData(); ImageWnd* parent=(ImageWnd*)Parent; void* x; CString T,T1;
	MyTimer Timer1,Timer2; sec time; CString logT; TPointVsErrorSeries *t2;
	TPointVsErrorSeries::DataImportMsg *ChartMsg; 
	
	CMainFrame* MainWnd=(CMainFrame*)AfxGetMainWnd(); 
	MainWnd->TabCtrl1.ChangeTab(MainWnd->TabCtrl1.FindTab("Main control"));	

	LogMessage *log=new LogMessage(); log->CreateEntry("Log","Speed tests results",LogMessage::low_pr);

	if(!(parent->fiber.org.HasImage())) return;

	Timer2.Start();

	CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); TChart& chrt=mf->Chart1; 
	if((x=chrt.Series.GainAcsess(WRITE))!=0)
	{
		SeriesProtector Protector(x); TSeriesArray& Series(Protector);
		if((t2=new TPointVsErrorSeries(T))!=0)	
		{
			for(int i=0;i<Series.GetSize();i++) Series[i]->SetStatus(SER_INACTIVE);
			Series.Add(t2); 
			t2->_SymbolStyle::Set(NO_SYMBOL); 
			ChartMsg=t2->CreateDataImportMsg(); 
			t2->AssignColors(ColorsStyle(clRED,Series.GetRandomColor()));
			t2->PointType.Set(GenericPnt); 
			t2->SetStatus(SER_ACTIVE); t2->SetVisible(true);
		}		
	}
	
    Timer1.Start();	parent->fiber.org.LoadBitmapArray(); Timer1.Stop(); time=Timer1.GetValue();
	
	parent->fiber.org.UnloadBitmapArray();
	
	Timer2.Stop(); time=Timer2.GetValue();
	logT.Format("Total processing time=%s",ConvTimeToStr(time)); log->Msgs.Add(logT);

	CKSVU3App* Parent=(CKSVU3App*)AfxGetApp(); 
	Parent->myThread.PostParentMessage(UM_GENERIC_MESSAGE,log);	
}

ImageWnd::PicWnd::PicWnd()
{
	Parent=0;
}

ImageWnd::PicWnd::~PicWnd()
{

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
	if(org.HasImage())
	{
		
	}
	ava.CopyTo(hdc,TOP_LEFT);
}

void ImageWnd::PicWnd::UpdateNow(void)
{
	RedrawWindow(0,0,RDW_INVALIDATE | RDW_FRAME | RDW_NOERASE | RDW_ALLCHILDREN);					
}

void ImageWnd::PicWnd::LoadPic(CString T)
{
	if(org.LoadImage(T)==S_OK)
	{
		Parent->CameraWnd.Ctrls.UpdateData();			
		if (Parent->CameraWnd.Ctrls.ColorTransformSelector == CaptureWndCtrlsTab::TrueColor)
		{
			LogMessage *log=new LogMessage(); 
			log->CreateEntry("Error","Image you are trying to load is no GRAYSCALE.",LogMessage::high_pr);			
			log->CreateEntry("*****","In order to use bult-in convertor please select",LogMessage::high_pr);			
			log->CreateEntry("*****","convert method: NativeGDI, HSL or HSV.",LogMessage::high_pr);			
			CKSVU3App* Parent=(CKSVU3App*)AfxGetApp();
			Parent->myThread.PostParentMessage(UM_GENERIC_MESSAGE,log);
			return;
		}

		if (org.ColorType != BMPanvas::GRAY_PAL) ConvertOrgToGrayscale();
		FileName=T;
		SetStretchBltMode(ava.GetDC(),COLORONCOLOR);		
		org.StretchTo(&ava,ava.Rgn,org.Rgn,SRCCOPY);
        HGDIOBJ tfont=ava.SelectObject(font1);
		ava.SetBkMode(TRANSPARENT); ava.SetTextColor(clRED);
		ava.TextOut(0,0,T);
		T.Format("%dx%d",org.w,org.h); ava.TextOut(0,10,T);
		ava.SelectObject(tfont); 
		CaptureButton.ShowWindow(SW_HIDE); DragAcceptFiles(FALSE);
		UpdateNow();
		Parent->Ctrls.Xmax=org.w; Parent->Ctrls.UpdateData();
	}
	else FileName="";		 
}

void ImageWnd::PicWnd::OnDropFiles(HDROP hDropInfo)
{
	char buf[1000]; CString T,T2;  GetWindowText(T2);
	DragQueryFile(hDropInfo,0xFFFFFFFF,buf,1000);
	DragQueryFile(hDropInfo,0,buf,1000); T=CString(buf); 
	MyTimer Timer1; sec time; CString logT;
	LogMessage *log=new LogMessage(); log->CreateEntry("Log","Speed tests results",LogMessage::low_pr);
	
	Timer1.Start();
	LoadPic(T);	
	Timer1.Stop(); 
	time=Timer1.GetValue(); logT.Format("%s org (%.2f Mpix) load time=%s",T2,org.w*org.h/1.e6,ConvTimeToStr(time)); log->Msgs.Add(logT);	

	CKSVU3App* Parent=(CKSVU3App*)AfxGetApp(); 
	Parent->myThread.PostParentMessage(UM_GENERIC_MESSAGE,log);	

	CWnd::OnDropFiles(hDropInfo);
}

void ImageWnd::PicWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	BOOL update = FALSE; 
	if ( org.HasImage() == FALSE ) return;
	switch( nFlags )
	{
	case MK_SHIFT: update=TRUE; break;
	case MK_CONTROL: update=TRUE; break;
	case 0: update=TRUE; break;	
	}
	if ( update )
	{
	}
	CWnd::OnLButtonUp(nFlags, point);
}

void ImageWnd::PicWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
	BOOL update = FALSE; 
	if ( org.HasImage() == FALSE ) return;
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
	CaptureButton.EnableWindow(FALSE);
    Parent->CameraWnd.PostMessage(UM_CAPTURE_REQUEST,(WPARAM)this, (LPARAM)&org);
}

LRESULT ImageWnd::PicWnd::OnCaptureReady( WPARAM wParam, LPARAM lParam )
{
	CString T;
	if(org.HasImage())
	{
		SetStretchBltMode(ava.GetDC(),COLORONCOLOR);		
		org.StretchTo(&ava,ava.Rgn,org.Rgn,SRCCOPY);

		HGDIOBJ tfont=ava.SelectObject(font1);
		ava.SetBkMode(TRANSPARENT); ava.SetTextColor(clRED);
		T="Camera capture"; ava.TextOut(0,0,T);
		T.Format("%dx%d",org.w,org.h); ava.TextOut(0,10,T);
		ava.SelectObject(tfont);
		UpdateNow();
		CaptureButton.EnableWindow(TRUE); CaptureButton.ShowWindow(SW_HIDE); DragAcceptFiles(FALSE);
	}
	return 0;
}
void ImageWnd::PicWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if(org.HasImage())
	{
		CMenu* menu = menu1.GetSubMenu(0);
		menu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
	}
}

void ImageWnd::PicWnd::OnPicWndErase()
{
	if(org.HasImage())
	{
		org.Destroy(); EraseAva();

		UpdateNow();
		CaptureButton.ShowWindow(SW_SHOW); DragAcceptFiles(true);
	}
}

void ImageWnd::PicWnd::OnPicWndSave()
{
	if(org.HasImage())
	{
		CFileDialog dlg1(FALSE,"png");
		if(dlg1.DoModal()==IDOK)
		{
            org.SaveImage(dlg1.GetFileName());
		}
	}
}

void ImageWnd::CtrlsTab::OnBnClickedCalibrate()
{
	CalibratorDlg.ShowWindow(SW_SHOW);


	ImageWnd* parent=(ImageWnd*)Parent; void *x=NULL; MyTimer Timer1; ms dt1,dt2;
	DoubleArray N,teta; CString T;
	CalibrationParams cal; cal.n_p=2.14044; cal.n_s=1.; 
	N << 2933 << 2506 << 1922 << 1203;
	teta << 63.34*DEGREE << 60.04*DEGREE << 55.16*DEGREE << 49.02*DEGREE;

	double lambda, k, n1, n3; DoubleArray bettaexp_TE;
	lambda = 632.8; k = 2.*M_PI/lambda;	n1 = 1.; n3 = 1.45705;
	bettaexp_TE << 1.9129 << 1.8544 << 1.7568 << 1.6159; 
	FilmFuncTEParams in_TE(bettaexp_TE, n1,n3,k);
	FilmParams outTE,outTM;
	DoubleArray bettaexp_TM;
	bettaexp_TM << 1.82422 << 1.76110 << 1.65829 << 1.51765; 
	FilmFuncTMParams in_TM(bettaexp_TM, n1,n3,k);

	CalcRParams params;
	params.i=FilmParams(1,150,0+1e-100); 
	params.f=FilmParams(1.84,1082,5e-3+1e-100);  
	params.s=FilmParams(1.45705,1e6,0+1e-100); 
	params.lambda=632.8; params.Np=2.14044; params.teta_min=15; params.teta_max=85;

	TSimplePointSeries *t1=NULL; 
	TSimplePointSeries::DataImportMsg *CHM1, *CHM2; CHM1=CHM2=NULL; 
	CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); 
	LogMessage *log=new LogMessage(); log->CreateEntry("Log","Speed tests results",LogMessage::low_pr);
	SimplePoint pnt; pnt.type.Set(GenericPnt);

	CreateCalibration(N, teta, cal);
	CalibratorParams calb_params(1000.203414900858);
	Calibrator(calb_params,cal);
	CalclFilmParamsTE(in_TE,outTE);
	CalclFilmParamsTM(in_TM,outTM);
	int a=5;

	T.Format("****Statistics***"); log->CreateEntry("*",T,LogMessage::high_pr);
	T.Format("---Calibration---"); log->CreateEntry("*",T);
	T.Format("N0=%.10f L=%.10f d0=%.10f fi0=%.10f errabs=%g errrel=%g",cal.N0,cal.L,cal.d0,cal.fi0,cal.epsabs,cal.epsrel); log->CreateEntry("*",T);
	T.Format("dt=%.3f ms func_calls=%d",cal.dt.val(), cal.func_call_cntr); log->CreateEntry("*",T);
	T.Format("---Calibrator----"); log->CreateEntry("*",T);
	T.Format("Npix=%g teta=%.10f betta=%.10f errabs=%g errrel=%g",calb_params.Npix,calb_params.teta, calb_params.betta,calb_params.epsabs,calb_params.epsrel); log->CreateEntry("*",T);
	T.Format("dt=%.3f ms func_calls=%d",calb_params.dt.val(), calb_params.func_call_cntr); log->CreateEntry("*",T);
	T.Format("--FilmParamsTE---"); log->CreateEntry("*",T);
	T.Format("n=%.10f H=%.10f nm",outTE.n, outTE.H, outTE.epsabs, outTE.epsrel ); log->CreateEntry("*",T);
	T.Format("errabs=%g errrel=%g fval=%.10f, step=%.10f",outTE.epsabs, outTE.epsrel, outTE.fval, outTE.size ); log->CreateEntry("*",T);
	T.Format("dt=%.3f ms func_calls=%d",outTE.dt.val(), outTE.func_call_cntr); log->CreateEntry("*",T);
	for(int i=0;i<in_TE.betta_teor.GetSize();i++)
	{
		T.Format("betta_teor[%d]=%.5f betta_exp=%.5f",in_TE.betta_teor[i].n,in_TE.betta_teor[i].val,bettaexp_TE[i]); log->CreateEntry("*",T);
	}		
	T.Format("--FilmParamsTM---"); log->CreateEntry("*",T);
	T.Format("n=%.10f H=%.10f nm",outTM.n, outTM.H, outTM.epsabs, outTM.epsrel); log->CreateEntry("*",T);
	T.Format("errabs=%g errrel=%g fval=%.10f, step=%.10f",outTM.epsabs, outTM.epsrel, outTM.fval, outTM.size ); log->CreateEntry("*",T);
	T.Format("dt=%.3f ms func_calls=%d",outTM.dt.val(), outTM.func_call_cntr); log->CreateEntry("*",T);
	for(int i=0;i<in_TM.betta_teor.GetSize();i++)
	{
		T.Format("betta_teor[%d]=%.5f betta_exp=%.5f",in_TM.betta_teor[i].n,in_TM.betta_teor[i].val,bettaexp_TM[i]); log->CreateEntry("*",T);
	}		
	log->Dispatch();

}

int ImageWnd::CtrlsTab::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if (BarTemplate::OnCreate(lpCreateStruct) == -1)
		return -1;

	CalibratorDlg.Create(IDD_DIALOG_CAL,this);
	CalibratorDlg.SetWindowPos(NULL,300,300,0,0,SWP_NOSIZE | SWP_NOZORDER);

	CalcTEDlg.Create(IDD_DIALOG_CALCTE,this);
	CalcTEDlg.SetWindowPos(NULL,600,300,0,0,SWP_NOSIZE | SWP_NOZORDER);

	CalcTMDlg.Create(IDD_DIALOG_CALCTE,this);
	CalcTMDlg.SetWindowPos(NULL,300,300,0,0,SWP_NOSIZE | SWP_NOZORDER);
	CalcTMDlg.SetWindowText("TM Calculator"); CalcTMDlg.IsTM=TRUE;

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
	BMPanvas temp_replica; 
	temp_replica.Create(&org, org.Rgn); org.CopyTo(&temp_replica, TOP_LEFT);
	org.Destroy(); org.Create(this,temp_replica.w,temp_replica.h,8);
	org.CreateGrayPallete(); Parent->CameraWnd.Ctrls.UpdateData();			
	ColorTransform(&temp_replica, &org, Parent->CameraWnd.Ctrls.ColorTransformSelector);
}

BOOL ImageWnd::CtrlsTab::DestroyWindow()
{
	CalibratorDlg.DestroyWindow();

	return BarTemplate::DestroyWindow();
}

void ImageWnd::CtrlsTab::OnBnClickedCalcTE()
{
	CalcTEDlg.ShowWindow(SW_SHOW);
}

void ImageWnd::CtrlsTab::OnBnClickedCalcTM()
{
	CalcTMDlg.ShowWindow(SW_SHOW);
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

