#pragma once
#include "BarTemplate.h"
#include "camera.h"
#include "captureWnd.h"
#include "calibratorDialog.h"
#include "CalcTEDialog.h"

// ImageWnd
class ImageWndCtrlsTab : public BarTemplate
{		
protected:	
	CString Name;	
public:
	CalibratorDialog CalibratorDlg;
	CalcTEDialog	CalcTEDlg, CalcTMDlg;

	ImageWndCtrlsTab(CWnd* pParent = NULL);   // standard constructor	
	// Dialog Data
	//{{AFX_DATA(DialogBarTab1)
	enum { IDD = IDD_DIALOGBARTAB1 };
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DialogBarTab1)
public:
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(DialogBarTab1)
	virtual BOOL OnInitDialog();	
	virtual void OnOK() {};
	virtual void OnCancel() {};
//	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedScan();
	int stroka, AvrRange, Xmin, Xmax;
	afx_msg void OnBnClickedCalibrate();
public:
	virtual BOOL DestroyWindow();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnBnClickedCalcTE();
	afx_msg void OnBnClickedButton5();
public:
	afx_msg void OnEnKillfocusEdit1();
public:
	afx_msg void OnBnClickedCalcTM();
};

class ImageWnd;

class PicWndScanLine
{
protected:
	BOOL ToErase; BMPanvas *buf;
	CPoint lastL, lastR; int lastdy; 

public:
	int dy;
	CPoint curL, curR;

	virtual void Draw(BMPanvas* Parent);
	virtual void Erase();
public:
	PicWndScanLine() {ToErase=FALSE; curL.y=curR.y=-1; buf=NULL;}
};

enum CaptureWndMSGS {UM_CAPTURE_REQUEST=4000, UM_CAPTURE_READY};

class ImageWndPicWnd: public CWnd
{
protected:
	CButton CaptureButton;
	CMenu menu1;
public:
	BMPanvas org, ava;
	CRect ClientArea;
	ImageWnd* Parent;
	CFont font1;
	CString FileName;
	PicWndScanLine ScanLine;

enum {CaptureBtnID=234234};

	ImageWndPicWnd();
	virtual ~ImageWndPicWnd();
	void LoadPic(CString T);
	void UpdateNow(void);

	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnDropFiles(HDROP hDropInfo);	
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMvButton();
	LRESULT OnCaptureReady( WPARAM wParam, LPARAM lParam );
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	void OnPicWndErase();
	void OnPicWndSave();
	void EraseAva();
	afx_msg void OnMove(int x, int y);
};

class ImageWnd : public CWnd
{
	friend class ImageWndCtrlsTab;
	friend class ImageWndPicWnd;
	DECLARE_DYNAMIC(ImageWnd)
protected:
	CRect dark_ava_r,cupol_ava_r,strips_ava_r;
	ImageWndPicWnd dark,cupol,strips;
    int scale;	
	CScrollBar VertScroll;
public:
	ImageWndCtrlsTab Ctrls;
	CaptureWnd	CameraWnd;

public:
	ImageWnd();
	virtual ~ImageWnd();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	void DrawScanLine(CPoint curL, CPoint curR);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void OnChildMove();
	afx_msg void OnDestroy();
	void * GetChartFromParent();

};


