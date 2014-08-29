#pragma once

#include "TChartGlobal.h"
#include "TChartPoints.h"

class TAbstractElement
{
public:		
	CString Name;
	TAbstractElement* Parent;

	TAbstractElement(CString name="noname") {Name=name; Parent=0;}
	virtual BOOL PostParentMessage(UINT msg,WPARAM wParam=0, LPARAM lParam=0) 
	{
		BOOL ret=FALSE;
		if(Parent!=NULL) ret=Parent->PostParentMessage(msg,wParam,lParam);
		return ret;
	}
	virtual ~TAbstractElement() {};
};
//////////////////////////////////////////////////////////////////////////
class TAbstractGraphics: public TAbstractElement, public ColorsStyle
{
protected:
	CBrush brush;
	CPen pen;
	BYTE visible;
	ChartRender* Render;	
public:
	CRect DrawArea;
//-------------------------------------------
protected:
	virtual ChartRender* GetParentRender(int RenderID=CHARTRENDER_DEFAULT);
public:
	TAbstractGraphics(CString name="noname");
	void AssignColors(ColorsStyle &style);
	virtual void Draw(BMPanvas*);
	virtual void _Draw(BMPanvas* Parent) {};
	virtual ~TAbstractGraphics();
	virtual BYTE IsVisible() {return visible;}
	virtual void SetVisible(BYTE t) {visible=t;}
	virtual ChartRender* SetRender(int RenderID);
};
//////////////////////////////////////////////////////////////////////////
class TChartElement: public TAbstractGraphics
{	
public:		
	TChartElement(CString name="noname");
	virtual void Create(TAbstractElement *_parent,ColorsStyle &style);
	virtual ~TChartElement() {}	
	virtual BOOL PtInArea(CPoint*);
	virtual CPoint ScreenToClient(CPoint);
};

typedef CArray<TChartElement*,TChartElement*> TChartElementsArray; 
//+++++++++++++++++++++++++++++++++++

struct GridStyle
{
	BOOL					gridVisbl;  
	ColorsStyle				gridColor;
	GridStyle() { gridVisbl=FALSE;}
};

struct TicksStyle: public GridStyle
{
	ChartAxisTickStyle		tickstyle;
	BOOL					ticksVisbl;    
	ColorsStyle				ticksColor;
	TicksStyle() {tickstyle=TICK_NONE; ticksVisbl=FALSE;}			
};

struct AxisStyle: public TicksStyle
{
	ChartAxisLayoutStyle	layout;
	BOOL					axisVisbl;    
	ColorsStyle				axisColor;
	AxisStyle() {layout=AXIS_NONE; axisVisbl=FALSE;}
};

class TChartGrid: public TChartElement
{
protected:	
public:
	TChartGrid(CString name);
	virtual ~TChartGrid() {};	
	virtual void Create(TAbstractElement * p,GridStyle style);	
};
//+++++++++++++++++++++++++++++++++++

class TChartAxisMajorTicks: public TChartElement
{
	friend class TChartGrid;
	friend class TChartAxis;
protected:	
public:
	int numTicks;
	double dx; int len;	
	CArray<double> ticks;
	ChartAxisTickStyle tickStyle;	
	TChartGrid*	grid;

	TChartAxisMajorTicks(CString name);
	virtual ~TChartAxisMajorTicks() {if(grid!=NULL) {delete grid; grid=NULL;}};
	virtual void Create(TAbstractElement * p,TicksStyle style);	
	int RenderTicks();
};
//+++++++++++++++++++++++++++++++++++

class TChartAxis: public TChartElement
{
	friend class TChartGrid;
	friend class TChartAxisMajorTicks;
protected:
	TChartInterval range,max_range;
public:
	CPoint Beg, End;	
	ChartAxisLayoutStyle LayoutStyle;
	TChartAxisMajorTicks* Ticks;

	TChartAxis(CString name);
	virtual ~TChartAxis() {if(Ticks!=NULL) {delete Ticks; Ticks=NULL;}}
	virtual void Create(TAbstractElement * p,AxisStyle style);	
	virtual void _Draw(BMPanvas*);
	int RenderMajorTicks();
	virtual void SetInterval(TChartInterval _range);
	virtual void SetMinMax(TChartInterval _range);
	virtual void SetMinMax(double min, double max);
	virtual TChartInterval GetMinMax() {return max_range;};
	virtual TChartInterval GetInterval() {return range;};
	
};

class TChartAxisHorizontal: public TChartAxis
{
protected:
	class MajorTicks: public TChartAxisMajorTicks
	{
	protected:
		class Grid: public TChartGrid
		{
		public:
			Grid(CString name);
			~Grid(){};
			virtual void _Draw(BMPanvas*);
		};
	public:
		MajorTicks(CString name);
		~MajorTicks(){};
		virtual void _Draw(BMPanvas*);
		virtual void Create(TAbstractElement * p,TicksStyle style);	
	};
public:
	TChartAxisHorizontal(CString name);
	virtual ~TChartAxisHorizontal() {};
	virtual void Create(TAbstractElement * p,AxisStyle style);	
};

class TChartAxisVertical: public TChartAxis
{
protected:
	class MajorTicks: public TChartAxisMajorTicks
	{
	protected:
		class Grid: public TChartGrid
		{
		public:
			Grid(CString name);
			~Grid(){};
			virtual void _Draw(BMPanvas*);
		};
	public:
		MajorTicks(CString name);
		~MajorTicks(){};
		virtual void _Draw(BMPanvas*);
		virtual void Create(TAbstractElement * p,TicksStyle style);	
	};
public:
	TChartAxisVertical(CString name);
	virtual ~TChartAxisVertical() {};
	virtual void Create(TAbstractElement * p,AxisStyle style);	
};

//+++++++++++++++++++++++++++++++++++
class TChartFrame: public TChartElement
{
	friend class TChartGrid;
	friend class TChartAxisMajorTicks;
public:
	TChartFrame(CString);
	virtual void Create(TAbstractElement*, ColorsStyle &style);
protected:
	virtual ~TChartFrame() {};
	virtual void _Draw(BMPanvas*);
};
//+++++++++++++++++++++++++++++++++++

class TChartBackground: public TChartElement
{
public:
	TChartBackground(CString name);
	virtual void Create(TAbstractElement*, ColorsStyle &style);
protected:
	virtual ~TChartBackground() {};
	virtual void _Draw(BMPanvas*);
};
//+++++++++++++++++++++++++++++++++++

