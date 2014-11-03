// CalcTEDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CalcTEDialog.h"


// CalcTEDialog dialog

IMPLEMENT_DYNAMIC(CalcTEDialog, CDialog)

CalcTEDialog::CalcTEDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CalcTEDialog::IDD, pParent)
	, nf(0)
	, hf(0)
{
	for(int i=0;i<modes_num;i++) { N[i]=0; Q[i]=0; beta[i]=0; }
	Series=NULL; IsTM=FALSE; lambda = 632.8; n3 = 1.45705;
//	N[0]=3128.82; N[1]=2714.61; N[2]=2149; N[3]=1426.4;
}

CalcTEDialog::~CalcTEDialog()
{
}

void CalcTEDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, SeriesCombo);
	DDX_Text(pDX, IDC_EDIT1, N[0]);
	DDX_Text(pDX, IDC_EDIT10, N[1]);
	DDX_Text(pDX, IDC_EDIT2, N[2]);
	DDX_Text(pDX, IDC_EDIT6, N[3]);
	DDX_Text(pDX, IDC_EDIT5, Q[0]);
	DDX_Text(pDX, IDC_EDIT13, Q[1]);
	DDX_Text(pDX, IDC_EDIT8, Q[2]);
	DDX_Text(pDX, IDC_EDIT16, Q[3]);
	DDX_Text(pDX, IDC_EDIT14, nf);
	DDX_Text(pDX, IDC_EDIT17, hf);
	DDX_Text(pDX, IDC_EDIT18, lambda);
	DDX_Text(pDX, IDC_EDIT19, n3);
	DDX_Text(pDX, IDC_EDIT21, n_p);
}


BEGIN_MESSAGE_MAP(CalcTEDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON4, &CalcTEDialog::OnBnClickedConvertToAngles)
	ON_MESSAGE(UM_SERIES_UPDATE,OnSeriesUpdate)	
	ON_CBN_SELCHANGE(IDC_COMBO1, &CalcTEDialog::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BUTTON11, &CalcTEDialog::OnBnClickedCalculate)
END_MESSAGE_MAP()


// CalcTEDialog message handlers

void CalcTEDialog::OnBnClickedConvertToAngles()
{
	CalibrationParams cal; UpdateData();
	MainCfg.GetCalibration(&cal); cal.n_p = n_p;
	for(int i=0;i<modes_num;i++)
	{
		CalibratorParams calb_params(N[i]);
		Calibrator(calb_params,cal);	
		Q[i]=calb_params.teta;
		beta[i]=calb_params.betta;
	}
	UpdateData(0);
}

LRESULT CalcTEDialog::OnSeriesUpdate(WPARAM wParam, LPARAM lParam )
{
	CString T; int n;
	if(lParam!=0) 
	{
		Series=(ProtectedSeriesArray*)lParam;
		SeriesCombo.ResetContent();
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
					n=SeriesCombo.AddString(T);
					SeriesCombo.SetItemData(n,(DWORD)results.Series[i]);
				}				
			}
		}
	}
	return 0;
}

void CalcTEDialog::OnCbnSelchangeCombo1()
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

void CalcTEDialog::OnBnClickedCalculate()
{
	CString T; LogMessage *log=new LogMessage(); FilmParams *outTX=NULL; FilmFuncParams *in_TX=NULL;
	double k, n1; DoubleArray bettaexp_TX; 
	UpdateData();
	k = 2.*M_PI/lambda;	n1 = 1.; 

	for(int i=0;i<modes_num;i++) bettaexp_TX << beta[i];	
	outTX=new FilmParams();

	if(IsTM)
	{
		in_TX=new FilmFuncTMParams(bettaexp_TX, n1,n3,k);
		CalclFilmParamsTM(*((FilmFuncTMParams*)in_TX),*outTX);
		T.Format("--FilmParamsTM---"); log->CreateEntry("*",T);
	}
	else
	{
		in_TX=new FilmFuncTEParams(bettaexp_TX, n1,n3,k);
		CalclFilmParamsTE(*((FilmFuncTEParams*)in_TX),*outTX);
		T.Format("--FilmParamsTE---"); log->CreateEntry("*",T);
	}

	T.Format("status = %s", gsl_strerror (outTX->status)); 
	if(outTX->status==GSL_SUCCESS)
	{
		nf=outTX->n;
		hf=outTX->H;
		UpdateData(0);
		log->CreateEntry(CString('*'),T);
	}
	else log->CreateEntry(CString('*'),T,LogMessage::high_pr);
	
	T.Format("n=%.10f H=%.10f nm",outTX->n, outTX->H, outTX->epsabs, outTX->epsrel ); log->CreateEntry("*",T);
	T.Format("errabs=%g errrel=%g fval=%.10f, step=%.10f",outTX->epsabs, outTX->epsrel, outTX->fval, outTX->size ); log->CreateEntry("*",T);
	T.Format("dt=%.3f ms func_calls=%d",outTX->dt.val(), outTX->func_call_cntr); log->CreateEntry("*",T);
	
	for(int i=0;i<in_TX->betta_teor.GetSize();i++)
	{
		T.Format("betta_teor[%d]=%.5f betta_exp=%.5f",in_TX->betta_teor[i].n,in_TX->betta_teor[i].val,bettaexp_TX[i]); log->CreateEntry("*",T);
	}	
	
	log->Dispatch();
	if(outTX!=NULL) delete outTX; 
	if(in_TX!=NULL) delete in_TX;
}
