#include "stdafx.h"
#include "calibratorDialog.h"
#include "ImageWnd.h"

// CalibratorDialog dialog

IMPLEMENT_DYNAMIC(CalibratorDialog, CDialog)

CalibratorDialog::CalibratorDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CalibratorDialog::IDD, pParent)
{
	for(int i=0;i<modes_num;i++) { N[i]=0; Q[i]=0; }
	Nc=Lc=dc=fic=0;
	Series=NULL; 
//	Q[0]=63.37; Q[1]=60.02; Q[2]=55.13; Q[3]=48.98;
}

CalibratorDialog::~CalibratorDialog()
{
}

void CalibratorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, N[0]);
	DDX_Text(pDX, IDC_EDIT10, N[1]);
	DDX_Text(pDX, IDC_EDIT2, N[2]);
	DDX_Text(pDX, IDC_EDIT6, N[3]);
	DDX_Text(pDX, IDC_EDIT5, Q[0]);
	DDX_Text(pDX, IDC_EDIT13, Q[1]);
	DDX_Text(pDX, IDC_EDIT8, Q[2]);
	DDX_Text(pDX, IDC_EDIT16, Q[3]);
	DDX_Text(pDX, IDC_EDIT14, Nc);
	DDX_Text(pDX, IDC_EDIT17, Lc);
	DDX_Text(pDX, IDC_EDIT18, dc);
	DDX_Text(pDX, IDC_EDIT19, fic);
	DDX_Control(pDX, IDC_COMBO1, SeriesCombo);
	DDX_Control(pDX, IDC_COMBO3, SeriesCombo1);
}


BEGIN_MESSAGE_MAP(CalibratorDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON8, &CalibratorDialog::OnBnClickedButton8)
	ON_MESSAGE(UM_SERIES_UPDATE,OnSeriesUpdate)	
	ON_CBN_SELCHANGE(IDC_COMBO1, &CalibratorDialog::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BUTTON4, &CalibratorDialog::OnBnClickedCalculateCal)
	ON_BN_CLICKED(IDC_BUTTON10, &CalibratorDialog::OnBnClickedSaveToConfig)
	ON_CBN_SELCHANGE(IDC_COMBO3, &CalibratorDialog::OnCbnSelchangeCombo3)
END_MESSAGE_MAP()


// CalibratorDialog message handlers

void CalibratorDialog::OnBnClickedButton8()
{
	OnOK();
}

LRESULT CalibratorDialog::OnSeriesUpdate(WPARAM wParam, LPARAM lParam )
{
	CString T; int n;
	if(lParam!=0) 
	{
		Series=(ProtectedSeriesArray*)lParam;
		SeriesCombo.ResetContent(); SeriesCombo1.ResetContent();
		void *x; 
		if((x=Series->GainAcsess(READ))!=0)
		{
			SeriesProtector Protector(x); TSeriesArray& Series(Protector);			

			SeriesSearchPattern pattern; 
			pattern.mode=SERIES_NAME | SERIES_TYPE; 
			pattern.name=CString("FinalMins"); pattern.SeriesType=SIMPLE_POINT;
			TSeriesArray results(DO_NOT_DELETE_SERIES); 
			if( Series.FindSeries(pattern,results)!=0 )
			{				
				for(int i=0;i<results.Series.GetSize();i++)
				{
					T.Format("%s (%d)",results.Series[i]->Name,results.Series[i]->GetSize());
					n=SeriesCombo.AddString(T); SeriesCombo.SetItemData(n,(DWORD)results.Series[i]);
					n=SeriesCombo1.AddString(T); SeriesCombo1.SetItemData(n,(DWORD)results.Series[i]);
				}				
			}
		}
	}
	return 0;
}
void CalibratorDialog::OnCbnSelchangeCombo1()
{
	int n, i; double t;
	if( (n=SeriesCombo.GetCurSel())!=CB_ERR)
	{
		TSimplePointSeries *seriea=(TSimplePointSeries *)SeriesCombo.GetItemData(n);
		for(i=0;i<seriea->GetSize() && i<4;i++)
		{
			t=(*seriea)[i].x;
			t*=100; t=(int)t; t/=100;
			N[modes_num-i-1]=t;			
		}
		for(;i<4;i++) N[modes_num-i-1]=0;
		UpdateData(0);
	}
}

