// captureWnd.cpp : implementation file
//

#include "stdafx.h"
#include "captureWnd.h"
#include "my_color.h"
//#include "ml44ctrl.h"
//#include "Monochromator.h"
#include "KSVU3.h"
#include "ImageWnd.h"
// CaptureWnd

IMPLEMENT_DYNAMIC(CaptureWnd, CWnd)
CaptureWnd::CaptureWnd(): thrd(444)
{
	Src=NULL;
	CameraOutWnd=CRect(CPoint(0,0),CSize(800,600));
	LevelsScanWnd=CRect(CPoint(0,600+10),CSize(800,200));
	penRed.CreatePen(PS_SOLID,1,clRED);
	penBlue.CreatePen(PS_SOLID,1,clBLUE);
	penGreen.CreatePen(PS_SOLID,1,clGREEN);
}

CaptureWnd::~CaptureWnd()
{
	if(Src!=NULL) 
	{
		delete Src; Src=NULL;
	}
}


BEGIN_MESSAGE_MAP(CaptureWnd, CWnd)
	ON_WM_CREATE()
	ON_MESSAGE(UM_DATA_UPDATE,OnDataUpdate)	
	ON_WM_PAINT()
	ON_MESSAGE(UM_CAPTURE_REQUEST,OnCaptureRequest)
	ON_WM_DESTROY()
END_MESSAGE_MAP()



// CaptureWnd message handlers


BEGIN_MESSAGE_MAP(CaptureWndCtrlsTab, CDialog)
	//{{AFX_MSG_MAP(DialogBarTab1)
	//}}AFX_MSG_MAP	
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClicked_Capture)
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClicked_StopCapture)
	ON_BN_CLICKED(IDC_BUTTON3, OnBnClicked_PauseCapture)
	ON_BN_CLICKED(IDC_BUTTON4, OnBnClicked_ResumeCapture)
	ON_BN_CLICKED(IDC_BUTTON5, OnBnClicked_FilterParams)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_BUTTON6, OnBnClickedChooseCam)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_RADIO1, &CaptureWndCtrlsTab::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO4, &CaptureWndCtrlsTab::OnBnClickedRadio4)
END_MESSAGE_MAP()

int CaptureWndCtrlsTab::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	int ret=0;
	if (CWnd::OnCreate(lpCreateStruct) == -1) ret=-1;
	return ret;
}

BOOL CaptureWndCtrlsTab::OnInitDialog()
{
	CaptureWnd *pParent=(CaptureWnd*)Parent; 
	CDialog::OnInitDialog();

	BtnCapture.EnableWindow(TRUE);
	BtnStop.EnableWindow(FALSE);
	BtnPause.EnableWindow(FALSE);
	BtnResume.EnableWindow(FALSE);
	BtnFilterParams.EnableWindow(FALSE);
	PreviewSize.AddString("3264x2448");
	PreviewSize.AddString("1600x1200");
	PreviewSize.AddString("1280x960");
	PreviewSize.AddString("1024x768");
	PreviewSize.AddString("640x480");
	PreviewSize.SetCurSel(3);
	return TRUE; 
}

eDcm800Size CaptureWndCtrlsTab::GetPreviewSize()
{
	eDcm800Size ret;
	switch (PreviewSize.GetCurSel())
	{
	case 0: ret=eDcm800Size_3264_2448;	break;
	case 1: ret=eDcm800Size_1600_1200;	break;
	case 2: ret=eDcm800Size_1280_960;	break;
	case 3: ret=eDcm800Size_1024_768;	break;
	case 4: 
	default:
			ret=eDcm800Size_640_480;	break;
	}
	return ret;
}

