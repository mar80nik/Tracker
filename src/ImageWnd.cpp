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

// ImageWnd

IMPLEMENT_DYNAMIC(ImageWnd, CWnd)
ImageWnd::ImageWnd(): //darkChart("dark"), cupolChart("cupol"),stripsChart("strips"),
	dark_ava_r(CRect(CPoint(0,0),CSize(326,244))),
	cupol_ava_r(CRect(CPoint(0,250),CSize(326,244))),
	strips_ava_r(CRect(CPoint(0,500),CSize(326,244)))
{
	scale=10;
}

ImageWnd::~ImageWnd() {}


BEGIN_MESSAGE_MAP(ImageWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
ON_WM_DESTROY()
END_MESSAGE_MAP()



// ImageWnd message handlers


int ImageWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect tr; GetClientRect(&tr); CSize wndSize(800,600+200);
	dark.Create(0,"DARK",WS_CHILD | WS_VISIBLE | WS_CAPTION,CRect(CPoint(0,0),CSize(326,265)),this,111,0);
	cupol.Create(0,"CUPOL",WS_CHILD | WS_VISIBLE | WS_CAPTION,CRect(CPoint(0,265),CSize(326,265)),this,222,0);
	strips.Create(0,"STRIPS",WS_CHILD | WS_VISIBLE | WS_CAPTION,CRect(CPoint(0,530),CSize(326,265)),this,333,0);
	CameraWnd.Create(0, "CameraWnd", WS_CHILD | WS_BORDER | WS_VISIBLE, CRect(CPoint(tr.Width()-wndSize.cx,0),wndSize), this, ID_MV_WND, 0);	
	OnChildMove();
	CameraWnd.Ctrls.Create(IDD_DIALOGBARTAB4,&Ctrls); 
	CameraWnd.Ctrls.SetWindowPos(NULL,500,0,0,0,SWP_NOSIZE | SWP_NOZORDER);
	CameraWnd.Ctrls.ShowWindow(SW_SHOW);

	dark.LoadPic(CString("exe\\dark.png"));
	cupol.LoadPic(CString("exe\\cupol.png"));
	strips.LoadPic(CString("exe\\strips.png"));

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

void ImageWnd::DrawScanLine( CPoint curL, CPoint curR )
{
	dark.ScanLine.curL=cupol.ScanLine.curL=strips.ScanLine.curL=curL;
	dark.ScanLine.curR=cupol.ScanLine.curR=strips.ScanLine.curR=curR;
	if(dark.org.HasImage()) dark.UpdateNow(); 
	if(cupol.org.HasImage()) cupol.UpdateNow(); 
	if(strips.org.HasImage()) strips.UpdateNow();
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
BEGIN_MESSAGE_MAP(ImageWndCtrlsTab, BarTemplate)
	//{{AFX_MSG_MAP(DialogBarTab1)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedScan)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedCalibrate)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_BUTTON4, &ImageWndCtrlsTab::OnBnClickedCalcTE)
	ON_BN_CLICKED(IDC_BUTTON5, &ImageWndCtrlsTab::OnBnClickedButton5)
	//}}AFX_MSG_MAP	
	ON_EN_KILLFOCUS(IDC_EDIT1, &ImageWndCtrlsTab::OnEnKillfocusEdit1)
	ON_BN_CLICKED(IDC_BUTTON11, &ImageWndCtrlsTab::OnBnClickedCalcTM)
END_MESSAGE_MAP()

BOOL ImageWndCtrlsTab::OnInitDialog() 
{
	CDialog::OnInitDialog();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

ImageWndCtrlsTab::ImageWndCtrlsTab( CWnd* pParent /*= NULL*/ ): BarTemplate(pParent)
, stroka(1220)
, AvrRange(100)
, Xmin(100)
, Xmax(3100)
{
}

void ImageWndCtrlsTab::DoDataExchange(CDataExchange* pDX)
{
	BarTemplate::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogBarTab1)
	DDX_Text(pDX, IDC_EDIT1, stroka);
	DDX_Text(pDX, IDC_EDIT2, AvrRange);
	DDV_MinMaxInt(pDX, AvrRange, 1, 1000);
	DDX_Text(pDX, IDC_EDIT3, Xmin);
	DDX_Text(pDX, IDC_EDIT4, Xmax);
	//}}AFX_DATA_MAP
}

void ImageWndCtrlsTab::OnBnClickedScan()
{
	UpdateData(); ImageWnd* parent=(ImageWnd*)Parent;
	
	parent->strips.ScanLine.curL.x=Xmin/parent->scale; parent->strips.ScanLine.curR.x=Xmax/parent->scale;
	parent->strips.ScanLine.curL.y=parent->strips.ScanLine.curR.y=stroka/parent->scale;
	
	parent->DrawScanLine(parent->strips.ScanLine.curL,parent->strips.ScanLine.curR);
	CMainFrame* MainWnd=(CMainFrame*)AfxGetMainWnd(); 
	MainWnd->TabCtrl1.ChangeTab(MainWnd->TabCtrl1.FindTab("Main control"));	

	 void* x; CString T,T1; BYTE *dark,*cupol,*strips;
	TPointVsErrorSeries *t2; //TSimplePointSeries *t1; 
	int midl=stroka, dd=AvrRange,w=parent->strips.org.w, min=midl-dd, max=midl+dd, mm=(max-min)/2; 
	MyTimer Timer1,Timer2; sec time; CString logT; 
	TPointVsErrorSeries::DataImportMsg *CHM2, *AdarkChartMsg, *AcupolChartMsg, *AstripsChartMsg, *ChartMsg; 
	TSimplePointSeries::DataImportMsg *CHM1, *darkChartMsg,*cupolChartMsg, *stripsChartMsg;
	CHM2=AdarkChartMsg=AcupolChartMsg=AstripsChartMsg=ChartMsg=NULL; 
	CHM1=darkChartMsg=cupolChartMsg=stripsChartMsg=NULL;
	LogMessage *log=new LogMessage(); log->CreateEntry("Log","Speed tests results",LogMessage::low_pr);

	if(!(parent->dark.org.HasImage() && parent->cupol.org.HasImage() && parent->strips.org.HasImage())) return;

	if(Xmin<0) Xmin=0;
	if(Xmin>=parent->strips.org.w) Xmin=parent->strips.org.w;
	if(Xmax<0) Xmax=0;
	if(Xmax>=parent->strips.org.w) Xmax=parent->strips.org.w;
	if(Xmax<Xmin) {int t=Xmax; Xmax=Xmin; Xmin=t;}	

	Timer2.Start();
	T.Format("%d",midl); T1.Format("%davrg",midl);

	for(int i=0;i<3;i++)
	{
		CHM2 =new TPointVsErrorSeries::DataImportMsg();		

		switch(i)
		{
		case 0: AdarkChartMsg=CHM2; break;
		case 1: AcupolChartMsg=CHM2; break;
		case 2: AstripsChartMsg=CHM2; break;
		}
	}

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
	
    Timer1.Start();	
	parent->dark.org.LoadBitmapArray(min,max); 
	parent->cupol.org.LoadBitmapArray(min,max);
	parent->strips.org.LoadBitmapArray(min,max);
	Timer1.Stop(); time=Timer1.GetValue();
	logT.Format("D C S %d lines load = %s",2*dd+1,ConvTimeToStr(time)); log->Msgs.Add(logT);
	dark=parent->dark.org.arr; 	cupol=parent->cupol.org.arr; strips=parent->strips.org.arr;

	SimplePoint pnt; pnt.type.Set(GenericPnt); 
	ValuesAccumulator pnteD,pnteC,pnteS; PointVsError pnte; 
	pnteD.type.Set(AveragePnt);	pnteC.type.Set(AveragePnt);	pnteS.type.Set(AveragePnt);	
	
	Timer1.Start();	
	for(int i=Xmin;i<Xmax;i++)
	{
		pnteD.Clear();pnteC.Clear();pnteS.Clear();		
		for(int j=1;j<=dd;j++)
		{
            pnteD << dark[i+(mm+j)*w] << dark[i+(mm-j)*w];
			pnteC << cupol[i+(mm+j)*w] << cupol[i+(mm-j)*w];
			pnteS << strips[i+(mm+j)*w] << strips[i+(mm-j)*w];
		}
		pnt.x=i;
		pnt.y=dark[i+mm*w]; pnteD << pnt.y; 
		pnt.y=cupol[i+mm*w]; pnteC << pnt.y;
		pnt.y=strips[i+mm*w]; pnteS << pnt.y;

		pnte=pnteD; pnte.x=i; AdarkChartMsg->Points.Add(pnte);
		pnte=pnteC; pnte.x=i; AcupolChartMsg->Points.Add(pnte);
		pnte=pnteS; pnte.x=i; AstripsChartMsg->Points.Add(pnte);
		
	}
	Timer1.Stop(); time=Timer1.GetValue();
	logT.Format("D C S %d lines averaging time=%s",2*dd+1,ConvTimeToStr(time)); log->Msgs.Add(logT);
    
	Timer1.Start();
	for(int i=0;i<AdarkChartMsg->Points.GetSize();i++)
	{
		pnte=(AstripsChartMsg->Points[i]-AdarkChartMsg->Points[i])/(AcupolChartMsg->Points[i]-AdarkChartMsg->Points[i]);
        pnte.type.Set(GenericPnt);
		ChartMsg->Points.Add(pnte);		
	}
	Timer1.Stop(); time=Timer1.GetValue();
	logT.Format("Normalizing %d point time=%s",AdarkChartMsg->Points.GetSize(),ConvTimeToStr(time)); log->Msgs.Add(logT);

	parent->dark.org.UnloadBitmapArray(); parent->cupol.org.UnloadBitmapArray(); parent->strips.org.UnloadBitmapArray();	

	delete AdarkChartMsg; delete AcupolChartMsg; delete AstripsChartMsg; 
	ChartMsg->Dispatch();


	Timer2.Stop(); time=Timer2.GetValue();
	logT.Format("Total processing time=%s",ConvTimeToStr(time)); log->Msgs.Add(logT);

	CKSVU3App* Parent=(CKSVU3App*)AfxGetApp(); 
	Parent->myThread.PostParentMessage(UM_GENERIC_MESSAGE,log);	
}

ImageWndPicWnd::ImageWndPicWnd()
{
	Parent=0;
}

ImageWndPicWnd::~ImageWndPicWnd()
{

}
BEGIN_MESSAGE_MAP(ImageWndPicWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_DROPFILES()
//	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_COMMAND(CaptureBtnID, OnMvButton)
	ON_MESSAGE(UM_CAPTURE_READY,OnCaptureReady)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_PICWNDMENU_ERASE, OnPicWndErase)
	ON_COMMAND(ID_PICWNDMENU_SAVE, OnPicWndSave)

	
//	ON_WM_ERASEBKGND()
	ON_WM_MOVE()
END_MESSAGE_MAP()

int ImageWndPicWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
    Parent=(ImageWnd*)GetParent();
	CRect r; GetClientRect(&r);
	ava.Create(this,r.Width(),r.Height(),24); 
	EraseAva();
	DragAcceptFiles(true);
	font1.CreatePointFont(80,"Arial"); 
	ScanLine.curL.x=0; ScanLine.curR.x=r.right;
	CRect t(CPoint(0,0),CSize(150,20)); t.OffsetRect(r.CenterPoint()-t.CenterPoint());
	CString T; GetWindowText(T); T="Capture "+T+" image";
	CaptureButton.Create(T,BS_CENTER | BS_TEXT | WS_VISIBLE | BS_PUSHBUTTON, t,this,CaptureBtnID);
	menu1.LoadMenu(IDR_MENU3);
	return 0;
}

void ImageWndPicWnd::EraseAva()
{
	CString T,T1;  GetWindowText(T1); T.Format("Drag or Capture %s image",T1);
	CRect r; GetClientRect(&r); ScanLine.Erase();
	ava.PatBlt(WHITENESS); ava.Rectangle(r);
	ava.TextOut(10,10,T);
}

void ImageWndPicWnd::OnPaint()
{
	CPaintDC dc(this); 	
	HDC hdc=dc.GetSafeHdc();
	if(org.HasImage())
	{
		if(ScanLine.curL.y>=0 && ScanLine.curR.y>=0) ScanLine.Draw(&ava);
	}
	ava.CopyTo(hdc,TOP_LEFT);
}

void ImageWndPicWnd::UpdateNow(void)
{
	RedrawWindow(0,0,RDW_INVALIDATE | RDW_FRAME | RDW_NOERASE | RDW_ALLCHILDREN);					
}

void ImageWndPicWnd::LoadPic(CString T)
{
	if(org.LoadImage(T)==S_OK)
	{
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

void ImageWndPicWnd::OnDropFiles(HDROP hDropInfo)
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

#define ScanLineXShift 20
void PicWndScanLine::Draw( BMPanvas* canvas )
{
	canvas->SetROP2(R2_NOT);
	Erase();
	canvas->MoveTo(curL); canvas->LineTo(curR); 
	canvas->MoveTo(curL+CPoint(ScanLineXShift,dy)); canvas->LineTo(curR+CPoint(-ScanLineXShift,dy)); 
	canvas->MoveTo(curL+CPoint(ScanLineXShift,-dy)); canvas->LineTo(curR+CPoint(-ScanLineXShift,-dy)); 
	ToErase=TRUE; lastL=curL; lastR=curR; buf=canvas; lastdy=dy;
	canvas->SetROP2(R2_COPYPEN); 			
}

void PicWndScanLine::Erase()
{
	if(ToErase==TRUE) 
	{
		if(buf!=NULL)
		{
			buf->MoveTo(lastL); buf->LineTo(lastR); 
			buf->MoveTo(lastL+CPoint(ScanLineXShift,lastdy)); buf->LineTo(lastR+CPoint(-ScanLineXShift,lastdy)); 
			buf->MoveTo(lastL+CPoint(ScanLineXShift,-lastdy)); buf->LineTo(lastR+CPoint(-ScanLineXShift,-lastdy)); 
			ToErase=FALSE;
		}		
	}
}

void ImageWndPicWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	BOOL update=FALSE; Parent->Ctrls.UpdateData();
	if(org.HasImage()==FALSE) return;
	switch(nFlags)
	{
	case MK_SHIFT: 
		ScanLine.curL.x=point.x; Parent->Ctrls.Xmin=point.x*Parent->scale; update=TRUE; 
		ScanLine.dy=Parent->Ctrls.AvrRange/Parent->scale;		
		break;
	case 0: 
		ScanLine.curL.y=ScanLine.curR.y=point.y; 
		ScanLine.curL.x=Parent->Ctrls.Xmin/Parent->scale;
		ScanLine.curR.x=Parent->Ctrls.Xmax/Parent->scale;
		update=TRUE; 
		Parent->Ctrls.stroka=point.y*Parent->scale; 
		ScanLine.dy=Parent->Ctrls.AvrRange/Parent->scale;		
		break;
	case MK_CONTROL:
		ScanLine.curL.y=ScanLine.curR.y=point.y; update=TRUE; 
		if(org.HasImage()==FALSE) break;
		int y=point.y*Parent->scale;
		if(y<0) y=0;
		if(y>=org.h) y=org.h-1;
		Parent->Ctrls.stroka=y;
		Parent->Ctrls.UpdateData(0);
		Parent->Ctrls.OnBnClickedScan();		
		break;
	}
	if(update) 
	{		
		Parent->DrawScanLine(ScanLine.curL, ScanLine.curR);		
		Parent->Ctrls.UpdateData(0);
	}
	CWnd::OnLButtonUp(nFlags, point);
}

void ImageWndPicWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
	BOOL update=FALSE;
	switch(nFlags)
	{
	case MK_SHIFT: ScanLine.curR.x=point.x; Parent->Ctrls.Xmax=point.x*Parent->scale; update=TRUE; break;
	default: CWnd::OnRButtonUp(nFlags, point);
	}
	if(update) 	
	{		
		Parent->DrawScanLine(ScanLine.curL, ScanLine.curR);
		Parent->Ctrls.UpdateData(0);
	}
}

void ImageWndPicWnd::OnMvButton()
{
	CaptureButton.EnableWindow(FALSE);
    Parent->CameraWnd.PostMessage(UM_CAPTURE_REQUEST,(WPARAM)this, (LPARAM)&org);
}

LRESULT ImageWndPicWnd::OnCaptureReady( WPARAM wParam, LPARAM lParam )
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
void ImageWndPicWnd::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if(org.HasImage())
	{
		CMenu* menu = menu1.GetSubMenu(0);
		menu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
	}
}

