// SeriesStyleDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SeriesStyleDialog.h"


// SeriesStyleDialog dialog

IMPLEMENT_DYNAMIC(SeriesStyleDialog, CDialog)

SeriesStyleDialog::SeriesStyleDialog(CWnd* pParent /*=NULL*/)
	: CDialog(SeriesStyleDialog::IDD, pParent)
	, SeriesName(_T(""))
{


}

SeriesStyleDialog::~SeriesStyleDialog()
{
}

void SeriesStyleDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, SeriesName);
	DDX_Control(pDX, IDC_COMBO1, SymbolStyleCombo);
	DDX_Control(pDX, IDC_COMBO4, LineStyleCombo);
	DDX_Control(pDX, IDC_COMBO5, ErrBarsStyleCombo);
	DDX_Control(pDX, IDC_BUTTON1, LineColorBtn);
	DDX_Control(pDX, IDC_BUTTON3, FillColorBtn);
}


BEGIN_MESSAGE_MAP(SeriesStyleDialog, CDialog)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_BUTTON2, &SeriesStyleDialog::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON1, &SeriesStyleDialog::OnBnClickedLineColor)
	ON_BN_CLICKED(IDC_BUTTON3, &SeriesStyleDialog::OnBnClickedFillColor)
	ON_WM_CTLCOLOR()
	ON_CBN_SELCHANGE(IDC_COMBO1, &SeriesStyleDialog::OnBnClickedButton2)
	ON_CBN_SELCHANGE(IDC_COMBO4, &SeriesStyleDialog::OnBnClickedButton2)
	ON_CBN_SELCHANGE(IDC_COMBO5, &SeriesStyleDialog::OnCbnSelchangeCombo5)
END_MESSAGE_MAP()


// SeriesStyleDialog message handlers

BOOL SeriesStyleDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	int n;

	n=LineStyleCombo.AddString("No line"); LineStyleCombo.SetItemData(n,ChartLineStyles::NO_LINE);
	n=LineStyleCombo.AddString("Straight");	LineStyleCombo.SetItemData(n,ChartLineStyles::STRAIGHT);

	n=SymbolStyleCombo.AddString("No symbol");	SymbolStyleCombo.SetItemData(n,ChartSymbolStyles::NO_SYMBOL);
	n=SymbolStyleCombo.AddString("Circle");	SymbolStyleCombo.SetItemData(n,ChartSymbolStyles::CIRCLE);
	n=SymbolStyleCombo.AddString("Cross 45");	SymbolStyleCombo.SetItemData(n,ChartSymbolStyles::CROSS45);
	n=SymbolStyleCombo.AddString("Vertical line");	SymbolStyleCombo.SetItemData(n,ChartSymbolStyles::VERT_LINE);

	n=ErrBarsStyleCombo.AddString("No bars");	ErrBarsStyleCombo.SetItemData(n,ChartErrorBarStyles::NO_BARS);
	n=ErrBarsStyleCombo.AddString("Both bars");	ErrBarsStyleCombo.SetItemData(n,ChartErrorBarStyles::POINTvsERROR_BAR);

	void *x; int i;
	if((x=Series->GainAcsess(READ))!=NULL)
	{
		SeriesProtector guard(x); TSeriesArray& series(guard);
		TChartSeries *graph=series[graph_num];

		ChartSeriesStyleParams style=graph->GetStyle();
		SeriesName=graph->Name;
		for(i=0;i<LineStyleCombo.GetCount();i++)
		{
			if(LineStyleCombo.GetItemData(i)==style.LineStyleStyleParams::style) 
			{
				LineStyleCombo.SetCurSel(i); break;
			}
		}
		for(i=0;i<SymbolStyleCombo.GetCount();i++)
		{
			if(SymbolStyleCombo.GetItemData(i)==style.SymbolStyleStyleParams::style) 
			{
				SymbolStyleCombo.SetCurSel(i); break;
			}
		}
		for(i=0;i<ErrBarsStyleCombo.GetCount();i++)
		{
			if(ErrBarsStyleCombo.GetItemData(i)==style.ErrorBarStyleParams::style) 
			{
				ErrBarsStyleCombo.SetCurSel(i); break;
			}
		}

		LineColorBtn.FaceColor=graph->PColor;
		FillColorBtn.FaceColor=graph->BColor;

		if(graph->GetSeriesType()!=POINTvsERROR) 
			ErrBarsStyleCombo.EnableWindow(FALSE);
		UpdateData(0);
	}

	
	return TRUE;  
}



int SeriesStyleDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	SetWindowPos(NULL,400,400,0,0,SWP_NOSIZE | SWP_NOZORDER);

	return 0;
}

void SeriesStyleDialog::OnBnClickedButton2()
{
	void *x; UpdateData();
	if((x=Series->GainAcsess(WRITE))!=NULL)
	{
		SeriesProtector guard(x); TSeriesArray& series(guard);
		TChartSeries *graph=series[graph_num];

		ChartSeriesStyleParams style=graph->GetStyle();
		graph->Name=SeriesName;

		switch (LineStyleCombo.GetItemData(LineStyleCombo.GetCurSel()))
		{
		case STRAIGHT: style.LineStyleStyleParams::style=STRAIGHT; break;
		default: style.LineStyleStyleParams::style=NO_LINE; break;
		}

		switch (SymbolStyleCombo.GetItemData(SymbolStyleCombo.GetCurSel()))
		{
		case CIRCLE: style.SymbolStyleStyleParams::style=CIRCLE; break;
		case CROSS45: style.SymbolStyleStyleParams::style=CROSS45; break;
		case VERT_LINE: style.SymbolStyleStyleParams::style=VERT_LINE; break;
		default: style.SymbolStyleStyleParams::style=NO_SYMBOL; break;
		}

		switch (ErrBarsStyleCombo.GetItemData(ErrBarsStyleCombo.GetCurSel()))
		{
		case POINTvsERROR_BAR: style.ErrorBarStyleParams::style=POINTvsERROR_BAR; break;
		default: style.ErrorBarStyleParams::style=NO_BARS; break;
		}

		graph->SetStyle(style);
		graph->PostParentMessage(UM_SERIES_UPDATE);	

	}
	
}

void SeriesStyleDialog::OnBnClickedLineColor()
{
	OnBtnChangeColor(&LineColorBtn);
}

void SeriesStyleDialog::OnBnClickedFillColor()
{
	OnBtnChangeColor(&FillColorBtn);
}

void SeriesStyleDialog::OnBtnChangeColor(CWnd* pWnd)
{
	CColorDialog dlg1; OwnerDrawBtn* pBtn=(OwnerDrawBtn*)pWnd;
	if(dlg1.DoModal()==IDOK)
	{
		pBtn->FaceColor=dlg1.GetColor();
		pBtn->Invalidate(TRUE);

		void *x; 
		if((x=Series->GainAcsess(WRITE))!=NULL)
		{
			SeriesProtector guard(x); TSeriesArray& series(guard);
			TChartSeries *graph=series[graph_num];

			graph->AssignColors(ColorsStyle(FillColorBtn.FaceColor,LineColorBtn.FaceColor));
			graph->PostParentMessage(UM_SERIES_UPDATE);	

		}

	}
}

HBRUSH SeriesStyleDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if(pWnd==&LineColorBtn || pWnd==&FillColorBtn)
	{
		OwnerDrawBtn* pBtn=(OwnerDrawBtn*)pWnd;

		LOGBRUSH LogBrush;

		pBtn->FaceBrush.GetLogBrush(&LogBrush);
		if(LogBrush.lbColor!=pBtn->FaceColor)
		{
			pBtn->FaceBrush.DeleteObject();
			pBtn->FaceBrush.CreateSolidBrush(pBtn->FaceColor);
		}
		return pBtn->FaceBrush;
	}
	else
	{
		return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	}
}

void SeriesStyleDialog::OnCbnSelchangeCombo1()
{
	OnBnClickedButton2();	
}

void SeriesStyleDialog::OnCbnSelchangeCombo4()
{
	OnBnClickedButton2();		
}

void SeriesStyleDialog::OnCbnSelchangeCombo5()
{
	OnBnClickedButton2();
}