void CaptureWndCtrlsTab::OnBnClicked_Capture()
{
	UpdateData();

	CaptureWnd *pParent=(CaptureWnd*)Parent; CString T;	LogMessage *log=NULL; 
	if(pParent->Src==NULL) pParent->SelectCaptureSrc(pParent->Ctrls.Chooser.CaptureName);
	if(pParent->Src==NULL) 
	{
		log=new LogMessage(); 
		T.Format(" No cameras found at all"); log->CreateEntry("ERR",T,LogMessage::high_pr);
		log->Dispatch(); return;
	}
	if(pParent->Src->status==S_OK)
	{
		pParent->Timer1.Start(); 
		pParent->cntr=0;

		pParent->thrd.params.Parent.pThrd=AfxGetThread();
		pParent->thrd.params.Parent.pWND=pParent;
		pParent->thrd.params.Pbuf=&pParent->Pbuf;
		pParent->thrd.params.LevelScanBuf=&pParent->LevelsScanBuf;
		pParent->thrd.params.Src=pParent->Src;
		pParent->thrd.params.StopCapture.ResetEvent();
		pParent->thrd.params.PauseCapture.ResetEvent();
		pParent->thrd.params.ResumeCapture.ResetEvent();
		pParent->thrd.params.ShowFilterParams.ResetEvent();
		pParent->thrd.params.size=GetPreviewSize();
		pParent->thrd.params.thrd=&pParent->thrd;

		CRect r=((CaptureWnd*)Parent)->CameraOutWnd;
		pParent->grayscaleBuf.Create(this,r.Width(),r.Height(),8);
		pParent->grayscaleBuf.CreateGrayPallete();		
		pParent->truecolorBuf.Create(this,r.Width(),r.Height(),24);		

		pParent->thrd.Start();	
		BtnCapture.EnableWindow(FALSE);
		BtnStop.EnableWindow(TRUE);
		BtnPause.EnableWindow(TRUE);
		BtnResume.EnableWindow(FALSE);
		BtnChooseCam.EnableWindow(FALSE);
		BtnFilterParams.EnableWindow(TRUE);
	}		
	else 
	{
		log=new LogMessage(); 
		T.Format(" Please CHOOSE valid CAMera"); log->CreateEntry("ERR",T,LogMessage::high_pr); 
		log->Dispatch(); return;
	}	
	return;
}

void CaptureWndCtrlsTab::OnBnClicked_StopCapture()
{
	CaptureWnd *pParent=(CaptureWnd*)Parent;
	pParent->thrd.params.StopCapture.SetEvent();  
	BtnCapture.EnableWindow(TRUE);
	BtnStop.EnableWindow(FALSE);
	BtnPause.EnableWindow(FALSE);
	BtnResume.EnableWindow(FALSE);
	BtnChooseCam.EnableWindow(TRUE);
	BtnFilterParams.EnableWindow(FALSE);
}

void CaptureWndCtrlsTab::OnBnClicked_PauseCapture()
{
	CaptureWnd *pParent=(CaptureWnd*)Parent;
	pParent->thrd.params.PauseCapture.SetEvent();  
	BtnCapture.EnableWindow(FALSE);
	BtnStop.EnableWindow(TRUE);
	BtnPause.EnableWindow(FALSE);
	BtnResume.EnableWindow(TRUE);
}

void CaptureWndCtrlsTab::OnBnClicked_ResumeCapture()
{
	CaptureWnd *pParent=(CaptureWnd*)Parent;
	pParent->thrd.params.ResumeCapture.SetEvent();    
	BtnCapture.EnableWindow(FALSE);
	BtnStop.EnableWindow(TRUE);
	BtnPause.EnableWindow(TRUE);
	BtnResume.EnableWindow(FALSE);
}

void CaptureWndCtrlsTab::OnBnClicked_FilterParams()
{
	CaptureWnd *pParent=(CaptureWnd*)Parent;
	pParent->thrd.params.ShowFilterParams.SetEvent();    
}

void CaptureWnd::SelectCaptureSrc(CString name)
{	
	MyEnumerator CaptureSrc; MonikersArray arr;
	CaptureSrc.FindSource(name,arr); CString T;
//	ASSERT(arr.GetSize()==1);

	for (int i=0;i<arr.GetSize();i++)
	{
		if(Src!=NULL) {delete Src; Src=NULL;}
		if(name=="ScopeTek DCM800") 
		{
			Src=new ScopeTek_DCM800();
			Ctrls.PreviewSize.EnableWindow(TRUE);
		}
		else 
		{
			Src=new DSCaptureSource(); 
			Ctrls.PreviewSize.EnableWindow(FALSE);
		}
		Src->Create(arr[i]);
		if(Src->status!=S_OK)
		{			
			LogMessage *log=new LogMessage(); 
			T.Format(" %s - INIT ERROR",name);
			log->CreateEntry("*",T,LogMessage::high_pr);
			log->Dispatch();
		}
		else Src->Name=name;
	}	
}

int CaptureWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	Ctrls.Parent=this;

	font1.CreatePointFont(90,"Arial"); 

	//for(int i=0;i<256;i++) pal[i]=HSLColor((int)(i*360./256.));
	int range_min=250;
	for(int i=0;i<range_min;i++) pal[i].rgbBlue=pal[i].rgbGreen=pal[i].rgbRed=i;
	for(int i=range_min;i<256;i++) 
		pal[i]=HSLColor(0,127,(i-range_min)/(256.-range_min));

	void *x;
	if((x=LevelsScanBuf.GainAcsess(WRITE))!=NULL)
	{
		BMPanvasGuard guard1(x); BMPanvas &buf(guard1);
		buf.Create(this,LevelsScanWnd.Width(),LevelsScanWnd.Height(),24);
	}

	Timer1.Start(); cntr=0;

	return 0;
}

LRESULT CaptureWnd::OnDataUpdate(WPARAM wParam, LPARAM lParam )
{
	if(lParam!=0)
	{
		MessageForWindow* msg=(MessageForWindow*)lParam;
		delete msg;
	}
	if(wParam!=0)
	{
		AbstractMessage* msg=(AbstractMessage*)wParam;
		delete msg;
	}
	RedrawWindow(0,0,RDW_INVALIDATE | RDW_NOERASE | RDW_NOFRAME | RDW_ALLCHILDREN);
	return 0;
}

void ColorTransform(BMPanvas *color, BMPanvas *grayscale, CaptureWndCtrlsTab::ColorTransformModes mode)
{	
	RGBTRIPLE *colorCurrentPoint; int i,j; BYTE *colorLineBegin=NULL, *bwCurrentPoint=NULL, *bwLineBegin=NULL;
	int (*LuminosityFunc)(RGBTRIPLE&)=NULL;
	
	switch(mode)
	{
	case CaptureWndCtrlsTab::HSL: LuminosityFunc=getL_HSL; break;
	case CaptureWndCtrlsTab::HSV: LuminosityFunc=getL_HSV; break;
	case CaptureWndCtrlsTab::NativeGDI: color->CopyTo(grayscale,TOP_LEFT); return;
	}
	if(LuminosityFunc==NULL) return;

	color->LoadBitmapArray(); colorLineBegin=bwLineBegin=color->arr;
	for(i=0;i<color->h;i++)
	{
		colorCurrentPoint=(RGBTRIPLE *)colorLineBegin; bwCurrentPoint=bwLineBegin;
		for(j=0;j<color->w;j++)
		{
			(*bwCurrentPoint) = LuminosityFunc(*colorCurrentPoint);
			colorCurrentPoint++; bwCurrentPoint++;
		}
		colorLineBegin += color->wbyte; bwLineBegin += grayscale->wbyte;
	}
	grayscale->SetBitmapArray(BMPanvas::MIN_SCANLINE,BMPanvas::MAX_SCANLINE,color->arr);
	color->UnloadBitmapArray();

}

void CaptureWnd::ScanLevels(BMPanvas* src,BMPanvas *dest)
{
	RGBTRIPLE *col; int i;  HGDIOBJ t; double l; MyTimer time1; time1.Start(); ms dt;CString T;
	src->LoadBitmapArray(src->h/2,src->h/2); dest->PatBlt(BLACKNESS);

	col=(RGBTRIPLE*)src->arr;
	for(i=0;i<src->w;i++)
	{
		t=dest->SelectObject(penBlue); l=(dest->h-1)*(255-(*col).rgbtBlue)/255.; 
		dest->MoveTo(i,dest->h-1); dest->LineTo(i,(int)l);
		dest->SelectObject(penRed); l=(dest->h-1)*(255-(*col).rgbtRed)/255.; 
		dest->MoveTo(i,dest->h-1); dest->LineTo(i,(int)l);
		dest->SelectObject(penGreen); l=(dest->h-1)*(255-(*col).rgbtGreen)/255.; 
		dest->MoveTo(i,dest->h-1); dest->LineTo(i,(int)l);
		dest->SelectObject(t); 
		col++;
	}
	src->UnloadBitmapArray();
	HGDIOBJ tf=dest->SelectObject(font1); dest->SetBkMode(TRANSPARENT); dest->SetTextColor(clWHITE);
	dt=time1.StopStart(); T.Format("%.2f ms",dt.val()); dest->TextOut(0,0,T);
	dest->SelectObject(tf);
}