void ImageWndPicWnd::OnPicWndErase()
{
	if(org.HasImage())
	{
		org.Destroy(); EraseAva();

		UpdateNow();
		CaptureButton.ShowWindow(SW_SHOW); DragAcceptFiles(true);
	}
}

void ImageWndPicWnd::OnPicWndSave()
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

void ImageWndCtrlsTab::OnBnClickedCalibrate()
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

int ImageWndCtrlsTab::OnCreate( LPCREATESTRUCT lpCreateStruct )
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

void * ImageWnd::GetChartFromParent()
{
	return ((CMainFrame*)GetParent())->GetChartFromParent();
}

void ImageWndPicWnd::OnMove(int x, int y)
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


BOOL ImageWndCtrlsTab::DestroyWindow()
{
	CalibratorDlg.DestroyWindow();

	return BarTemplate::DestroyWindow();
}

void ImageWndCtrlsTab::OnBnClickedCalcTE()
{
	CalcTEDlg.ShowWindow(SW_SHOW);
}

void ImageWndCtrlsTab::OnBnClickedCalcTM()
{
	CalcTMDlg.ShowWindow(SW_SHOW);
}


void ImageWndCtrlsTab::OnBnClickedButton5()
{
	ImageWnd *parent=(ImageWnd*)Parent;
	parent->dark.PostMessage(WM_COMMAND,ID_PICWNDMENU_ERASE,0);
	parent->cupol.PostMessage(WM_COMMAND,ID_PICWNDMENU_ERASE,0);
	parent->strips.PostMessage(WM_COMMAND,ID_PICWNDMENU_ERASE,0);
}

void ImageWndCtrlsTab::OnEnKillfocusEdit1()
{
	// TODO: Add your control notification handler code here
}

void * ImageWndCtrlsTab::GetChartFromParent()
{
	CWnd *t = GetParent(); //ctabctrl
	CWnd *t1 = t->GetParent(); //cdialogbar
	CWnd *t2 = t1->GetParent(); //cmainframe
	return ((ImageWnd*)GetParent())->GetChartFromParent();
}

