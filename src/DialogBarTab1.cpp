#include "stdafx.h"
#include "DialogBarTab1.h"
#include "MainFrm.h"
#include "mytime.h"
#include "metricon.h"

IMPLEMENT_DYNAMIC(MainChartWnd, TChart)
MainChartWnd::MainChartWnd(): TChart("Chart1")
{
}
/////////////////////////////////////////////////////////////////////////////
// DialogBarTab1 dialog
BEGIN_MESSAGE_MAP(MainChartWnd, TChart)
	//{{AFX_MSG_MAP(DialogBarTab1)
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

void MainChartWnd::Serialize(CArchive& ar)
{		
	Panel.Serialize(ar);
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
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_MESSAGE(UM_SERIES_UPDATE,OnSeriesUpdate)	
	//}}AFX_MSG_MAP	
	ON_BN_CLICKED(IDC_BUTTON2, OnBnClicked_Fit)
	ON_BN_CLICKED(IDC_BUTTON5, OnBnClicked_Locate)
	ON_BN_CLICKED(IDC_BUTTON1, &DialogBarTab1::OnBnClickedLoadCalibration)
	ON_BN_CLICKED(IDC_BUTTON4, &DialogBarTab1::OnBnClickedKneeTest)

	ON_BN_CLICKED(IDC_BUTTON9, &DialogBarTab1::OnBnClickedCalibrate)
	ON_BN_CLICKED(IDC_BUTTON11, &DialogBarTab1::OnBnClickedCalcTE)
	ON_BN_CLICKED(IDC_BUTTON16, &DialogBarTab1::OnBnClickedCalcTM)


END_MESSAGE_MAP()

DialogBarTab1::DialogBarTab1(CWnd* pParent /*=NULL*/)
	: BarTemplate(pParent)
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
	//}}AFX_DATA_INIT
	Name="MainControlTab";
//	Series=0;
	CheckButtonsStatus=0;
}


void DialogBarTab1::DoDataExchange(CDataExchange* pDX)
{
	BarTemplate::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DialogBarTab1)
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

	CalibratorDlg.Create(IDD_DIALOG_CAL,this);
	CalibratorDlg.SetWindowPos(NULL,300,300,0,0,SWP_NOSIZE | SWP_NOZORDER);

	CalcTEDlg.Create(IDD_DIALOG_CALCTE,this);
	CalcTEDlg.SetWindowPos(NULL,600,300,0,0,SWP_NOSIZE | SWP_NOZORDER);

	CalcTMDlg.Create(IDD_DIALOG_CALCTE,this);
	CalcTMDlg.SetWindowPos(NULL,300,300,0,0,SWP_NOSIZE | SWP_NOZORDER);
	CalcTMDlg.SetWindowText("TM Calculator"); 


	return 0;
}

