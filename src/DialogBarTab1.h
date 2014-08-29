#if !defined(AFX_DIALOGBARTAB1_H__45599A3E_AB64_4D60_A561_550B80F651DC__INCLUDED_)
#define AFX_DIALOGBARTAB1_H__45599A3E_AB64_4D60_A561_550B80F651DC__INCLUDED_

//#include "myprogressbar.h"
//#include "mybutton.h"
#include "resource.h"
#include "BarTemplate.h"
#include "tchart.h"
#include "systemconfig.h"
#include "mythread.h"
//#include "abstractcomstage.h"
#include "afxwin.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DialogBarTab1.h : header file
/////////////////////////////////////////////////////////////////////////////
// DialogBarTab1 dialog
//class SeriesArray;
/*
struct DialogBarTab1Config 
{
	double l1, l2, scanstep, scanspeed; int nummes; double timemes;
};

class MVThread;
class WarningDialog;
class MotorStage;

struct ScenarioParams: public ErrorInspector
{
protected:
	ErrorsArray err;
public:
	PointBckgType regime;
	ComVoltStage::SetupParams vp;
	SDRstage::SetupParams mp;
	fm10 scanStep, curl;
	int sign, numPoints; 
	WindowAddress UpdateWnd;
	//variables
	FilterHeadPos filter_pos; fm10 scenl;
	ProtectedSeriesArray* Series;
	//constants
	BYTE automatic;

	ScenarioParams() {AttachErrors(&err);}
};

enum CheckButtonCodes {Chnl0=1<<0, Chnl1=1<<1, Chnl01=1<<2};

class MainStagesCtrl: public MyButton
{
	friend class DialogBarTab1;
protected:
	DialogBarTab1* Parent;

	virtual RequestForAcsessMsg* GetRequestInfo();

	int Setup(void *p);
	int CreateSeries();
	scenario* CreateFiltersSubScenario(ScenarioParams &Params);
	scenario* CreatePreWCHSubScenario(ScenarioParams &Params);
	scenario* CreateMainSubScenario(ScenarioParams &Params);
	scenario* CreateVoltSubScenario(ScenarioParams &Params);
	SDRstage* CreateMotorStage(CString name);	
public:
	enum StartButtonStates {Start, Pause, Resume};	

	MainStagesCtrl() {}
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnAccessGranted(WPARAM,LPARAM);
	afx_msg LRESULT  OnStop(WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT  OnPause(WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT  OnContinue(WPARAM wParam, LPARAM lParam );

};

class StopStageCtrl: public MyButton
{
	friend class DialogBarTab1;
protected:
	DialogBarTab1* Parent;

	virtual RequestForAcsessMsg* GetRequestInfo();
	DECLARE_MESSAGE_MAP()
};
class MainChartWnd;
*/
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

//	MyButton* GetButtonsClick(int* _n=0);
//	SDRstage::SetupParams GetMotorSetupParams();
public:
	DialogBarTab1(CWnd* pParent = NULL);   // standard constructor	
//	void CreateScenarioParams(ScenarioParams &Params);
// Dialog Data
	//{{AFX_DATA(DialogBarTab1)
	enum { IDD = IDD_DIALOGBARTAB11 };
//	MyProgressBar	ProgressBar;
//	StopStageCtrl StopButton;
//	MainStagesCtrl StartButton;

	CButton Channel0Check, Channel1Check, Chnl0_1Check;
	double	LMax, LMin,ScanStep;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DialogBarTab1)
	public:
	virtual void Serialize(CArchive& ar);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(DialogBarTab1)
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocus();
//	afx_msg LRESULT  OnStageUpdate(WPARAM wParam, LPARAM lParam );
//	virtual afx_msg LRESULT  OnStop(WPARAM wParam, LPARAM lParam );
//	virtual afx_msg LRESULT  OnStart(WPARAM wParam, LPARAM lParam );
//	virtual afx_msg LRESULT  OnPause(WPARAM wParam, LPARAM lParam );
	virtual void OnOK() {};
	virtual void OnCancel() {};
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	int spec_wdth;
	int Nmin;
	int Nmax;
	int PolinomOrder;

	int minimum_widht_2, dX;
	double X0;

	afx_msg void OnBnClicked_Fit();
	afx_msg void OnBnClicked_Locate();
public:
	afx_msg void OnBnClickedLoadCalibration();
	LRESULT OnSeriesUpdate(WPARAM wParam, LPARAM lParam );
	TPointVsErrorSeries* GetSeries(TSeriesArray& series);
public:
	CComboBox SeriesCombo;
	afx_msg void OnBnClickedButton4();
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
//	MessagesInspector Inspector1;

	MainChartWnd();
	virtual ~MainChartWnd() {} 
	virtual void Serialize(CArchive& ar);
protected:
	// Generated message map functions
	//{{AFX_MSG(DialogBarTab1)
	afx_msg LRESULT  OnStageUpdate(WPARAM wParam, LPARAM lParam );
	virtual afx_msg LRESULT  OnStop(WPARAM wParam, LPARAM lParam );
	virtual afx_msg LRESULT  OnStart(WPARAM wParam, LPARAM lParam );
	virtual afx_msg LRESULT  OnPause(WPARAM wParam, LPARAM lParam );
	virtual afx_msg LRESULT  OnContinue(WPARAM wParam, LPARAM lParam );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
//	SeriesArray* GetSeries() {return Series;}
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOGBARTAB1_H__45599A3E_AB64_4D60_A561_550B80F651DC__INCLUDED_)

