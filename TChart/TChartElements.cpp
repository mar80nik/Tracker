#include "stdafx.h"
#include "tchartelements.h"

TChartElement::TChartElement(CString name): TAbstractGraphics(name) {}

void TChartElement::Create(TAbstractElement *parent,ColorsStyle &style)
{
	Parent=parent; 
	AssignColors(style);
}

BOOL TChartElement::PtInArea( CPoint* pnt)
{
	BOOL ret;
	ret=DrawArea.PtInRect(*pnt);
	return ret;
}

CPoint TChartElement::ScreenToClient( CPoint pnt)
{
	CPoint ret;
    ret.x=pnt.x-DrawArea.left; ret.y=pnt.y-DrawArea.top; 
	return ret;
}

//+++++++++++++++++++++++++++++++++++++++++++
TChartAxis::TChartAxis(CString name): TChartElement(name) {Ticks=NULL;}

void TChartAxis::Create(TAbstractElement * parent,AxisStyle style)
{
	TChartElement::Create(parent,style.axisColor);
	TChartFrame *ChartFrame=(TChartFrame *)Parent;    
	CRect& Frame=ChartFrame->DrawArea; LayoutStyle=style.layout;
    SetRender(FRAME_RENDER);	
	switch(LayoutStyle)
	{
	case LEFT_AXIS: Beg.x=End.x=Frame.left; Beg.y=Frame.bottom; End.y=Frame.top; break;
	case RIGHT_AXIS: Beg.x=End.x=Frame.right; Beg.y=Frame.bottom; End.y=Frame.top; break;
	case TOP_AXIS: Beg.y=End.y=Frame.top; Beg.x=Frame.left; End.x=Frame.right; break;
	case BOTTOM_AXIS: Beg.y=End.y=Frame.bottom; Beg.x=Frame.left; End.x=Frame.right; break;
	}
	SetVisible(style.axisVisbl);
	
}

void TChartAxis::_Draw(BMPanvas* canvas)
{	
	canvas->MoveTo(Beg); canvas->LineTo(End);
	Ticks->Draw(canvas);
}


void TChartAxis::SetMinMax(TChartInterval _range) {max_range=_range;}

void TChartAxis::SetMinMax( double _min, double _max ) {max_range.min=_min; max_range.max=_max;}

void TChartAxis::SetInterval( TChartInterval _range )
{
	_range.Normalize();
	range=_range;
	Ticks->RenderTicks();
}

//++++++++++++++++++++++++++++++++++++++++++++++


TChartAxisMajorTicks::TChartAxisMajorTicks(CString name): TChartElement(name)
{
	numTicks=5;len=5; grid=NULL;
}


void TChartAxisMajorTicks::Create( TAbstractElement * p,TicksStyle style )
{
	TChartElement::Create(p,style.ticksColor);
	SetRender(FRAME_RENDER);	
	tickStyle=style.tickstyle;
	SetVisible(style.ticksVisbl);
}

int TChartAxisMajorTicks::RenderTicks()
{
	TChartAxis* parent=(TChartAxis*)Parent;
	double Min=parent->range.min, Max=parent->range.max;
	double X=0,dx=fabs(Max-Min)/numTicks; double p=10.,q=1.; int e;
	ticks.RemoveAll();

	if( dx > 1e-16)
	{
		e=(int)(log10(dx)); 
		if(e<=0) {e=abs(e)+1; p=0.1;}
		while((e--)>0) {dx/=p; q*=p;}
		if(((int)dx)!=0)
		{
			dx=((int)dx)*q; e=(int)(Min/dx); if(e>=0) e++;
			X=e*dx; ticks.Add(X);
			while((X+=dx)<=Max) ticks.Add(X);
		}
	}
	return ticks.GetSize();
}

//+++++++++++++++++++++++++++++++++++++++++
TChartFrame::TChartFrame(CString name): TChartElement(name) {}

void TChartFrame::Create(TAbstractElement *parent, ColorsStyle &style)
{	
	TChartElement::Create(parent,style);
	CRect temp=((TAbstractGraphics*)Parent)->DrawArea; 
	temp.left+=60; temp.top+=30; temp.right-=30; temp.bottom-=30; 
	DrawArea=temp; 
}