void CalibratorDialog::OnCbnSelchangeCombo3()
{
	int n, i; double t;
	if( (n=SeriesCombo1.GetCurSel())!=CB_ERR)
	{
		TSimplePointSeries *seriea=(TSimplePointSeries *)SeriesCombo1.GetItemData(n);
		for(i=0;i<seriea->GetSize() && i<4;i++)
		{
			t=(*seriea)[i].x;
			t*=100; t=(int)t; t/=100;
			Q[modes_num-i-1]=t;			
		}
		for(;i<4;i++) Q[modes_num-i-1]=0;
		UpdateData(0);
	}
}


void CalibratorDialog::OnBnClickedCalculateCal()
{
	MyTimer Timer1; ms dt1,dt2; double t;
	DoubleArray Nexp,teta; CString T;
	cal.n_p=2.14044; cal.n_s=1.; 
	UpdateData();
	for(int i=0;i<modes_num;i++)
	{
		t=N[i]; Nexp.Add(N[i]); 
		t=Q[i]*DEGREE; teta.Add(t);
	}

	TChart *chart=(TChart*)GetChartFromParent(); 
	ASSERT(chart != NULL);
	if (chart == NULL) return;
	
	void *xxx; 	CString str; TSimplePointSeries* t1=NULL; SimplePoint pnt;
	
	if((xxx=chart->Series.GainAcsess(WRITE))!=NULL)
	{
		SeriesProtector guard(xxx); TSeriesArray& series(guard); str.Format("PolyFit%d",1234);
		if((t1=new TSimplePointSeries(str))!=0)	
		{
			series.Add(t1); 
			t1->_SymbolStyle::Set(NO_SYMBOL);
			t1->AssignColors(ColorsStyle(clRED,series.GetRandomColor()));
			t1->SetVisible(true); 

			CalibrationFuncParams in_params(Nexp.GetSize(), Nexp, teta, cal.n_p, cal.n_s, cal.alfa );

			t1->ParentUpdate(UPD_OFF);
			for(double x=0; x<45.0; x+=0.1)
			{
				pnt.x=x;
				pnt.y=CalibrationSolver::func(x*DEGREE,&in_params);
				t1->AddXY(pnt);
			}
			t1->ParentUpdate(UPD_ON);
		}	
	}
	chart->PostMessage(UM_CHART_SHOWALL);	
	
	LogMessage *log=new LogMessage(); log->CreateEntry("Log","Speed tests results",LogMessage::low_pr);

	CreateCalibration(Nexp, teta, cal);

	Nc=cal.N0; Lc=cal.L; dc=cal.d0; fic=cal.fi0/DEGREE;
	UpdateData(0);

	T.Format("****Statistics***"); 
	if(cal.status!=GSL_SUCCESS) log->CreateEntry("*",T,LogMessage::high_pr);
	T.Format("---Calibration---"); log->CreateEntry("*",T);
	T.Format("Nexp=[%g %g %g %g]",cal.Nexp[0],cal.Nexp[1],cal.Nexp[2],cal.Nexp[3]);log->CreateEntry("*",T);
	T.Format("teta=[%g %g %g %g]",cal.teta[0],cal.teta[1],cal.teta[2],cal.teta[3]);log->CreateEntry("*",T);
	T.Format("N0=%.10f L=%.10f d0=%.10f fi0=%.10f errabs=%g errrel=%g",cal.N0,cal.L,cal.d0,cal.fi0,cal.epsabs,cal.epsrel); log->CreateEntry("*",T);
	T.Format("dt=%.3f ms func_calls=%d",cal.dt.val(), cal.func_call_cntr); log->CreateEntry("*",T);
	log->Dispatch();		
}

void CalibratorDialog::OnBnClickedSaveToConfig()
{
	MainCfg.SetCalibration(cal);	
	CWnd* mainfrm=AfxGetMainWnd();
	mainfrm->PostMessage(UM_UPDATE_CONFIG,0,0);
}

void * CalibratorDialog::GetChartFromParent()
{
	CWnd *t = GetParent(); 
	return ((ImageWndCtrlsTab*)GetParent())->GetChartFromParent();
}