void CaptureWnd::OnPaint()
{	
	CPaintDC dc(this); Ctrls.UpdateData();
	CRect tr; GetClientRect(&tr); HDC hdc=dc.GetSafeHdc();
	
    if(grayscaleBuf.GetDC()==NULL) return;
	void* x,* xx; CString T; sec dt1; MyTimer Timer2;
	if((x=Pbuf.GainAcsess(READ))!=NULL)
	{
		Timer2.Start();
		BMPanvasGuard guard1(x); BMPanvas &buf(guard1);
		BMPanvas* tbuf=&buf;
		dt1=Timer1.StopStart();		

		CRect rgn1=buf.Rgn, rgn2=grayscaleBuf.Rgn, trgn;
		double r1=(double)rgn1.Width()/rgn2.Width(), r2=(double)rgn1.Height()/rgn2.Height(), r;
		r=(r1>r2 ? r1:r2);
		if(r>1) rgn1=CRect(0,0,(int)(rgn1.Width()/r),(int)(rgn1.Height()/r));
		else rgn1.OffsetRect(rgn2.CenterPoint()-rgn1.CenterPoint());

		tbuf=&truecolorBuf;
		if(r>1) { SetStretchBltMode(tbuf->GetDC(),COLORONCOLOR); buf.StretchTo(tbuf,rgn1,buf.Rgn,SRCCOPY); }
		else buf.CopyTo(tbuf,rgn1);

		if((xx=LevelsScanBuf.GainAcsess(WRITE))!=NULL)
		{
			BMPanvasGuard guard1(xx); BMPanvas &buf(guard1);
			ScanLevels(tbuf,&buf);
			buf.CopyTo(hdc,LevelsScanWnd.TopLeft());
		}			

		if(Ctrls.ColorTransformSelector!=CaptureWndCtrlsTab::TrueColor)
		{
			ColorTransform(tbuf, &grayscaleBuf, Ctrls.ColorTransformSelector);
			tbuf=&grayscaleBuf;

			CaptureRequestStack::Item request;
			while(Stack >> request)
			{
				if((*request.buf)!=buf)
				{
					request.buf->Create(buf.GetDC(),buf.w,buf.h,8);
					request.buf->CreateGrayPallete();
				}
				buf.CopyTo(request.buf,TOP_LEFT);
				ColorTransform(&buf, request.buf, Ctrls.ColorTransformSelector);
				request.sender->PostMessage(UM_CAPTURE_READY,0,0);
			}

		}


		BMPanvasTAGSmk1* tags=(BMPanvasTAGSmk1*)buf.tags;
		CPoint TextOutput; TextOutput=rgn1.TopLeft();

		HGDIOBJ tf=tbuf->SelectObject(font1); tbuf->SetBkMode(TRANSPARENT); tbuf->SetTextColor(RGB(255,255,0));
		T.Format("%dx%dx%d %.2f sec",tags->i1,tags->i2,tags->i3,tags->d1); tbuf->TextOut(TextOutput.x,TextOutput.y+0,T);
		T.Format("DShow fps=%g",tags->d2); tbuf->TextOut(TextOutput.x,TextOutput.y+10,T);

		T.Format("Buffer process = %.2f+%.2f ms",tags->timel.val(),Timer2.StopStart().val()); tbuf->TextOut(TextOutput.x,TextOutput.y+30,T);		
		T.Format("%d eqv fps = %g",cntr++,(1./dt1.val()));  tbuf->TextOut(TextOutput.x,TextOutput.y+40,T);
		tbuf->SelectObject(tf);
		
		if(Ctrls.ColorTransformSelector!=CaptureWndCtrlsTab::TrueColor) grayscaleBuf.SetPallete(pal); 
		tbuf->CopyTo(hdc,TOP_LEFT);
		if(Ctrls.ColorTransformSelector!=CaptureWndCtrlsTab::TrueColor) grayscaleBuf.CreateGrayPallete();
	}
	else ASSERT(0);
}
/*
struct Slit 
{
	DWORD NumSlitWidths, SlitWidthInd, ID;
	double *widths;

	Slit(DWORD id) {ID=id; widths=NULL;}
	{
		NumSlitWidths=ml44_GetNumSlitWidths(ID);
		SlitWidthInd=ml44_GetSlitWidthInd(ID);
		widths=new double[NumSlitWidths];
		for(DWORD i=0;i<NumSlitWidths;i++)
		{
			widths[i]=ml44_GetSlitWidth(ID,i);
		}
	}
	~Slit() {if(widths!=NULL) delete widths;}

};
*/
//void CaptureWndCtrlsTab::OnBnClickedButton6()
//{
//	bool ret=0; DWORD err,clbr,stps; double wl,wlchng, Min,Max,disp,range;
//	Slit EnterSlit(ENTR_SLIT), ExitSlit(EXIT_SLIT); TGratingPrm Grating0,Grating1;
//	if(ml44_Init(PORT_COM4,0)!=0)
//	{
//		if(ml44_HitTest()!=0)
//		{
//			ml44_GetWLRange(&Min,&Max); err=ml44_GetLastError();
//			disp=ml44_GetDispersion(); err=ml44_GetLastError();
//			range=ml44_GetSpcRange(); err=ml44_GetLastError();
//			clbr=ml44_GetClbrType(); err=ml44_GetLastError();
//			wl=ml44_GetWaveLength(); err=ml44_GetLastError();
//			wlchng=ml44_GetChangeGratingWL(); err=ml44_GetLastError();
//			ret=ml44_GetGratingPrms(&Grating0,0); err=ml44_GetLastError();
//			ret=ml44_GetGratingPrms(&Grating1,1); err=ml44_GetLastError();
//			stps=ml44_GetStepsByWL(600,0); err=ml44_GetLastError();
//			stps=ml44_GetGratingStepPos(); err=ml44_GetLastError();
//
//			EnterSlit.Init();
//			ExitSlit.Init();
//
//		}
//		else err=ml44_GetLastError();
//		ml44_Free();
//	}
//	else err=ml44_GetLastError();
//}

