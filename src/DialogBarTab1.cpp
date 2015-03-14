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
	//{{AFX_DATA_INIT(DialogBarTab1)
	//}}AFX_DATA_INIT
	Name="MainControlTab";
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
	DDX_Text(pDX, IDC_EDIT3, minimum_widht_2);
	DDV_MinMaxInt(pDX, minimum_widht_2, 1, 3000);
	DDX_Control(pDX, IDC_COMBO1, SeriesCombo);
	DDX_Text(pDX, IDC_EDIT7, Xmin);
	DDX_Text(pDX, IDC_EDIT8, Xmax);
	DDX_Text(pDX, IDC_EDIT9, level);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// DialogBarTab1 message handlers
int DialogBarTab1::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if (BarTemplate::OnCreate(lpCreateStruct) == -1) return -1;
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

BOOL DialogBarTab1::DestroyWindow()
{
	return BarTemplate::DestroyWindow();
}

//////////////////////////////////////////////////////////////////////////

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
