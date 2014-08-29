// DialogBarTab1.cpp : implementation file
//

#include "stdafx.h"
#include "DialogBarTab1.h"
#include "MainFrm.h"
#include "mytime.h"
//#include "mythread1.h"
//#include "abstractcomstage.h"
//#include "warningdialog.h"
#include "metricon.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(MainChartWnd, TChart)
MainChartWnd::MainChartWnd(): TChart("Chart1")
{
}
/////////////////////////////////////////////////////////////////////////////
// DialogBarTab1 dialog
BEGIN_MESSAGE_MAP(MainChartWnd, TChart)
	//{{AFX_MSG_MAP(DialogBarTab1)
	ON_MESSAGE(UM_STAGE_UPDATE, OnStageUpdate)
	ON_MESSAGE(UM_STOP,OnStop)	
	ON_MESSAGE(UM_PAUSE,OnPause)	
	ON_MESSAGE(UM_START,OnStart)	
	ON_MESSAGE(UM_CONTINUE,OnContinue)	
//	ON_MESSAGE(UM_SERIES_UPDATE,OnSeriesUpdate)	
	//}}AFX_MSG_MAP	
	ON_WM_CREATE()
END_MESSAGE_MAP()

int MainChartWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (TChart::OnCreate(lpCreateStruct) == -1)
		return -1;
	Panel.Parent=this;
	return 0;
}

LRESULT  MainChartWnd::OnStageUpdate(WPARAM wParam, LPARAM lParam )
{
	Panel.PostMessage(UM_STAGE_UPDATE,wParam,lParam); return 0;
}

LRESULT  MainChartWnd::OnStop(WPARAM wParam, LPARAM lParam )
{	
	MyThread::ParentStatusReport* rep=(MyThread::ParentStatusReport*)wParam;
	wParam=rep->ConfigCopy.PersonalID; delete rep;

	switch (wParam)
	{
	case MainTaskID: Panel.PostMessage(UM_STOP,wParam,lParam); break;
	}
	return 0;
}

LRESULT  MainChartWnd::OnPause(WPARAM wParam, LPARAM lParam )
{	
	MyThread::ParentStatusReport* rep=(MyThread::ParentStatusReport*)wParam;
//	wParam=rep->ConfigCopy.PersonalID; 
//	delete rep;

	switch (rep->ConfigCopy.PersonalID)
	{
	case MainTaskID: Panel.SendMessage(UM_PAUSE,wParam,lParam); break;
	}
	MainFrame.pWND->PostMessage(UM_PAUSE,wParam,lParam);
	return 0;
}

LRESULT MainChartWnd::OnStart(WPARAM wParam,LPARAM lParam)
{
	MyThread::ParentStatusReport* rep=(MyThread::ParentStatusReport*)wParam;
//	wParam=rep->ConfigCopy.PersonalID; 
	delete rep;

	switch (wParam)
	{
	case MainTaskID: Panel.PostMessage(UM_START,wParam,lParam); break;
	}
	return 0;
}

void MainChartWnd::Serialize(CArchive& ar)
{		
	Panel.Serialize(ar);
}

LRESULT MainChartWnd::OnContinue( WPARAM wParam, LPARAM lParam )
{
	MyThread::ParentStatusReport* rep=(MyThread::ParentStatusReport*)wParam;

	switch (rep->ConfigCopy.PersonalID)
	{
	case MainTaskID: Panel.SendMessage(UM_CONTINUE,wParam,lParam); break;
	}
	MainFrame.pWND->PostMessage(UM_CONTINUE,wParam,lParam);
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// DialogBarTab1 dialog
BEGIN_MESSAGE_MAP(DialogBarTab1, BarTemplate)
	//{{AFX_MSG_MAP(DialogBarTab1)
	ON_EN_KILLFOCUS(IDC_LMAX_EDIT, OnKillfocus)
	ON_EN_KILLFOCUS(IDC_STEP_EDIT, OnKillfocus)
	ON_EN_KILLFOCUS(IDC_SCANSPEED_EDIT, OnKillfocus)
	ON_EN_KILLFOCUS(IDC_QUANTITY_EDIT, OnKillfocus)
	ON_EN_KILLFOCUS(IDC_LMIN_EDIT, OnKillfocus)
	ON_EN_KILLFOCUS(IDC_INTERVAL_EDIT, OnKillfocus)
//	ON_MESSAGE(UM_STAGE_UPDATE, OnStageUpdate)
//	ON_MESSAGE(UM_STOP,OnStop)	
//	ON_MESSAGE(UM_START,OnStart)	
//	ON_MESSAGE(UM_PAUSE,OnPause)	
	ON_WM_CTLCOLOR()
	ON_WM_CREATE()
	ON_MESSAGE(UM_SERIES_UPDATE,OnSeriesUpdate)	
	//}}AFX_MSG_MAP	
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClicked_Fit)
	ON_BN_CLICKED(IDC_BUTTON5, OnBnClicked_Locate)
	ON_BN_CLICKED(IDC_BUTTON1, &DialogBarTab1::OnBnClickedLoadCalibration)
	ON_BN_CLICKED(IDC_BUTTON4, &DialogBarTab1::OnBnClickedButton4)
END_MESSAGE_MAP()

DialogBarTab1::DialogBarTab1(CWnd* pParent /*=NULL*/)
	: BarTemplate(pParent)//, StartButton()
	, spec_wdth(5)
	, X0(1200)
	, dX(20)
	, PolinomOrder(2)
	, minimum_widht_2(20)
	, Xmin(860)
	, Xmax(3000)
	, level(0.95)
{
	//Message1.body.Object="MainControl";
	//{{AFX_DATA_INIT(DialogBarTab1)
	LMax = 518.5;
	LMin = 514.7;
	ScanStep = 1;
	//}}AFX_DATA_INIT
	Name="MainControlTab";
//	Series=0;
	CheckButtonsStatus=0;
}


