#pragma once

#include "TChartSeries.h"
#include "resource.h"
#include "OwnerDrawBtn.h"

class SeriesStyleDialog : public CDialog
{
	DECLARE_DYNAMIC(SeriesStyleDialog)
protected:
	CBrush LineColorBtnBrush;
	CButton myButton1;
public:
	ProtectedSeriesArray* Series; int graph_num;

	SeriesStyleDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~SeriesStyleDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG18 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CString SeriesName;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	CComboBox SymbolStyleCombo;
	CComboBox LineStyleCombo;
	CComboBox ErrBarsStyleCombo;
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedLineColor();
	afx_msg void OnBnClickedFillColor();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void OnBtnChangeColor(CWnd* btn);
	OwnerDrawBtn LineColorBtn, FillColorBtn;
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnCbnSelchangeCombo4();
	afx_msg void OnCbnSelchangeCombo5();
};