void TChartFrame::_Draw(BMPanvas* canvas)
{
	CRect temp=DrawArea; temp.right++; temp.bottom++;
	canvas->Rectangle(temp);	
}
//+++++++++++++++++++++++++++++++++++++++++
TChartGrid::TChartGrid(CString name) :TChartElement(name) {}

void TChartGrid::Create( TAbstractElement * p,GridStyle style )
{
	TChartElement::Create(p,style.gridColor);
	SetRender(FRAME_RENDER);	
	SetVisible(style.gridVisbl);
}

//+++++++++++++++++++++++++++++++++++++++++
TChartBackground::TChartBackground(CString name): TChartElement(name) {}

void TChartBackground::Create(TAbstractElement* parent, ColorsStyle &style)
{
	TChartElement::Create(parent,style);
	DrawArea=((TAbstractGraphics*)Parent)->DrawArea; 
}

void TChartBackground::_Draw(BMPanvas* canvas)
{
	CRect temp=DrawArea; temp.right++; temp.bottom++;
	canvas->Rectangle(temp);	
}


TAbstractGraphics::TAbstractGraphics(CString name): TAbstractElement(name)
{
	visible=false; Render=NULL;
	brush.CreateSolidBrush(clWHITE);
	pen.CreatePen(PS_SOLID,1,clBLACK);
}

TAbstractGraphics::~TAbstractGraphics() {}

void TAbstractGraphics::Draw(BMPanvas* canvas) 
{
	if(visible) 
	{
		HGDIOBJ t1,t2;
		t1=canvas->SelectObject(brush); t2=canvas->SelectObject(pen);
		_Draw(canvas);
		canvas->SelectObject(t1); canvas->SelectObject(t2);
	}
}		

void TAbstractGraphics::AssignColors(ColorsStyle &style)
{
	*((ColorsStyle*)this)=style; HGDIOBJ t; BOOL ret;
	t=brush.Detach(); ret=DeleteObject(t);
	brush.CreateSolidBrush(BColor); 
	t=pen.Detach(); ret=DeleteObject(t);
	pen.CreatePen(PS_SOLID,1,PColor);
}

ChartRender* TAbstractGraphics::SetRender(int RenderID)
{
	ChartRender* ret=NULL; 
	if(Render!=NULL) if(Render->GetUID()==RenderID) ret=Render;
	if(ret==NULL) ret=GetParentRender(RenderID);
	Render=ret;
	return ret;
}

ChartRender* TAbstractGraphics::GetParentRender( int RenderID/*=CHARTRENDER_DEFAULT*/ )
{
	ChartRender* ret=NULL; 
	TAbstractGraphics* parent=(TAbstractGraphics*)Parent;
	if(parent!=NULL) ret=parent->GetParentRender(RenderID); 
	return ret;
}

TChartAxisHorizontal::TChartAxisHorizontal( CString name ): TChartAxis(name){}

void TChartAxisHorizontal::Create( TAbstractElement * p,AxisStyle style )
{
	Ticks=new MajorTicks(Name);	
	TChartAxis::Create(p,style);
	Ticks->Create(this,style);
}

TChartAxisHorizontal::MajorTicks::MajorTicks(CString name): TChartAxisMajorTicks(name) {}

void TChartAxisHorizontal::MajorTicks::_Draw( BMPanvas* canvas)
{
	TChartAxis *Axis=(TChartAxis *)Parent; double t;
	CString ttt; int x,xText=0,y,yText=0; CSize size; CPoint cur;
    if(Render->IsInit()==FALSE) return;
	for(int i=0;i<ticks.GetSize();i++) 
	{
		t=ticks[i];
		y=Axis->Beg.y; x=Render->LogicToScreenX(ticks[i]);
		ASSERT(x>=Axis->Beg.x);
		ttt.Format("%g",t); 
		size=canvas->GetTextExtent(ttt); 

		switch (tickStyle)
		{

		case TICK_OUT: canvas->MoveTo(x,y); canvas->LineTo(x,y+len); break;		
		case TICK_IN: canvas->MoveTo(x,y-len); canvas->LineTo(x,y); break;
		case TICK_INOUT: canvas->MoveTo(x,y-len); canvas->LineTo(x,y+len); break;
		case TICK_NONE:
		default:;
		}
		cur=canvas->GetCurrentPosition(); xText=cur.x-size.cx/2; yText=cur.y;
		canvas->TextOut(xText,yText,ttt);		
	}
	grid->Draw(canvas);
}