void DialogBarTab1::DoDataExchange(CDataExchange* pDX)
{
	BarTemplate::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogBarTab1)
//	DDX_Control(pDX, IDC_PROGRESS1, ProgressBar);
	DDX_Text(pDX, IDC_EDIT1, spec_wdth);
	DDV_MinMaxInt(pDX, spec_wdth, 1, 100);
	DDX_Text(pDX, IDC_EDIT4, X0);
	DDX_Text(pDX, IDC_EDIT5, dX);
	DDX_Text(pDX, IDC_EDIT2, PolinomOrder);
	DDV_MinMaxInt(pDX, PolinomOrder, 0, 100);
	//}}AFX_DATA_MAP
	DDX_Text(pDX, IDC_EDIT3, minimum_widht_2);
	DDV_MinMaxInt(pDX, minimum_widht_2, 1, 3000);
	DDX_Control(pDX, IDC_COMBO1, SeriesCombo);
	DDX_Text(pDX, IDC_EDIT7, Xmin);
	DDX_Text(pDX, IDC_EDIT8, Xmax);
	DDX_Text(pDX, IDC_EDIT9, level);
}

/////////////////////////////////////////////////////////////////////////////
// DialogBarTab1 message handlers
int DialogBarTab1::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if (BarTemplate::OnCreate(lpCreateStruct) == -1) return -1;

//	DWORD style=WS_CHILD | WS_VISIBLE | BS_ICON ;
//	StartButton.Create(0,style,CRect(CPoint(580,5),CSize(55,45)),this,IDC_START_BUTTON);
//	StopButton.Create(0,style,CRect(CPoint(580,50),CSize(55,45)),this,IDC_STOP_BUTTON);	
//	StartButton.Parent=StopButton.Parent=this;
	return 0;
}

BOOL DialogBarTab1::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
//	StartButton.bitmaps.Add(StartIcon); StartButton.bitmaps.Add(PauseIcon);	StartButton.bitmaps.Add(ResumeIcon);
//	StartButton.SetState(MainStagesCtrl::Start);
//	StopButton.bitmaps.Add(StopIcon);	
//	StopButton.SetState(0);

///	StartButton.ShowWindow(SW_HIDE);StopButton.ShowWindow(SW_HIDE);

//	font1.CreatePointFont(10,"Arial");
//	ProgressBar.SetFont(&font1); ProgressBar.SetRange32(0,100); 	

//	Channel1Check.SetCheck(true);
//	OnBnClickedCheck1();

//	StartButton.Parent=StopButton.Parent=this;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void DialogBarTab1::OnKillfocus() {UpdateData();}


void DialogBarTab1::Serialize(CArchive& ar) 
{
	if (ar.IsStoring())
	{	// storing code
		ar <<  LMax << LMin << ScanStep;
	}
	else
	{	// loading code
		ar >> LMax >> LMin >> ScanStep;
		UpdateData(0);
	}
}
/*
LRESULT  DialogBarTab1::OnStageUpdate(WPARAM wParam, LPARAM lParam )
{
	MyThread::ParentStatusReport* rep=(MyThread::ParentStatusReport*)wParam;
	wParam=rep->ConfigCopy.PersonalID; delete rep;
	SDRstage::UpdateMsg *msg=(SDRstage::UpdateMsg*)lParam;
	int n; CString T; 
	if(msg)
	{
		ProgressBar.Text=MyTime::ConvertToStr(msg->info.t-msg->cur.t); 
		n=(msg->cur.st*100)/msg->info.st;
		ProgressBar.SetPos(n);	
	}    
	delete msg;
	return 0;
}

LRESULT  DialogBarTab1::OnStop(WPARAM wParam, LPARAM lParam )
{	
	switch (wParam)
	{
	case MainTaskID: StartButton.PostMessage(UM_STOP,wParam,lParam);	break;
	}
	return 0;
}

LRESULT DialogBarTab1::OnStart(WPARAM wParam,LPARAM lParam)
{
	switch (wParam)
	{
	case MainTaskID:
//		ProgressBar.Text=MyTime::ConvertToStr(Info.t); ProgressBar.TextOut();
		break;
	}
	return 0;
}
*/

HBRUSH DialogBarTab1::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch (nCtlColor) 
	{
	case CTLCOLOR_STATIC   :
		if(pWnd==&Channel1Check) 
		{
			pDC->SetTextColor(RGB(255, 0, 0));	
			pDC->SetBkMode(TRANSPARENT);
			return GetSysColorBrush(COLOR_MENU);
		}
		if(pWnd==&Channel0Check) 
		{
			pDC->SetTextColor(RGB(0, 0, 255));	
			pDC->SetBkMode(TRANSPARENT);
			return GetSysColorBrush(COLOR_MENU);
		}
		
		break;
	}
	return BarTemplate::OnCtlColor(pDC, pWnd, nCtlColor);;
}

