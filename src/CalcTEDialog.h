#pragma once
#include "afxwin.h"
#include "resource.h"
#include "TChart/TChartSeries.h"
#include "metricon.h"
#include "SystemConfig.h"

#define modes_num 4

// CalcTEDialog dialog

class CalcTEDialog : public CDialog
{
	DECLARE_DYNAMIC(CalcTEDialog)

public:
	double N[modes_num], Q[modes_num], beta[modes_num];
	BOOL IsTM;

	CalcTEDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CalcTEDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_CALCTE };

protected:
	ProtectedSeriesArray* Series;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	LRESULT OnSeriesUpdate(WPARAM wParam, LPARAM lParam );
public:
	afx_msg void OnBnClickedButton4();
	afx_msg void OnCbnSelchangeCombo1();
public:
	CComboBox SeriesCombo;
public:
	afx_msg void OnBnClickedButton8();
public:
	double nf;
public:
	double hf;
public:
	afx_msg void OnBnClickedButton11();
};
