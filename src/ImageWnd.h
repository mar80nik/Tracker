#pragma once
#include "BarTemplate.h"
#include "camera.h"
#include "captureWnd.h"
#include "afxwin.h"
#include "fittings.h"

#define CrossWidth 10

enum CaptureWndMSGS {UM_CAPTURE_REQUEST=4000, UM_CAPTURE_READY};
enum CEditInterceptorMessages {UM_BUTTON_ITERCEPTED = 3000};

enum HelperEvent {
	EvntOnCaptureButton, EvntOnCaptureReady,
	RSLT_HELPER_COMPLETE, RSLT_OK, RSLT_BMP_ERR, RSLT_ERR
};
//================================================
struct BaseForHelper
{
	virtual HelperEvent Update(const HelperEvent &event) = 0;
	virtual ~BaseForHelper() {}
};

//================================================

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

struct ScanRgnData
{ int stroka, Xmin, Xmax, AvrRange;};

enum ScanLineRotationMode { BEG, CNTR, END };

struct ScanLineData
{ 
protected:
	HRESULT status;
public:
	CPoint beg, end;
	double dX, dY, len, cosfi, sinfi;
	
	ScanLineData() {dX = dY= len = 0; cosfi = 1.; sinfi = 0; status = E_FAIL;};
	HRESULT Init(const CPoint &_beg, const CPoint &_end);
	BOOL IsInited() const { return (status == S_OK);}
	void RotateByAngle(const double RadiansAngle, const ScanLineRotationMode mode);
protected:
	double Get_Length() const;
	double Get_dX() const;
	double Get_dY() const;
	HRESULT Get_cosfi(double &cosfi) const;
	HRESULT Get_sinfi(double &sinfi) const;
};

struct AccumInfo
{		
	USHORT w, h, n; BYTE HasErrors;
	virtual void Serialize(CArchive &ar);
	AccumInfo() {w = h = n = 0; HasErrors = FALSE; }
	size_t GetSumsSize() const;
	size_t GetCompressorBufferSize() const;
};

struct PointVsError3D
{
	double x, y, z, dz;
	PointVsError3D() {x = y = z = dz = 0.;};
};

struct ImagesAccumulator: public AccumInfo
{
protected:
	BYTE *sums; size_t OldSumsSize;
public:
	BMPanvas *bmp; 

	ImagesAccumulator();
	~ImagesAccumulator() {Reset();};
	void Reset();
	void ResetSums();	
	unsigned short *GetSum() const;
	unsigned int *GetSums2() const;
	HRESULT GetPicRgn(CRect&) const;

	HRESULT Initialize(int _w, int _h, BOOL hasErrors = TRUE);
	HRESULT FillAccum(BMPanvas *src, MyTimer *Timer1 = NULL);
	void ConvertToBitmap(CWnd *ref, MyTimer *Timer1 = NULL);
	HRESULT SaveTo(const CString &file);
	HRESULT LoadFrom(const CString &file);
	void ScanLine( void *buf, const ScanRgnData &data, MyTimer *Timer1 = NULL) const;
	void ScanArbitaryLine( void * const buf, const ScanLineData &data, MyTimer *Timer1 = NULL) const;
	PointVsError3D GetPoint(const CPoint &pnt) const;
	BOOL HasImage() const {return (sums != NULL);};
};

class ImageWnd : public CWnd
{
public:
	enum DrawModes { DRAW, ERASE };
	enum MarkerNames { BGN, END};

	class CtrlsTab : public BarTemplate
	{		
	protected:	
		CString Name;	
	public:
		enum { IDD = IDD_DIALOGBARTAB1 };

		int stroka, Xmin, Xmax;
		CEditInterceptor XminCtrl, XmaxCtrl, strokaCtrl, AvrRangeCtrl;
		CComboBox NofScans;

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
		virtual BOOL DestroyWindow();
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnBnClickedButton5();
		afx_msg void OnEnKillfocusEdit1();
		LRESULT OnButtonIntercepted(WPARAM wParam, LPARAM lParam );
		int GetNofScans();
	};

	class PicWnd: public CWnd
	{
		friend struct AccumHelper;

		class AvaMarker
		{
		protected:
			BOOL ToErase; 
			CPoint last;

			void Draw(BMPanvas* bmp, const CPoint& pnt, DrawModes mode );
		public:
			AvaMarker() { ToErase=FALSE; }
			virtual void Draw(BMPanvas* Parent, const CPoint& pnt);
			virtual void Erase(BMPanvas * canvas);		
			//AvaMarker& operator= (const AvaPoint& pnt) { *((AvaPoint*)this) = pnt; return *this; }
		};

	protected:
		CButton CaptureButton;
		CMenu menu1; AvaMarker MarkerAvaBGN, MarkerAvaEND;
		CList<BaseForHelper*> helpers; 
				
		void UpdateHelpers(const HelperEvent &event);
	public:
		BMPanvas ava;
		ImageWnd* Parent;
		CFont font1;
		CString FileName;
		ImagesAccumulator accum;
		enum {CaptureBtnID=234234};

		PicWnd();
		virtual ~PicWnd();
		HRESULT LoadPic(CString T);
		void UpdateNow(void);
		void OnPicWndErase();
		void OnPicWndSave();
		void OnPicWndScanLine();
		void OnPicWndScanArbitaryLine();
		void OnPicWndMultiCross();
		void EraseAva();
		void SetMarker(const CPoint& mark, MarkerNames pos);
		HRESULT MakeAva();
		HRESULT ValidatePnt( CPoint& );

		DECLARE_MESSAGE_MAP()
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnPaint();
		afx_msg void OnDropFiles(HDROP hDropInfo);	
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnCaptureButton();
		LRESULT OnCaptureReady( WPARAM wParam, LPARAM lParam );
		afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
		afx_msg void OnMove(int x, int y);	
		void ConvertOrgToGrayscale();
		HRESULT TryLoadBitmap(CString T, BMPanvas &bmp);
		HRESULT ConvertAvaToOrg( CPoint& pnt ) const;
		HRESULT ConvertOrgToAva( CPoint& pny ) const;
		HRESULT ScanArbitaryLine(void * const buf, CPoint beg, CPoint end, MyTimer &Timer);
		void OnPicWndMultiCrossHelper(ScanLineData tmp_line, GaussFitFunc &GaussFit);
	};
	
	DECLARE_DYNAMIC(ImageWnd)
protected:
	PicWnd fiber;
    int scale;	CPoint MarkerBGN, MarkerEND;
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
	void SetMarker(const CPoint& mark, MarkerNames pos);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);	
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnEnUpdateEdit3();
	afx_msg void OnNMThemeChangedEdit3(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);	
};

struct AccumHelper: public BaseForHelper
{	
	int n_max;
	ImageWnd::PicWnd *parent;
	BMPanvas *tmp_bmp;

	AccumHelper(ImageWnd::PicWnd *_parent, const int _n_max);
	virtual ~AccumHelper();
	virtual HelperEvent Update(const HelperEvent &event);
};