/*
LRESULT DialogBarTab1::OnPause( WPARAM wParam, LPARAM lParam )
{
	StartButton.PostMessage(UM_PAUSE,wParam,lParam);
	return 0;
}

//SDRstage::SetupParams DialogBarTab1::GetMotorSetupParams() {return SDRstage::SetupParams(nm(LMin),nm(LMax));}

void DialogBarTab1::CreateScenarioParams(ScenarioParams &Params)
{
	SDRstage *MS=0; fm10 MotorDL, templ1;
	UpdateData();
	Params.scanStep=nm(ScanStep); Params.mp=GetMotorSetupParams();
	MainCfg.Get(CFG_MESUREMENT_NUM,Params.vp.n); MainCfg.Get(CFG_MESUREMENT_TIME,Params.vp.dt); 
	MainCfg.Get(CFG_FILTER_MODE,Params.automatic);

	MS=StartButton.CreateMotorStage("Test"); 
	MS->Setup(&Params.mp); 
	Params.scanStep.MakeMultipleTo((MotorDL=MS->GetStep()));
	if(Params.scanStep<MotorDL) {Error* t=new Error(); Params.RegisterError(t->Create(ERR_SCANSTEP_TOO_SMALL,Name));}
	delete MS;

	Params.sign=(Params.mp.l1<Params.mp.l2 ? 1:-1); 
	if(Params.scanStep!=0) Params.numPoints=(int)(Abs(Params.mp.l2-Params.mp.l1)/Params.scanStep);
	templ1=Params.mp.l1+Params.scanStep*Params.sign*Params.numPoints; 
	if( (templ1*Params.sign) < (Params.mp.l2*Params.sign)) Params.numPoints++; 

	Params.curl=MainCfg.GetWavelength(); Params.scenl=Params.curl;
	Params.Series=&(((MainChartWnd*)Parent)->Series);
	MainCfg.Get(CFG_FILTER_POS,Params.filter_pos);
	if(!Params.automatic) Params.filter_pos=MainCfg.FilterHead.FindFilter(Params.scenl,Params.filter_pos); 
	Params.UpdateWnd.pThrd=AfxGetThread(); Params.UpdateWnd.pWND=Parent;
}

BEGIN_MESSAGE_MAP(MainStagesCtrl, MyButton)
	ON_MESSAGE(UM_ACCESS_GRANTED,OnAccessGranted)	
	ON_MESSAGE(UM_STOP,OnStop)	
	ON_MESSAGE(UM_PAUSE,OnPause)	
END_MESSAGE_MAP()

int MainStagesCtrl::Setup(void *p)
{
	int ret; ScenarioParams Params; Parent->CreateScenarioParams(Params);
	ScenarioStage *mOFF;  scenario stages, *temp; stages.AttachErrorsOF(&Params);		

	if(!Parent->CheckButtonsStatus) {Error* t=new Error(); stages.RegisterError(t->Create(ERR_NO_CHANNELS,Parent->Name));}

	if(stages.Errors()==0)
	{		
		stages << (temp=CreatePreWCHSubScenario(Params)); 
		Parent->LMin=((nm)Params.scenl).val(); 
		Params.regime=BCKG; 
		stages << (temp=CreateVoltSubScenario(Params)); 
		Params.regime=NON_BCKG; 
		stages << (temp=CreateVoltSubScenario(Params)); 
		stages << (temp=CreateMainSubScenario(Params)); 

		mOFF=new ComMotorOFFStage(CString("MotorOFFAfterMain")); stages << mOFF;
		Params.numPoints++; 
	}

	if((ret=stages.DispatchErrors())==NO_ERRORS)
	{
		MVThread* WorkThread=(MVThread*)p;
		WorkThread->Stages.Reset(); 
		if (WorkThread->Stages.Add(&stages)!=0)
		{			
			WorkThread->Stages.SetBeg(LStT(Params.curl,0,us10())); 
			WorkThread->Stages.HostThreadConfig.PersonalID=MainTaskID; 
			WorkThread->Stages.HostThreadConfig.ParentWindow=Params.UpdateWnd;;
			WorkThread->Stages.RegisterStages();
		}		
		Parent->ScanStep=((nm)Params.scanStep).val(); Parent->LMax=((nm)Params.scenl).val();
		Parent->UpdateData(0); 
	}	
	stages.DetachErrors();
	Params.DetachErrors();
	return ret;
}

SDRstage* MainStagesCtrl::CreateMotorStage(CString name)
{
	SDRstage* ret; int Status; MainCfg.Get(CFG_SENSOR_STATUS,Status);

	switch(Status)
	{
	case CFG_ENABLE: ret=new SDRSstage(name); break;
	case CFG_DISABLE: ret=new SDRstage(name); break;
	default: ret=0;
	}
	if(ret!=NULL) ret->HostThreadConfig.NotifyUser=true;
	return ret;
}

int  MainStagesCtrl::CreateSeries()
{
	int ret=0; 
	return ret;
}


scenario* MainStagesCtrl::CreatePreWCHSubScenario( ScenarioParams &Params )
{
	scenario *ret=NULL; SDRstage *MS=0; 
	if(Params.curl!=Params.mp.l1)
	{
		ret=new scenario();
		MS=CreateMotorStage(CString("PrepWCh")); 
		SDRstage::SetupParams t(Params.curl,Params.mp.l1);
		MS->HostThreadConfig.ParentWindow=Params.UpdateWnd;
		MS->Setup(&t); 
		(*ret) << MS;
		Params.scenl+=MS->End().l;		
	}
	return ret;
}

scenario* MainStagesCtrl::CreateMainSubScenario( ScenarioParams &Params )
{
	scenario *ret=new scenario(), *r1; ret->AttachErrorsOF(&Params);
	SDRstage *MS=0; fm10 templ2; 

	for(int i=0; i<Params.numPoints; i++)
	{
		templ2=Params.scenl+Params.scanStep*Params.sign;
		MS=CreateMotorStage(CString("NormWCh")); 
		MS->HostThreadConfig.ParentWindow=Params.UpdateWnd;
		r1=CreateVoltSubScenario(Params);
		(*ret) << MS << r1; 
		SDRstage::SetupParams t(Params.scenl,templ2);
		MS->Setup(&t); 
		Params.scenl=templ2;		
	}	
	ret->DetachErrors();
	return ret;
}


scenario* MainStagesCtrl::CreateFiltersSubScenario(ScenarioParams &Params)
{
	scenario* ret=new scenario; ret->AttachErrorsOF(&Params);
	ScenarioStage *mOFF; PauseStage *PS=0; 
	CString T; int tid=MSG_STRING0; ms t; FilterHeadPos fp;

	switch(Params.regime)
	{
	case BCKG:  fp=FilterHeadPos(SOLID); break;
	case NON_BCKG: fp=MainCfg.FilterHead.FindFilter(Params.scenl,Params.filter_pos); break;
	}

	if(fp!=Params.filter_pos)
	{
		if(Params.automatic) 
		{
			SDAstage::SetupParams pp(fp,Params.filter_pos);
			SDAstage *fs=new SDAstage(CString("Filter change")); (*ret) << fs; 
			fs->HostThreadConfig.NotifyUser=true;		
			fs->Setup(&pp); fs->HostThreadConfig.ParentWindow=Params.UpdateWnd;
			t=ms(1000);	
			switch(Params.regime)
			{
			case BCKG:  tid=MSG_STRING1; break;
			case NON_BCKG: tid=MSG_STRING2; break;
			}
		}
		else
		{			
			mOFF=new ComMotorOFFStage(CString("MotorOFFAfterPrepare")); (*ret) << mOFF;
			t=ms(INFNT);
			switch(Params.regime)
			{
			case BCKG:  tid=MSG_STRING3; break;
			case NON_BCKG: tid=MSG_STRING4; break;
			}
		}	
		Params.filter_pos=fp;

		if(t!=ms(0)) 
		{
			PS=new PauseStage(CString("Pause after filter change"),t); PS->HostThreadConfig.NotifyUser=true;
			PS->HostThreadConfig.ParentWindow=Params.UpdateWnd;
			T.LoadString(AfxGetInstanceHandle(),tid);
			PS->HostThreadConfig.ActionDescription=PS->HostThreadConfig.PauseComment=T; (*ret) << PS;
		}	
	}

	
	ret->DetachErrors();
	return ret;
}
scenario* MainStagesCtrl::CreateVoltSubScenario( ScenarioParams &Params )
{
	scenario *ret=new scenario(), *r1; VoltmeterStage *VS=0; CString Name;
	ret->AttachErrorsOF(&Params);
	r1=CreateFiltersSubScenario(Params);	
	switch(Params.regime)
	{
	case BCKG:  Name=CString("BCKG"); VS=new ComVoltStage_BCKG(Name); break;
	case NON_BCKG: Name=CString("NormM"); VS=new ComVoltBckgStage(Name); break;
	}
	VS->Series=Params.Series; VS->HostThreadConfig.NotifyUser=true;
	VS->HostThreadConfig.ParentWindow=Params.UpdateWnd;
	(*ret) << r1 << VS;
	VS->Setup(&Params.vp);
	ret->DetachErrors();
	return ret;
}


LRESULT MainStagesCtrl::OnAccessGranted( WPARAM wParam,LPARAM lParam)
{	
	MVThread* WorkThread=(MVThread*)wParam;
	switch (GetLastState())
	{
	case Start:
		if(Setup(WorkThread)==0)
		{
			WarningDialog WD_Dialog; WD_Dialog.H=WorkThread;
			if(WD_Dialog.DoModal()==IDOK)
			{						
				if(CreateSeries()==0) 
				{
					WorkThread->StartThread(0,0);
					MyButton::OnAccessGranted(wParam,lParam);
					return 0;
				}
			}
		}
		EnableWindow(TRUE);
		break;
	case Pause: MyButton::OnAccessGranted(wParam,lParam); break;
	case Resume: EnableWindow(TRUE);SetState(Pause); break;
	}
	return 0;	
}

LRESULT MainStagesCtrl::OnStop( WPARAM wParam, LPARAM lParam )
{
	Parent->ProgressBar.Text="0 ms"; Parent->ProgressBar.SetPos(0);	
	SetState(Start);
	return 0;
}

RequestForAcsessMsg* MainStagesCtrl::GetRequestInfo()
{
	RequestForAcsessMsg* t=new RequestForAcsessMsg(this); 
	switch(GetLastState())
	{
	case Start: t->data.msg=UM_STOP; t->data.WorkThreadID=DONT_CARE; break;
	case Pause: t->data.msg=UM_PAUSE; t->data.WorkThreadID=MainTaskID; break;
	case Resume: t->data.msg=UM_CONTINUE; t->data.WorkThreadID=MainTaskID; break;
	}	
	return t;
}

LRESULT MainStagesCtrl::OnPause( WPARAM wParam, LPARAM lParam )
{
	SetState(Resume);
	return 0;
}

RequestForAcsessMsg* StopStageCtrl::GetRequestInfo() {return new RequestForAcsessMsg(this,UM_STOP,DONT_CARE);}

BEGIN_MESSAGE_MAP(StopStageCtrl, MyButton)
END_MESSAGE_MAP()

*/
void DialogBarTab1::OnBnClicked_Fit()
{		
	/*
	CString str; UpdateData(); TPointVsErrorSeries *graph; 
		size_t i; 
		
		PointVsErrorArray buf;		
		LogMessage *log=new LogMessage(); 
		void *x; TChart *chart=(TChart *)Parent;

		if((x=chart->Series.GainAcsess(READ))!=NULL)
		{
			SeriesProtector guard(x); TSeriesArray& series(guard);
			if(	(graph=GetSeries(series))!=NULL) 
			{
				graph->GetValues(buf);
				int N=buf.GetSize(), n0=GetArrayIndex(buf.x,X0), n1=n0-dX, n2=n0+dX;
				if(n1<0 || n2>=N)
				{
					str.Format("No valid points found %d+/-%d",X0,dX); 
					log->CreateEntry(CString('*'),str,LogMessage::high_pr);	
					log->Dispatch(); 
					return;
				}				
				buf.RemoveAll(); graph->GetValues(buf,n1,n2);					
			}			
			else
			{
				str.Format("No series matching criteria (ACTIVE) found"); 
				log->CreateEntry(CString('*'),str,LogMessage::high_pr);	
				log->Dispatch(); return;
			}
		}
		else return;

		Fit_Ax2BxCParams init(1e-2,1e-1,1), out;
		MultiFitterFuncParams params(buf.x,buf.y,buf.dy);
		Fit_Parabola(params, init,  out);
		
		str.Format("**********************************"); log->CreateEntry(CString('*'),str);
		str.Format("status = %s", gsl_strerror (out.status)); log->CreateEntry(CString('*'),str);
		if (out.status!=GSL_SUCCESS) log->CreateEntry(CString('*'),CString(' '),LogMessage::high_pr);
		str.Format("----------------------------------"); log->CreateEntry(CString('*'),str);
		str.Format("chisq/dof = %g", out.chisq_dof); log->CreateEntry(CString('*'),str);
		for(i=0;i<out.GetParamsNum();i++)
		{
			str.Format("x%d = %g +/- %g%%", i,out.a[i],100*out.da[i]/out.a[i]); log->CreateEntry(CString('*'),str);
		}
		str.Format("xmin = %g ymin = %g", out.GetTop().x,out.GetTop().y);
		log->CreateEntry(CString('*'),str,LogMessage::high_pr);
		str.Format("time = %g ms", out.dt.val()); log->CreateEntry(CString('*'),str);
		str.Format("func_cals = %d", out.func_call_cntr); log->CreateEntry(CString('*'),str);
		str.Format("iter_num = %d", out.iter_num); log->CreateEntry(CString('*'),str);

		if(out.status==GSL_SUCCESS)
		{
			TSimplePointSeries* t1=NULL; 
			if((x=chart->Series.GainAcsess(WRITE))!=NULL)
			{
				SeriesProtector guard(x); TSeriesArray& series(guard); str.Format("PolyFit%d",PolinomOrder);
				if((t1=new TSimplePointSeries(str))!=0)	
				{
					series.Add(t1); 
					t1->_SymbolStyle::Set(NO_SYMBOL);
					t1->AssignColors(ColorsStyle(clRED,series.GetRandomColor()));
					t1->SetVisible(true); 

					t1->ParentUpdate(UPD_OFF);
					double x_min,x_max,dx,t;
					t=x_min=out.leftmostX; x_max=out.rightmostX; dx=fabs(x_max-x_min)/out.n;
					for(i=0;i<out.n;i++) 
					{
						t1->AddXY(out.GetXabsY(t));
						t+=dx;
					}
					t1->ParentUpdate(UPD_ON);
				}	
			}
			chart->PostMessage(UM_CHART_SHOWALL);		
		}
		log->Dispatch();
		*/
}