BOOL DialogBarTab1::OnInitDialog() 
{
	CDialog::OnInitDialog();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void DialogBarTab1::OnKillfocus() {UpdateData();}

void DialogBarTab1::Serialize(CArchive& ar) 
{
	if (ar.IsStoring())
	{	// storing code
	}
	else
	{	// loading code
		UpdateData(0);
	}
}

void DialogBarTab1::OnBnClickedCalcTE()
{
	CalcTEDlg.ShowWindow(SW_SHOW);
}

void DialogBarTab1::OnBnClickedCalcTM()
{
	CalcTMDlg.ShowWindow(SW_SHOW);
}

BOOL DialogBarTab1::DestroyWindow()
{
	CalibratorDlg.DestroyWindow();
	CalcTEDlg.DestroyWindow();
	CalcTMDlg.DestroyWindow();
	return BarTemplate::DestroyWindow();
}

void MyGSL_Tester_Helper(Polarization pol, DoubleArray &Nexp, DoubleArray &teta_exp)
{
	// TE	
	//Metricon			nf = 1.9572		Hf = 993.8
	//Victor Ivanovich	nf = 1.9591		Hf = 989.6
	//Refractometr		nf = 2.01086	Hf = 1174.25
	//NewRefractmtr		nf = 1.959		Hf = 989.67
	//					
	// TM
	//Metricon			nf = 1.9490		Hf = 1026.2	
	//Victor Ivanovich	nf = 1.9488		Hf = 1028.7	
	//Refractometr		nf = 1.9488		Hf = 1028.57	
	//NewRefractmtr		nf = 1.9488		Hf = 1028.57

	FilmParams film; CalibrationParams cal; ControledLogMessage log;
	if (pol == TE)
	{
		log.T.Format("Log: ---=== TE testing ===---"); log << log.T;
	}
	if (pol == TM)
	{
		log.T.Format("Log: ---=== TM testing ===---"); log << log.T;
	}
	
	// tetsing calibration creation
	cal.CalculateFrom(Nexp, teta_exp, 2.15675, 1., 1.45705, 51*DEGREE, 632.8);
	log.T.Format("---Calibration (status = %s)---", gsl_strerror (cal.status)); log << log.T;
	log.T.Format("N0=%.10f L=%.10f", 
		cal.val[CalibrationParams::ind_N0], cal.val[CalibrationParams::ind_L]); log << log.T;
	log.T.Format("d0=%.10f fi0=%.10f", 
		cal.val[CalibrationParams::ind_d0],	cal.val[CalibrationParams::ind_fi0]); log << log.T;

	log.T.Format("errabs=%g errrel=%g dt=%.3f ms func_calls=%d",
		cal.err.abs, cal.err.rel, cal.dt.val(), cal.cntr.func_call); log << log.T;

	// testing pixel to angle conversion with calibration
	log.T.Format("---Calibrator----"); log << log.T;
	TypeArray<AngleFromCalibration> bettaexp; 
	for(int i = 0; i < Nexp.GetSize(); i++)
	{
		AngleFromCalibration angle;
		angle = cal.ConvertPixelToAngle(Nexp[i]); 
		log.T.Format("status = %s", gsl_strerror (angle.status)); log << log.T;
		log.T.Format("teta_calc=%.10f teta_orig=%.10f diff=%g%%", 
			angle.teta, teta_exp[i], (angle.teta - teta_exp[i])/teta_exp[i]); log << log.T;
		log.T.Format("dt=%.3f ms func_calls=%d", angle.dt.val(), angle.cntr.func_call); log << log.T;
		bettaexp << angle;
	}
		
	// test film parameters calculation
	film.Calculator(pol, cal, bettaexp); 
	log.T.Format("--FilmParams (status = %s)---", gsl_strerror (film.status)); log << log.T;	
	log.T.Format("n=%.10f H=%.10f nm", film.n, film.H); log << log.T;
	log.T.Format("errabs=%g errrel=%g fval=%.10f", 
		film.err.abs, film.err.rel, film.minimum_value); log << log.T;
	log.T.Format("dt=%.3f ms func_calls=%d", film.dt.val(), film.cntr.func_call); log << log.T;
	for( int i = 0; i < film.betta_teor.GetSize(); i++)
	{
		log.T.Format("betta_teor[%d]=%.5f betta_exp=%.5f", 
			film.betta_teor[i].n, film.betta_teor[i].val, film.betta_exp[i]); log << log.T;
	}	
	log.Dispatch();
}

void DialogBarTab1::OnBnClickedCalibrate()
{
	CalibratorDlg.ShowWindow(SW_SHOW);

	TSimplePointSeries *t1=NULL; 
	TSimplePointSeries::DataImportMsg *CHM1, *CHM2; CHM1=CHM2=NULL; 
	CMainFrame* mf=(CMainFrame*)AfxGetMainWnd(); 
	SimplePoint pnt; pnt.type.Set(GenericPnt);
	ImageWnd* parent=(ImageWnd*)Parent; void *x=NULL; MyTimer Timer1; ms dt1, dt2;
	DoubleArray Nexp_TE, Nexp_TM, teta_exp_TE, teta_exp_TM; CString T; 
	CalibrationParams cal_TE, cal_TM; AngleFromCalibration angle;

	CalcRParams params;
	params.i=FilmParams(1,			150,	0+1e-100); 
	params.f=FilmParams(1.84,		1082,	5e-3+1e-100);  
	params.s=FilmParams(1.45705,	1e6,	0+1e-100); 
	params.lambda=632.8; params.Np=2.14044; params.teta_min=15; params.teta_max=85;

	Nexp_TE << 1161 << 1951 << 2594 << 3077; 
	teta_exp_TE << 48.62*DEGREE << 55.24*DEGREE << 60.43*DEGREE << 64.02*DEGREE;
	Nexp_TM << 1014 << 1856 << 2514 << 2990 ;
	teta_exp_TM << 47.4*DEGREE << 54.08*DEGREE << 59.63*DEGREE << 63.4*DEGREE;
	MyGSL_Tester_Helper(TE, Nexp_TE, teta_exp_TE);
	MyGSL_Tester_Helper(TM, Nexp_TM, teta_exp_TM);
}

void DialogBarTab1::OnBnClicked_Fit()
{		
		CString str; UpdateData(); TPointVsErrorSeries *graph; SimplePoint pnt;		
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
					str.Format("No valid points found %d+/-%d",X0,dX); log->CreateEntry("ERR",str,LogMessage::high_pr);	
					log->Dispatch(); 
					return;
				}				
				buf.RemoveAll(); graph->GetValues(buf,n1,n2);					
			}			
			else
			{
				str.Format("No series matching criteria (ACTIVE) found"); log->CreateEntry("ERR",str,LogMessage::high_pr);	
				log->Dispatch(); return;
			}
		}
		else return;

		DoubleArray init; ParabolaFitFunc fiting;
		init << 1 << 1e-1 << 1e-2;		
		fiting.CalculateFrom(buf.x, buf.y, buf.dy, init);
		
		str.Format("************ ParabolaFit ******************"); *log << str;
		str.Format("status = %s", gsl_strerror (fiting.status)); *log << str;
		str.Format("----------------------------------"); *log << str;
		if (fiting.status == GSL_SUCCESS)
		{
			for(int i = 0; i < fiting.a.GetSize(); i++)
			{
				str.Format("x%d = %g +/- %g%%", i, fiting.a[i], 100*fiting.da[i]/fiting.a[i]); *log << str;
			}
			pnt.y = fiting.GetTop(pnt.x); str.Format("xmin = %g ymin = %g", pnt.x, pnt.y);
		}
		else
		{
			log->SetPriority(lmprHIGH);
		}
		str.Format("time = %g ms", fiting.dt.val()); *log << str;
		str.Format("func_cals = %d", fiting.cntr.func_call); *log << str;
		str.Format("iter_num = %d", fiting.cntr.iter); *log << str;
		
		if(fiting.status == GSL_SUCCESS && (x=chart->Series.GainAcsess(WRITE))!=NULL)
		{
			SeriesProtector guard(x); TSeriesArray& series(guard); str.Format("PolyFit%d",PolinomOrder);
			TSimplePointSeries* t1 = NULL; 
			if((t1=new TSimplePointSeries(str))!=0)	
			{
				series.Add(t1); 
				t1->_SymbolStyle::Set(NO_SYMBOL);
				t1->AssignColors(ColorsStyle(clRED,series.GetRandomColor()));
				t1->SetVisible(true); 

				t1->ParentUpdate(UPD_OFF);
				for(int i = 0; i < buf.x.GetSize(); i++) 
				{
					pnt.x = i; pnt.y = fiting.GetXrelY(pnt.x); t1->AddXY(pnt);
				}
				t1->ParentUpdate(UPD_ON);
			}	
		}
		chart->PostMessage(UM_CHART_SHOWALL);		
		log->Dispatch();
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

	FFTRealTransform::Params in(data.y), out;
	ret.status=FourierFilter(in,spec_wdth,out);
	ret.dt=out.dt;

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

