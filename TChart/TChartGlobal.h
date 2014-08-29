#pragma  once

#include <afxtempl.h>
#include <afxmt.h>
#include <math.h>
#include "BMPanvas.h"
#include "MessageInspector.h"
#include "GlobalHeader.h"
#include "ProtectedObject.h"
#include "MyThread.h"
#include "my_gsl.h"
#include <memory.h>

#define SET(byte,mask) byte|=(mask)
#define RST(byte,mask) byte&=(~(mask))
#define GET(byte,mask) (byte & (mask))
#define MEAN_CALC 1<<0
#define SD_CALC 1<<1

enum SeriesType {SCATER_LINE=3, SCATER_ERR=5, LINE_ERR=6, SCATER_LINE_ERR=7, 
				SCATER_LINE_ERR_BCKG=15, SLEB, SLEB_MULTI=31,
				SCATER_ERR_T=37,SCATER_ERR_T2, SCATER_LINE_ERR_T=39, SCATER_PVE=64, SIMPLE_LINE,
				SIMPLE_POINT, POINTvsERROR
};

enum TChartMessages
{
	UM_SERIES_DATA_IMPORT=WM_USER + 4000, UM_CHART_BUFFER_UPDATE, UM_CHART_SHOWALL
};

#define clWHITE RGB(255,255,255)
#define clBLACK RGB(0,0,0)
#define clRED RGB(255,0,0)
#define clGREEN RGB(0,255,0)
#define clBLUE RGB(0,0,255)

enum ChartLineStyles {NO_LINE=1000, STRAIGHT};
enum ChartSymbolStyles {NO_SYMBOL=1000, CIRCLE, CROSS45, VERT_LINE};
enum ChartErrorBarStyles {NO_BARS=1000, POINTvsERROR_BAR};

enum {ID_CHART=1234567,ID_CHART_FRAME};

enum ChartAxisTickStyle {TICK_NONE,TICK_IN,TICK_OUT,TICK_INOUT};
enum ChartAxisLayoutStyle {AXIS_NONE,LEFT_AXIS,RIGHT_AXIS,TOP_AXIS,BOTTOM_AXIS};

enum ChartRendersID {CHARTRENDER_DEFAULT,SERIES_RENDER,FRAME_RENDER};

struct ColorsStyle 
{
	COLORREF BColor,PColor; 

	ColorsStyle(COLORREF bColor=clWHITE, COLORREF pColor=clBLACK) {BColor=bColor; PColor=pColor;}	
	void Serialize(CArchive& ar)
	{
		if (ar.IsStoring()) {ar << BColor << PColor;}
		else {ar >> BColor >> PColor;}
	}
};

struct TChartInterval
{
	double min,max;

	TChartInterval(){min=max=0;}
	TChartInterval(double _min,double _max) {min=_min; max=_max;}
	double Length() {return max-min;}
	void Offset(double d) {min+=d;max+=d;}
	BOOL IsNull() {return (min==max ? TRUE:FALSE);}
	BOOL operator==(TChartInterval& t) 	
	{
		return (min==t.min && max==t.max);
	}
	void Normalize() { if(min>max) {double t; t=max; max=min; min=t;} } 
};

enum {SIMPLE_POINT_MSG=0xabcdea,POINTvsERROR_MSG};