//////////////////////////////////////////////////////////////////////////
struct FourierSmoothParams
{	
	ms dt;
	int status; 
};

FourierSmoothParams FourierSmoothFunc(SimplePointArray &data, int spec_wdth)
{
	FourierSmoothParams ret;
/*
	FFTRealTransform::Params in(data.y), out;
	ret.status=FourierFilter(in,spec_wdth,out);
	ret.dt=out.dt;
*/  
  return ret;
}


void ShowFourierSmooth(ProtectedSeriesArray &Series, SimplePointArray &data)
{
	void *x=NULL; TSimplePointSeries *t1=NULL; SimplePoint val; val.type.Set(GenericPnt);

	if((x=Series.GainAcsess(WRITE))!=NULL)
	{
		SeriesProtector guard(x); TSeriesArray& series(guard);

		if((t1=new TSimplePointSeries(CString("Spectrum")))!=0)	
		{
			series.Add(t1); 
			t1->AssignColors(ColorsStyle(clRED,clRED));
			t1->_SymbolStyle::Set(NO_SYMBOL);

			t1->ParentUpdate(UPD_OFF);
			for(int i=0;i<data.GetSize();i++) t1->AddXY(data[i]);
			t1->ParentUpdate(UPD_ON);
		}	

	}
}

//////////////////////////////////////////////////////////////////////////
struct LocateMinimumsParams
{	
	ms dt;
	int minimumN;
	int status; 
};
LocateMinimumsParams LocateMinimumsFunc(SimplePointArray &data)
{
	LocateMinimumsParams ret; double drv_l,drv_r; int i,j; MyTimer timer1;
    
	timer1.Start();

	double *y=data.GetY(), *x=data.GetX(); 
	drv_l=y[1]-y[0];
	for(i=1,j=0;i<data.GetSize()-1;i++)
	{		
		drv_r=y[i]-y[i+1]; 
		if( drv_l<0 && drv_r<0)
		{
			x[j]=x[i]; y[j++]=y[i]; 
		}
		drv_l=-drv_r;
	}
	data.SetSize(j); 

	ret.minimumN=data.GetSize();
	ret.dt=timer1.StopStart();
	ret.status=S_OK;
    return ret;
}
//////////////////////////////////////////////////////////////////////////
void ShowMinimums(ProtectedSeriesArray &Series, SimplePointArray &data)
{
	void *x=NULL; TSimplePointSeries *t1=NULL; SimplePoint val; val.type.Set(GenericPnt);
	CString str; int i;

	if((x=Series.GainAcsess(WRITE))!=NULL)
	{
		SeriesProtector guard(x); TSeriesArray& series(guard);

		if((t1=new TSimplePointSeries(CString("Minimums")))!=0)	
		{
			series.Add(t1); 
			t1->AssignColors(ColorsStyle(clRED,series.GetRandomColor()));
			t1->_LineStyle::Set(NO_LINE);

			t1->ParentUpdate(UPD_OFF);
			for(i=0;i<data.GetSize();i++) t1->AddXY(data[i]);
			t1->ParentUpdate(UPD_ON);
		}
	}
}
//////////////////////////////////////////////////////////////////////////
/*
struct MinimumsFitFilterParams
{	
	ms dt;
	int status; 
	CArray<Fit_Ax2BxCParams> fitings;
};
*/
#define INDEX_OUT_OF_RANGE -1;