struct MinimumsFitFilterParams
{	
	ms dt;
	int status; 
	TypeArray<ParabolaFitFunc> fitings;
};

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

MinimumsFitFilterParams* MinimumsFitFilterFunc(PointVsErrorArray &data,SimplePointArray &mins, int dn)
{
	MinimumsFitFilterParams* ret = new MinimumsFitFilterParams(); 
	MyTimer timer1; int i, dn_max=80, index; DoubleArray init; init << 1 << 1e-1 << 1e-2;
	ParabolaFitFunc fiting;  
	PointVsErrorArray buft; 

	timer1.Start();
	SimplePoint data0=data[0]; 	
	for(i = 0; i < mins.GetSize(); i++)
	{
		SimplePoint tt = mins[i];
		index = GetArrayIndex(data.x,mins[i].x);
		if( index < dn_max ) continue;

		buft.CopyFrom(data, 2*dn_max + 1, index - dn_max); 
		fiting.CalculateFrom(buft.x, buft.y, buft.dy, init);
	    if(fiting.status != GSL_SUCCESS) continue;
		if(fiting.a[2] < 0) continue;

		buft.CopyFrom(data,2*dn+1,index-dn); 
		fiting.CalculateFrom(buft.x, buft.y, buft.dy, init);
        if(fiting.status != GSL_SUCCESS) continue;
		if(fiting.a[2] < 0) continue;

		SimplePoint top; top.y = fiting.GetTop(top.x);
		if( top.x < fiting.leftmostX || top.x > fiting.rightmostX ) continue;

		ret->fitings << fiting;
	}
	ret->dt = timer1.StopStart();
	ret->status = S_OK;	
	return ret;
}

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
		for(int i = 0; i < data.fitings.GetSize() && t2 != NULL; i++)
		{
			val.y = data.fitings[i].GetTop(val.x); t2->AddXY(val);
		}		
		t2->ParentUpdate(UPD_ON);
	}
	
}

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
			str.Format("No series matching criteria (ACTIVE) found"); log->CreateEntry("ERR",str,LogMessage::high_pr);	
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

	str.Format("********Fourier smooth*************"); *log << str;
	str.Format("time=%g ms",FourierSmooth.dt.val()); *log << str;
	str.Format("********Minimums 1stage*************"); *log << str;
	str.Format("minimums=%d time=%g ms", LocateMinimums.minimumN, LocateMinimums.dt.val()); *log << str;
	str.Format("********Minimums 2stage*************"); *log << str;
	str.Format("minimums=%d time=%g ms", MinimumsFitFilter->fitings.GetSize(), MinimumsFitFilter->dt.val()); *log << str;
	str.Format("*********Total********************"); *log << str;
	str.Format("time=%g ms",dt1.val()); *log << str;

	log->Dispatch();
	delete MinimumsFitFilter;
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
		CalibratorDlg.PostMessage(UM_SERIES_UPDATE,wParam,lParam);	
		CalcTEDlg.PostMessage(UM_SERIES_UPDATE,wParam,lParam);	
		CalcTMDlg.PostMessage(UM_SERIES_UPDATE,wParam,lParam);	
	}
	return 0;
}