LRESULT CaptureWnd::OnCaptureRequest( WPARAM wParam, LPARAM lParam )
{
	Stack << CaptureRequestStack::Item((CWnd*)wParam,(BMPanvas*)lParam);
	return 0;
}
void CaptureWndCtrlsTab::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogBarTab1)
	DDX_Control(pDX, IDC_BUTTON1, BtnCapture);
	DDX_Control(pDX, IDC_BUTTON2, BtnStop);
	DDX_Control(pDX, IDC_BUTTON3, BtnPause);
	DDX_Control(pDX, IDC_BUTTON4, BtnResume);
	DDX_Control(pDX, IDC_BUTTON6, BtnChooseCam);
	DDX_Control(pDX, IDC_COMBO1, PreviewSize);
	DDX_Control(pDX, IDC_BUTTON5, BtnFilterParams);
	DDX_Radio(pDX, IDC_RADIO1, (int&)ColorTransformSelector);
	//}}AFX_DATA_MAP
}


void CaptureWndCtrlsTab::OnBnClickedChooseCam()
{
	CaptureWnd *pParent=(CaptureWnd*)Parent;
	if(Chooser.DoModal()==IDOK)
	{
		pParent->SelectCaptureSrc(Chooser.CaptureName);
	}	
}

void CaptureWndCtrlsTab::OnCbnSelchangeCombo1()
{
	// TODO: Add your control notification handler code here
}

void CaptureWnd::OnDestroy()
{
	__super::OnDestroy();

	font1.DeleteObject();
}

void CaptureWndCtrlsTab::OnBnClickedRadio1()
{
	// TODO: Add your control notification handler code here
}

void CaptureWndCtrlsTab::OnBnClickedRadio4()
{
	int a=5;
}