int GetArrayIndex(DoubleArray& arr, double x )
{
	int ret=INDEX_OUT_OF_RANGE;
	double l,r,m; int N=arr.GetSize(),l_n=0,r_n=N-1,m_n;
	if(N==0) return INDEX_OUT_OF_RANGE;
	if(N==1) return 0;
	l=arr[l_n]-x; r=arr[r_n]-x;
	if( (l<0 && r<0) || (l>0 && r>0) ) return ( (fabs(l)<fabs(r)) ? l_n: r_n );
	
	m_n=l_n+(r_n-l_n)/2; m=arr[m_n];
	while(m_n!=l_n)
	{
		m=arr[m_n]-x;
		if( (m<0 && r<0) || (m>0 && r>0) ) { r_n=m_n; r=m; }
		else { l_n=m_n; l=m; }
		m_n=l_n+(r_n-l_n)/2; 
	}
	ret=( (fabs(l)<fabs(r)) ? l_n: r_n );
	return ret;
}
/*
MinimumsFitFilterParams* MinimumsFitFilterFunc(PointVsErrorArray &data,SimplePointArray &mins, int dn)
{
	MinimumsFitFilterParams* ret=new MinimumsFitFilterParams();
	MyTimer timer1; int i, dn_max=80, index; 
	PointVsErrorArray buft; Fit_Ax2BxCParams init, out; MultiFitterFuncParams* params=NULL;

	timer1.Start();
	SimplePoint data0=data[0]; 
	for(i=0;i<mins.GetSize();i++)
	{
		SimplePoint tt=mins[i];
		index=GetArrayIndex(data.x,mins[i].x);
		if( index<dn_max ) continue;

		buft.CopyFrom(data,2*dn_max+1,index-dn_max); 
		init=Fit_Ax2BxCParams(1e-2,1e-1,1);
		if(params!=NULL) { delete params; params=NULL; }
		params=new MultiFitterFuncParams(buft.x,buft.y,buft.dy);
		Fit_Parabola(*params, init,  out);
        if(out.status!=GSL_SUCCESS) continue;
		if(out.a[2]<0) continue;

		buft.CopyFrom(data,2*dn+1,index-dn); 
		init=Fit_Ax2BxCParams(1e-2,1e-1,1);
		if(params!=NULL) { delete params; params=NULL; }
		params=new MultiFitterFuncParams(buft.x,buft.y,buft.dy);
		Fit_Parabola(*params, init,  out);
		if(out.status!=GSL_SUCCESS) continue;
		if(out.a[2]<0) continue;

		SimplePoint top=out.GetTop();
		if( top.x<out.leftmostX || top.x>out.rightmostX ) continue;

		ret->fitings.Add(out);
	}
	if(params!=NULL) { delete params; params=NULL; }
	ret->dt=timer1.StopStart();
	ret->status=S_OK;	
	return ret;
}
*/
/*
void ShowFittings(ProtectedSeriesArray &Series, MinimumsFitFilterParams &data)
{

	void *x=NULL; TSimplePointSeries *t1=NULL, *t2; SimplePoint val; val.type.Set(GenericPnt);
	CString str; size_t k=0; int i=0;

	if((x=Series.GainAcsess(WRITE))!=NULL)
	{
		SeriesProtector guard(x); TSeriesArray& series(guard);

		str=L"FinalMins"; t2=new TSimplePointSeries(str); 
		series.Add(t2); t2->AssignColors(ColorsStyle(clWHITE,clWHITE));
		t2->_LineStyle::Set(NO_LINE); t2->_SymbolStyle::Set(VERT_LINE);
		t2->_SymbolStyle::dy=10;
		t2->SetVisible(true); 

		t2->ParentUpdate(UPD_OFF);
		for(i=0;i<data.fitings.GetSize() && t2!=NULL;i++)
		{
			Fit_Ax2BxCParams& out=data.fitings[i];
			t2->AddXY(out.GetTop());
		}			
		t2->ParentUpdate(UPD_ON);
	}
	
}
*/
TPointVsErrorSeries* DialogBarTab1::GetSeries(TSeriesArray& series)
{
	TPointVsErrorSeries* ret=NULL;	TChart *chart=(TChart *)Parent; int n;
	
	SeriesSearchPattern pattern; pattern.mode=SERIES_PID | SERIES_TYPE; 
	pattern.SeriesType=POINTvsERROR;
	if( (n=SeriesCombo.GetCurSel())!=CB_ERR)
	{
		pattern.PID=SeriesCombo.GetItemData(n);
		TSeriesArray results(DO_NOT_DELETE_SERIES);
		if(	series.FindSeries(pattern,results)!=0 ) 
		{
			ret=(TPointVsErrorSeries*)results.Series[0];
		}	
	}	
	return ret;
}
//////////////////////////////////////////////////////////////////////////
afx_msg void DialogBarTab1::OnBnClicked_Locate()
{
/*
	TChart *chart=(TChart *)Parent; void *x; SimplePoint val; TChartSeries *t1=NULL;
	int n=0; MyTimer timer1; ms dt1,dt2,dt3,dt4,dt5; UpdateData();
	TPointVsErrorSeries *graph; CString str;
	PointVsErrorArray BUF; SimplePointArray buf_smooth; 
	LogMessage *log=new LogMessage(); 
	
	timer1.Start();
	if((x=chart->Series.GainAcsess(READ))!=NULL)
	{
		SeriesProtector guard(x); TSeriesArray& series(guard);
		if(	(graph=GetSeries(series))!=NULL) 
		{
			graph->GetValues(BUF);
		}			
		else
		{
			str.Format("No series matching criteria (ACTIVE) found"); 
			log->CreateEntry(CString('*'),str,LogMessage::high_pr);	
			log->Dispatch(); return;
		}
	}
	else return;

	buf_smooth.CopyFrom(BUF);

	FourierSmoothParams FourierSmooth=FourierSmoothFunc(buf_smooth, spec_wdth);
//	ShowFourierSmooth(chart->Series,buf_smooth);
	LocateMinimumsParams LocateMinimums=LocateMinimumsFunc(buf_smooth);
	ShowMinimums(chart->Series,buf_smooth);
	MinimumsFitFilterParams *MinimumsFitFilter=MinimumsFitFilterFunc(BUF, buf_smooth, minimum_widht_2);
	ShowFittings(chart->Series, *MinimumsFitFilter);
	dt1=timer1.StopStart();

	str.Format("********Fourier smooth*************"); log->CreateEntry(CString('*'),str);
	str.Format("time=%g ms",FourierSmooth.dt.val()); log->CreateEntry(CString('*'),str);
	str.Format("********Minimums 1stage*************"); log->CreateEntry(CString('*'),str);
	str.Format("minimums=%d time=%g ms",
		LocateMinimums.minimumN, LocateMinimums.dt.val()); log->CreateEntry(CString('*'),str);
	str.Format("********Minimums 2stage*************"); log->CreateEntry(CString('*'),str);
	str.Format("minimums=%d time=%g ms", MinimumsFitFilter->fitings.GetSize(), MinimumsFitFilter->dt.val()); log->CreateEntry(CString('*'),str);
	str.Format("*********Total********************"); log->CreateEntry(CString('*'),str);
	str.Format("time=%g ms",dt1.val()); log->CreateEntry(CString('*'),str,LogMessage::high_pr);

	log->Dispatch();
	delete MinimumsFitFilter;
	*/
}

