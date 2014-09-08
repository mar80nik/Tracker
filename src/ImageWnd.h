#pragma once
#include "BarTemplate.h"
#include "camera.h"
#include "captureWnd.h"
#include "calibratorDialog.h"
#include "CalcTEDialog.h"
#include "afxwin.h"

enum CaptureWndMSGS {UM_CAPTURE_REQUEST=4000, UM_CAPTURE_READY};
enum CEditInterceptorMessages {UM_BUTTON_ITERCEPTED = 3000};

class CEditInterceptor : public CEdit
{
	DECLARE_DYNAMIC(CEditInterceptor)
public:
	CEditInterceptor() {};
	virtual ~CEditInterceptor() {};
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
};

class ImageWnd : public CWnd
{
	struct AvaPoint: public CPoint 
	{ 
		AvaPoint(): CPoint() {}
		AvaPoint(const CPoint& pnt): CPoint(pnt) {}
	};
	struct OrgPoint: public CPoint
	{
		OrgPoint(): CPoint() {}
		OrgPoint(const CPoint& pnt): CPoint(pnt) {}	
	};

	enum DrawModes { DRAW, ERASE };
	enum MarkerNames { BGN, END};

	class CtrlsTab : public BarTemplate
	{		
	protected:	
		CString Name;	
	public:
		enum { IDD = IDD_DIALOGBARTAB1 };

		CalibratorDialog CalibratorDlg;
		CalcTEDialog	CalcTEDlg, CalcTMDlg;
		int stroka, AvrRange, Xmin, Xmax;
		CEditInterceptor XminCtrl, XmaxCtrl, strokaCtrl, AvrRangeCtrl;

		CtrlsTab(CWnd* pParent = NULL);  
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
		LRESULT OnButtonIntercepted(WPARAM wParam, LPARAM lParam );
	};

	class PicWnd: public CWnd
	{
	public:
		class AvaMarker: public AvaPoint
		{
		protected:
			BOOL ToErase; 
			AvaPoint last;

			void Draw(BMPanvas* bmp, const AvaPoint& rgn, DrawModes mode );
		public:
			AvaMarker() { ToErase=FALSE; }
			virtual void Draw(BMPanvas* Parent);
			virtual void Erase(BMPanvas * canvas);		
			AvaMarker& operator= (const AvaPoint& pnt) { *((AvaPoint*)this) = pnt; return *this; }
		};

	protected:
		CButton CaptureButton;
		CMenu menu1; AvaMarker MarkerAvaBGN, MarkerAvaEND;
				
	public:
		BMPanvas org, ava;
		CRect ClientArea;
		ImageWnd* Parent;
		CFont font1;
		CString FileName;
		enum {CaptureBtnID=234234};

		PicWnd();
		virtual ~PicWnd();
		void LoadPic(CString T);
		void UpdateNow(void);
		void OnPicWndErase();
		void OnPicWndSave();
		void EraseAva();
		void SetMarker(const AvaPoint& mark, MarkerNames pos);
		AvaPoint Convert(const OrgPoint&);
		OrgPoint Convert(const AvaPoint&);
		OrgPoint ValidatePnt( const OrgPoint& rgn);

		DECLARE_MESSAGE_MAP()
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnPaint();
		afx_msg void OnDropFiles(HDROP hDropInfo);	
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnCaptureButton();
		LRESULT OnCaptureReady( WPARAM wParam, LPARAM lParam );
		afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
		afx_msg void OnMove(int x, int y);	
		void ConvertOrgToGrayscale();
	};
	
	DECLARE_DYNAMIC(ImageWnd)
protected:
	PicWnd fiber;
    int scale;	OrgPoint MarkerBGN, MarkerEND;
	CScrollBar VertScroll;
public:
	CtrlsTab Ctrls;
	CaptureWnd	CameraWnd;

	ImageWnd();
	virtual ~ImageWnd();
protected:
	DECLARE_MESSAGE_MAP()
public:
	void OnChildMove();
	void * GetChartFromParent();
	void SetMarker(const OrgPoint& mark, MarkerNames pos);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);	
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnEnUpdateEdit3();
	afx_msg void OnNMThemeChangedEdit3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);	
};




