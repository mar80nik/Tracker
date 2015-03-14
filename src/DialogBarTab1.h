#pragma once

#include "resource.h"
#include "BarTemplate.h"
#include "tchart.h"
#include "systemconfig.h"
#include "mythread.h"
#include "afxwin.h"

int GetArrayIndex(DoubleArray& arr, double x );

class DialogBarTab1 : public BarTemplate
{		
	friend class MainStagesCtrl;
	friend class StopStageCtrl;
	friend class MainChartWnd;
protected:	
	CString Name;
	int CheckButtonsStatus;
	CFont font1;
	ProtectedSeriesArray* Series;
	CalibrationParams cal;
public:
	DialogBarTab1(CWnd* pParent = NULL);   // standard constructor	
	//{{AFX_DATA(DialogBarTab1)
	enum { IDD = IDD_DIALOGBARTAB11 };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(DialogBarTab1)
	public:
	virtual void Serialize(CArchive& ar);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
protected:
	//{{AFX_MSG(DialogBarTab1)
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocus();
	virtual void OnOK() {};
	virtual void OnCancel() {};
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
public:
	int spec_wdth;
	int Nmin;
	int Nmax;
	int PolinomOrder;

	int minimum_widht_2, dX;
	double X0;

	virtual BOOL DestroyWindow();
	afx_msg void OnBnClickedLoadCalibration();
	LRESULT OnSeriesUpdate(WPARAM wParam, LPARAM lParam );
	TPointVsErrorSeries* GetSeries(TSeriesArray& series);

	CComboBox SeriesCombo;
	double Xmin;
	double Xmax;
	double level;
};

class MainChartWnd: public TChart
{
	friend class CMainFrame;
	DECLARE_DYNAMIC(MainChartWnd)
protected:
	DialogBarTab1 Panel;
public:
	MainChartWnd();
	virtual ~MainChartWnd() {} 
	virtual void Serialize(CArchive& ar);
protected:
	// Generated message map functions
	//{{AFX_MSG(DialogBarTab1)
	afx_msg LRESULT  OnStageUpdate(WPARAM wParam, LPARAM lParam );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};