#define MAX_SYMBOLS 1000
#define MAX_VALUES 7
#define END_OF_FILE -1
#define BUF_OVERLOAD -2

int ReadString(FILE* file, unsigned char *buf, int buf_max )
{
	int n=0;
	while(n<buf_max)
	{
		if( fread(buf,1,1,file) ==0) { *buf=0; return END_OF_FILE; }		
		if(*buf==0x0a) { *buf=0; return n+1; }
		buf++; n++;
	}
	return BUF_OVERLOAD;
}

int AnalyzeString(CString &str, double *arr, int max_arr)
{
	int first=0,last=0,n=0; CString val_str;
	while( (last=str.Find(',',first)) >=0 && n<MAX_VALUES) 
	{
		val_str=str.Mid(first,last-first); arr[n++]=atof(val_str);
		first=last+1;
		
	}
	val_str=str.Mid(first,str.GetLength()-first); if(n<MAX_VALUES) arr[n++]=atof(val_str);
	return n;
}

void DialogBarTab1::OnBnClickedLoadCalibration()
{
	CString filter="Metricon output (*.txt)|*.txt||", logT, FullFileName, FileName; double vals[MAX_VALUES]; PointVsError pnte;
	void *x; TPointVsErrorSeries *t2; MyTimer Timer1; sec time; int n,strs=0;
	LogMessage *log=new LogMessage(); log->CreateEntry("Log","Metricon load Speed tests",LogMessage::low_pr);
	
	CFileDialog dlg1(true,NULL,NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter);
	if(dlg1.DoModal()==IDOK)
	{
		Timer1.Start(); FullFileName=dlg1.GetFileName();

		int pos=-1; n=-1;
		while( (n=FullFileName.Find("\\",n+1))!=-1 ) pos=n; 
		FileName=FullFileName.Right(FullFileName.GetLength()-(pos+1));
		while( (n=FullFileName.Find(".",n+1))!=-1 ) pos=n; 
		FileName=FileName.Left(pos);

		FILE *file; fopen_s(&file,FullFileName,"r");
		unsigned char buf[MAX_SYMBOLS]; 
		TPointVsErrorSeries::DataImportMsg *ChartMsg;
		CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); TChart& chrt=mf->Chart1; 
		if((x=chrt.Series.GainAcsess(WRITE))!=0)
		{
			SeriesProtector Protector(x); TSeriesArray& Series(Protector);
			if((t2=new TPointVsErrorSeries(FileName))!=0)	
			{
				Series.Add(t2); 
				t2->_SymbolStyle::Set(NO_SYMBOL); 
				ChartMsg=t2->CreateDataImportMsg(); 
				t2->AssignColors(ColorsStyle(clRED,Series.GetRandomColor()));
				t2->SetStatus(SER_ACTIVE); t2->SetVisible(true);
			}		
		}

		while( (n=ReadString(file,buf,MAX_SYMBOLS))!=END_OF_FILE && n!=BUF_OVERLOAD )
		{
			if(strs++<3) continue;
			CString str(buf);
			if( AnalyzeString(str, vals, MAX_VALUES)==MAX_VALUES )
			{
				pnte.x=vals[2];
				pnte.y=vals[5];
				pnte.dy=1e-3;
				if(pnte.x<0) break;
				ChartMsg->Points.Add(pnte);	
			}			
		}		
		int N=ChartMsg->Points.GetSize(); double tt;
		double* X=ChartMsg->Points.GetX(),*lX,*rX; lX=X; rX=lX+N-1;
		double* Y=ChartMsg->Points.GetY(),*lY,*rY; lY=Y; rY=lY+N-1;
		double* DY=ChartMsg->Points.GetdY(),*lDY,*rDY; lDY=DY; rDY=lDY+N-1;
		for(int i=0;i<N/2;i++)
		{
			tt=*lX; *lX=*rX; *rX=tt; lX++; rX--;
			tt=*lY; *lY=*rY; *rY=tt; lY++; rY--;
			tt=*lDY; *lDY=*rDY; *rDY=tt; lDY++; rDY--;
		}
		Timer1.Stop(); time=Timer1.GetValue();
		logT.Format("Loaded %d points time=%s",ChartMsg->Points.GetSize(),ConvTimeToStr(time)); 
		log->CreateEntry(CString('*'),logT);
		ChartMsg->Dispatch();		 
		log->Dispatch();
	}
}

