#pragma once
#include "BarTemplate.h"
#include "camera.h"
#include "captureWnd.h"
#include "calibratorDialog.h"
#include "CalcTEDialog.h"
#include "afxwin.h"

enum CaptureWndMSGS {UM_CAPTURE_REQUEST=4000, UM_CAPTURE_READY};

class ImageWndCtrlsCEditInterceptor : public CEdit
{
	DECLARE_DYNAMIC(ImageWndCtrlsCEditInterceptor)
public:
	ImageWndCtrlsCEditInterceptor() {};
	virtual ~ImageWndCtrlsCEditInterceptor() {};
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};

class ImageWnd : public CWnd
{
	struct AvaPicRgn: public CRect {};
	struct OrgPicRgn: public CRect {};
	
	class CtrlsTab : public BarTemplate
	{		
	protected:	
		CString Name;	
	public:
		enum { IDD = IDD_DIALOGBARTAB1 };

		CalibratorDialog CalibratorDlg;
		CalcTEDialog	CalcTEDlg, CalcTMDlg;
		int stroka, AvrRange, Xmin, Xmax;

		CtrlsTab(CWnd* pParent = NULL);  
		OrgPicRgn GetScanRgnFromCtrls();
		void InitScanRgnCtrlsFields(const OrgPicRgn&);
	protected:
		virtual void DoDataExchange(CDataExchange* pDX);   
	protected:
		virtual BOOL OnInitDialog();	
		virtual void OnOK() {};
		virtual void OnCancel() {};
		DECLARE_MESSAGE_MAP()
	public:
		afx_msg void OnBnClickedScan();
		afx_msg void OnBnClickedCalibrate();
		virtual BOOL DestroyWindow();
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnBnClickedCalcTE();
		afx_msg void OnBnClickedButton5();
		afx_msg void OnEnKillfocusEdit1();
		afx_msg void OnBnClickedCalcTM();
	
//		virtual BOOL OnInitDialog();

		ImageWndCtrlsCEditInterceptor XminCtrl;
	};

	class PicWnd: public CWnd
	{
		enum ScanRgnDrawModes { DRAW, ERASE };
		class c_ScanRgn: protected AvaPicRgn
		{
		protected:
			BOOL ToErase; 
			AvaPicRgn last;

			void Draw(BMPanvas* bmp, const AvaPicRgn& rgn, ScanRgnDrawModes mode );
		public:
			c_ScanRgn() { ToErase=FALSE; }

			virtual void Draw(BMPanvas* Parent);
			virtual void Erase(BMPanvas * canvas);
			void Set(const AvaPicRgn& rgn);
		};

	protected:
		CButton CaptureButton;
		CMenu menu1;

		AvaPicRgn Convert(const OrgPicRgn&);
		OrgPicRgn Convert(const AvaPicRgn&);
		BOOL IsRgnInAva( const AvaPicRgn& );	
	public:
		BMPanvas org, ava;
		CRect ClientArea;
		ImageWnd* Parent;
		CFont font1;
		CString FileName;
		c_ScanRgn ScanRgn;

		enum {CaptureBtnID=234234};

		PicWnd();
		virtual ~PicWnd();
		void LoadPic(CString T);
		void UpdateNow(void);
		void OnPicWndErase();
		void OnPicWndSave();
		void EraseAva();
		void SetScanRgn(const OrgPicRgn&);
		OrgPicRgn Validate(const OrgPicRgn&);

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
		afx_msg void OnMove(int x, int y);
	};

	class c_ScanRgn: protected OrgPicRgn
	{
	public:
		void Draw();
		void SetCoord(const OrgPicRgn& rgn);
	public:
		c_ScanRgn() {}
	};

	DECLARE_DYNAMIC(ImageWnd)
protected:
	PicWnd dark, cupol, strips;
    int scale;	
	CScrollBar VertScroll;
	c_ScanRgn ScanRgn;
public:
	CtrlsTab Ctrls;
	CaptureWnd	CameraWnd;
public:
	ImageWnd();
	virtual ~ImageWnd();

protected:
	DECLARE_MESSAGE_MAP()
public:
	void OnChildMove();
	void SetScanRgn(const OrgPicRgn&);
	void * GetChartFromParent();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);	
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnEnUpdateEdit3();
	afx_msg void OnNMThemeChangedEdit3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);	
};