void TChartAxisHorizontal::MajorTicks::Create( TAbstractElement * p,TicksStyle style )
{	
	grid=new Grid(Name);	
	TChartAxisMajorTicks::Create(p,style);
	grid->Create(this,style);
}

void TChartAxisVertical::MajorTicks::_Draw( BMPanvas* canvas)
{
	TChartAxis *Axis=(TChartAxis *)Parent;
	CString ttt; int x,xText=0,y,yText=0; CSize size; CPoint cur;

	for(int i=0;i<ticks.GetSize();i++) 
	{
		x=Axis->Beg.x; y=Render->LogicToScreenY(ticks[i]);
		ttt.Format("%g",ticks[i]); 
		size=canvas->GetTextExtent(ttt); yText=y-size.cy/2; 

		switch (tickStyle)
		{
		case TICK_OUT: canvas->MoveTo(x,y); canvas->LineTo(x-len,y); break;
		case TICK_IN: canvas->MoveTo(x+len,y); canvas->LineTo(x,y); break;
		case TICK_INOUT: canvas->MoveTo(x+len,y); canvas->LineTo(x-len,y); break;
		case TICK_NONE:
		default:;
		}
		cur=canvas->GetCurrentPosition(); xText=cur.x-size.cx; yText=cur.y-size.cy/2;
		canvas->TextOut(xText,yText,ttt);			
	}
	grid->Draw(canvas);
}


TChartAxisVertical::TChartAxisVertical( CString name ):TChartAxis(name){}

void TChartAxisVertical::Create( TAbstractElement * p,AxisStyle style )
{	
	Ticks=new MajorTicks(Name);	
	TChartAxis::Create(p,style);
	Ticks->Create(this,style);
}

TChartAxisVertical::MajorTicks::MajorTicks( CString name ): TChartAxisMajorTicks(name) {}

void TChartAxisVertical::MajorTicks::Create( TAbstractElement * p,TicksStyle style )
{
	grid=new Grid(Name);	
	TChartAxisMajorTicks::Create(p,style);
	grid->Create(this,style);
}


void TChartAxisHorizontal::MajorTicks::Grid::_Draw( BMPanvas* canvas)
{
	TChartAxisMajorTicks *Ticks=(TChartAxisMajorTicks *)Parent;
	TChartAxis *Axis=(TChartAxis *)(Parent->Parent);
	int x,y; CSize size=Render->GetScreenSize();

	for(int i=0;i<Ticks->ticks.GetSize();i++) 
	{
		x=Render->LogicToScreenX(Ticks->ticks[i]); y=Axis->Beg.y;
		canvas->MoveTo(x,y); canvas->LineTo(x,y-size.cy);
	}
}

TChartAxisHorizontal::MajorTicks::Grid::Grid( CString name ): TChartGrid(name) {}

void TChartAxisVertical::MajorTicks::Grid::_Draw( BMPanvas* canvas)
{
	TChartAxisMajorTicks *Ticks=(TChartAxisMajorTicks *)Parent;
	TChartAxis *Axis=(TChartAxis *)(Parent->Parent);
	int x,y; CSize size=Render->GetScreenSize();

	for(int i=0;i<Ticks->ticks.GetSize();i++) 
	{
		y=Render->LogicToScreenY(Ticks->ticks[i]); x=Axis->Beg.x;
		canvas->MoveTo(x,y); canvas->LineTo(x+size.cx,y);
	}
}

TChartAxisVertical::MajorTicks::Grid::Grid( CString name ): TChartGrid(name) {}