LRESULT DialogBarTab1::OnSeriesUpdate(WPARAM wParam, LPARAM lParam )
{
	CString T; int n,pid;
	if(lParam!=0) 
	{
		Series=(ProtectedSeriesArray*)lParam;
		SeriesCombo.ResetContent();
		void *x; 
		if((x=Series->GainAcsess(READ))!=0)
		{
			SeriesProtector Protector(x); TSeriesArray& Series(Protector);			

			SeriesSearchPattern pattern; 
			pattern.mode=SERIES_TYPE; pattern.SeriesType=POINTvsERROR;
			TSeriesArray results(DO_NOT_DELETE_SERIES); 
			if( Series.FindSeries(pattern,results)!=0 )
			{				
				for(int i=0;i<results.Series.GetSize();i++)
				{
					T.Format("%s (%d)",results.Series[i]->Name,results.Series[i]->GetSize());
					n=SeriesCombo.AddString(T); pid=results.Series[i]->PID;
					SeriesCombo.SetItemData(n,pid);
				}				
			}
		}
	}
	return 0;
}

void DialogBarTab1::OnBnClickedButton4()
{
/*
	CString str; UpdateData(); TPointVsErrorSeries *graph; 
	LogMessage *log=new LogMessage(); size_t i;

	PointVsErrorArray buf;		
	void *x; TChart *chart=(TChart *)Parent;

	if((x=chart->Series.GainAcsess(READ))!=NULL)
	{
		SeriesProtector guard(x); TSeriesArray& series(guard);
		if(	(graph=GetSeries(series))!=NULL) 
		{
			graph->GetValues(buf);
			int N=buf.GetSize(), n1=GetArrayIndex(buf.x,Xmin), n2=GetArrayIndex(buf.x,Xmax);
			if(n1<0 || n2>=N)
			{
				str.Format("No valid points found %d+/-%d",X0,dX); 
				log->CreateEntry(CString('*'),str,LogMessage::high_pr);	
				log->Dispatch(); 
				return;
			}				
			buf.RemoveAll(); graph->GetValues(buf,n1,n2);					
		}			
		else
		{
			str.Format("No series matching criteria (ACTIVE) found"); 
			log->CreateEntry(CString('*'),str,LogMessage::high_pr);	
			log->Dispatch(); return;
		}
	}
	else return;

	Fit_KneeParams init(1,0.1,0.1,.1), out;
	MultiFitterFuncParams params(buf.x,buf.y,buf.dy);
	Fit_StepFunc(params, init,  out);

	str.Format("**********************************"); log->CreateEntry(CString('*'),str);
	str.Format("status = %s", gsl_strerror (out.status)); log->CreateEntry(CString('*'),str);
	if (out.status!=GSL_SUCCESS) log->CreateEntry(CString('*'),CString(' '),LogMessage::high_pr);
	str.Format("----------------------------------"); log->CreateEntry(CString('*'),str);
	str.Format("chisq/dof = %g", out.chisq_dof); log->CreateEntry(CString('*'),str);
	for(i=0;i<out.GetParamsNum();i++)
	{
		str.Format("x%d = %g +/- %g%%", i,out.a[i],100*out.da[i]/out.a[i]); log->CreateEntry(CString('*'),str);
	}
	SimplePoint Top=out.GetTop(level);
	str.Format("xmin = %g ymin = %g", Top.x,Top.y); log->CreateEntry(CString('*'),str,LogMessage::high_pr);
	str.Format("time = %g ms", out.dt.val()); log->CreateEntry(CString('*'),str);
	str.Format("func_cals = %d", out.func_call_cntr); log->CreateEntry(CString('*'),str);
	str.Format("iter_num = %d", out.iter_num); log->CreateEntry(CString('*'),str);

	TSimplePointSeries* t1=NULL; 
	if((x=chart->Series.GainAcsess(WRITE))!=NULL)
	{
		SeriesProtector guard(x); TSeriesArray& series(guard); str.Format("PolyFit%d",PolinomOrder);
		if((t1=new TSimplePointSeries(str))!=0)	
		{
			series.Add(t1); 
			t1->_SymbolStyle::Set(NO_SYMBOL);
			t1->AssignColors(ColorsStyle(clRED,series.GetRandomColor()));
			t1->SetVisible(true); 

			t1->ParentUpdate(UPD_OFF);
			double x_min,x_max,dx,t;
			t=x_min=out.leftmostX; x_max=out.rightmostX; dx=fabs(x_max-x_min)/out.n;
			for(i=0;i<out.n;i++) 
			{
				t1->AddXY(out.GetXabsY(t));
				t+=dx;
			}
			t1->ParentUpdate(UPD_ON);
		}

		if( (t1=new TSimplePointSeries("FinalMins"))!=0)	
		{
			series.Add(t1); t1->AssignColors(ColorsStyle(clWHITE,clWHITE));
			t1->_LineStyle::Set(NO_LINE); t1->_SymbolStyle::Set(VERT_LINE);
			t1->_SymbolStyle::dy=10;
			t1->SetVisible(true); 
			t1->AddXY(Top);
		}
	}
	chart->PostMessage(UM_CHART_SHOWALL);	
	log->Dispatch();
*/
}
