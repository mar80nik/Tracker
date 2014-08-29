#pragma once

#include "resource.h"
#include "TChart.h"
#include "afxwin.h"
#include "metricon.h"
#include "SystemConfig.h"


#define modes_num 4

class CalibratorDialog : public CDialog
{
	DECLARE_DYNAMIC(CalibratorDialog)

public:
	CalibratorDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CalibratorDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_CAL };

protected:
	ProtectedSeriesArray* Series;
	CalibrationParams cal;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	double N[modes_num], Q[modes_num];
	double Nc, Lc, dc, fic;
	void * GetChartFromParent();;
	
	CComboBox SeriesCombo;
	CComboBox SeriesCombo1;
public:
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedCalculateCal();
	afx_msg void OnBnClickedSaveToConfig();
	afx_msg void OnCbnSelchangeCombo3();
	afx_msg void OnBnClickedButton8();
	LRESULT OnSeriesUpdate(WPARAM wParam, LPARAM lParam );
};