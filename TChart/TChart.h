#pragma once

#include "tchartglobal.h"
#include "TChartPoints.h"
#include "tchartelements.h"
#include "tchartseries.h"
#include "MouseTrackerWnd.h"
#include "SetAxisRangeDlg.h"

/////////////////////////////////////////////////////////////////////////////
// TChart window
enum RepaintSource {REPAINT_NONE, REPAINT_DEFAULT, REPAINT_CUSTOM};

class TChartZoomAreaRect: public TAbstractGraphics
{
protected:
	BOOL ToErase;
	CPoint LastMouseMoveDraw;
public:
	CPoint TopLeft, BottomRight;

	virtual void _Draw(BMPanvas* Parent);
	virtual void Erase(BMPanvas* Parent);
public:
	TChartZoomAreaRect() {SetVisible(TRUE); ToErase=FALSE;}
};

struct PerfomanceInfo 
{
	ms dt1,dt2,dt3,dt4;
	SimplePoint DataInfoPoint;
};

struct TextCanvasErase
{
protected:
	CRect area;
	BMPanvas buf,*master;
public:
    TextCanvasErase(BMPanvas *master_buf,CPoint lt,CString T);
	~TextCanvasErase();
};

class TChart;
struct AreaOfInterest: public CRect
{
	void (TChart::*Proceed)(CPoint pnt);
	AreaOfInterest() {Proceed=NULL;}
	AreaOfInterest& operator=(CRect& t) { *((CRect*)this)=t; return (*this); }
};


class TChart : public MouseTrackerWnd, public TAbstractGraphics, public PerfomanceStaff, public WindowAddress
{
	DECLARE_DYNAMIC(TChart)
protected:
	CFont font1,font2;		
	TChartElementsArray Elements;
	TChartFrame *Frame; 
	TChartAxis *LeftAxis, *BottomAxis;	
	CMenu menu1; MyTimer OnPaintTimer;
	CPoint LastMouseMove; 
	int LastSeriesUpdateID, LastSeriesRenderUpdateID;
	TChartZoomAreaRect ZoomAreaRect;
	PerfomanceInfo InfoOnCanvas;
	CArray<TextCanvasErase*> CanvasErasers;
	CArray<AreaOfInterest> Clickers; 
	CBrush BckgBrush;
	PointsRgn MinMax;
public:	
	SetAxisRangeDlg SetRangeDlg;
	ProtectedBMPanvas buffer;
	ProtectedSeriesArray Series;
	CWnd* SeriesDataWnd;
	ChartRender SeriesRender, FrameRender;
	BMPanvas sbuf,fbuf;
	RepaintSource repaint;
//-------------------------------------------
protected:	
	void InitBasicElements();	
	void CalcOriginScale();
	int RegisterChartElement(TChartElement*);
	virtual ChartRender* GetParentRender(int RenderID=CHARTRENDER_DEFAULT);
	void DefaultBufferRender(BMPanvas *);
	BOOL SeriesIsChanged();
	BOOL SeriesRenderIsChanged();
	void FindMinMax();

	void OnAxisDblClick(CPoint pnt);
	void OnPlotDblClick(CPoint pnt);
public:	
	TChart(CString name);
	virtual ~TChart();
	BOOL Create(CWnd* pParentWnd,const RECT& rect);
	void UpdateNow(RepaintSource src);
	void Clear(); 
	virtual BOOL PostParentMessage(UINT msg,WPARAM wParam, LPARAM lParam);
	virtual ChartRender* SetRender(int RenderID=CHARTRENDER_DEFAULT);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TChart)
	//}}AFX_VIRTUAL

// Implementation

	// Generated message map functions
protected:
	//{{AFX_MSG(TChart)
	afx_msg void OnPaint();	
	afx_msg LRESULT OnDataUpdate(WPARAM wParam, LPARAM lParam );	
	afx_msg LRESULT OnSeriesDataImport(WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnChartBufferUpdate( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSeriesUpdate( WPARAM wParam, LPARAM lParam );
	LRESULT OnChartShowAll( WPARAM wParam, LPARAM lParam );
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual BOOL BeginZOOMIN(CPoint point);
	virtual BOOL		EndZOOMIN(CPoint point);
	virtual BOOL BeginZOOMOUT(CPoint point);
	virtual BOOL		EndZOOMOUT(CPoint point);
	virtual BOOL BeginAREAZOOM(CPoint point);
	virtual BOOL	OnAREAZOOM(CPoint point);
	virtual BOOL		EndAREAZOOM(CPoint point);
	virtual BOOL BeginMOVE(CPoint point);
	virtual BOOL	OnMOVE(CPoint point);
	virtual BOOL		EndMOVE(CPoint point);
	virtual BOOL OnZOOMALL(CPoint point);	
	virtual BOOL OnIDLEMOVE(CPoint point);	
public:
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnSeriesmenuShowvalues();
	
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	void DestroyElements();
};