void DialogBarTab1::OnBnClickedKneeTest()
{
	CString str; UpdateData(); TPointVsErrorSeries *graph; KneeFitFunc fiting;  DoubleArray init;
	LogMessage *log=new LogMessage(); size_t i = 0; SimplePoint pnt;

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
				str.Format("ERR: No valid points found %d+/-%d",X0,dX); *log << str;	
				log->SetPriority(lmprHIGH); log->Dispatch(); return;
			}				
			buf.RemoveAll(); graph->GetValues(buf,n1,n2);					
		}			
		else
		{
			str.Format("ERR: No series matching criteria (ACTIVE) found"); *log << str;	
			log->SetPriority(lmprHIGH); log->Dispatch(); return;
		}
	}
	else return;
	
	init << 1 << 0.1 << 0.1 << 0.1; fiting.CalculateFrom(buf.x, buf.y, buf.dy, init);
	
	str.Format("************ KneeFit **********************"); *log << str;
	str.Format("status = %s", gsl_strerror (fiting.status)); *log << str;
	if (fiting.status != GSL_SUCCESS) log->SetPriority(lmprHIGH);
	str.Format("----------------------------------"); *log << str;
	
	if (fiting.status == GSL_SUCCESS)
	{
		for(int i = 0; i < fiting.a.GetSize(); i++)
		{
			str.Format("x%d = %g +/- %g%%", i, fiting.a[i], 100*fiting.da[i]/fiting.a[i]); *log << str;
		}
		pnt.y = fiting.GetInflection(pnt.x, level);
		str.Format("xmin = %g ymin = %g", pnt.x, pnt.y); *log << str;
	}
	str.Format("time = %g ms", fiting.dt.val()); *log << str;
	str.Format("func_cals = %d", fiting.cntr.func_call); *log << str;
	str.Format("iter_num = %d", fiting.cntr.iter); *log << str;
		
	if(fiting.status == GSL_SUCCESS && (x=chart->Series.GainAcsess(WRITE))!=NULL)
	{
		SeriesProtector guard(x); TSeriesArray& series(guard); str.Format("PolyFit%d",PolinomOrder);
		TSimplePointSeries* t1=NULL; 
		if( (t1=new TSimplePointSeries("FinalMins"))!=0)	
		{
			series.Add(t1); t1->AssignColors(ColorsStyle(clWHITE,clWHITE));
			t1->_LineStyle::Set(NO_LINE); t1->_SymbolStyle::Set(VERT_LINE);
			t1->_SymbolStyle::dy=10;
			t1->SetVisible(true); 
			t1->AddXY(pnt);
		}
		if((t1=new TSimplePointSeries(str))!=0)	
		{
			series.Add(t1); 
			t1->_SymbolStyle::Set(NO_SYMBOL);
			t1->AssignColors(ColorsStyle(clRED,series.GetRandomColor()));
			t1->SetVisible(true); 

			t1->ParentUpdate(UPD_OFF);

			for(int i = 0; i < buf.x.GetSize(); i++) 
			{
				pnt.x = i; pnt.y = fiting.GetXrelY(pnt.x); t1->AddXY(pnt);
			}
			t1->ParentUpdate(UPD_ON);
		}
	}
	chart->PostMessage(UM_CHART_SHOWALL);
	log->Dispatch();
}
 