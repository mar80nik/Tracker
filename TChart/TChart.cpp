// TChart.cpp : implementation file
#include "stdafx.h"
#include "TChart.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TChart

IMPLEMENT_DYNAMIC(TChart, MouseTrackerWnd)

BEGIN_MESSAGE_MAP(TChart, MouseTrackerWnd)
	//{{AFX_MSG_MAP(TChart)
	ON_WM_PAINT()
	ON_MESSAGE(UM_SERIES_DATA_IMPORT,OnSeriesDataImport)
	ON_MESSAGE(UM_CHART_BUFFER_UPDATE,OnChartBufferUpdate)
	ON_MESSAGE(UM_SERIES_UPDATE,OnSeriesUpdate)
	ON_MESSAGE(UM_CHART_SHOWALL,OnChartShowAll)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_SERIESMENU_SHOWVALUES, OnSeriesmenuShowvalues)
	//}}AFX_MSG_MAP	
	ON_WM_SIZE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// TChart message handlers
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TChart::TChart(CString name): TAbstractGraphics(name)
{	
	repaint=REPAINT_NONE; SeriesDataWnd=NULL; void *x; 
	if((x=Series.GainAcsess(WRITE))!=0)
	{
		SeriesProtector Protector(x); TSeriesArray& series(Protector);
		series.Parent=this;		
	}
	LastSeriesUpdateID=Series.GetModificationID();
	LastSeriesRenderUpdateID=0;	LeftAxis=BottomAxis=NULL; Frame=NULL;
}

TChart::~TChart()
{
	BckgBrush.DeleteObject();
	DestroyElements();
	menu1.DestroyMenu();	
	DestroyWindow(); 
}

void TChart::DestroyElements()
{
	void *x;
	if((x=buffer.GainAcsess(WRITE))!=NULL)
	{
		BMPanvasGuard guard(x); BMPanvas& bmp(guard);
		bmp.Destroy();

		for(int i=0;i<Elements.GetSize();i++) delete Elements[i];		
		Elements.RemoveAll();
        
		for(int i=0;i<CanvasErasers.GetSize();i++) delete CanvasErasers[i];	
		CanvasErasers.RemoveAll();

		LeftAxis=NULL; BottomAxis=NULL; Frame=NULL;

	}

}

BOOL TChart::Create(CWnd* pParentWnd,const RECT& rect)
{
	BOOL ret; void *x;
	CClientDC cdc(pParentWnd); 
	ret=CWnd::Create(0, Name, WS_CHILD | WS_BORDER, rect, pParentWnd, ID_CHART, 0);	

	font1.CreatePointFont(60,"MS Sans Serif");			
	font2.CreatePointFont(100,"Arial"); 

	if((x=Series.GainAcsess(WRITE))!=NULL)
	{
		SeriesProtector guard(x); TSeriesArray& series(guard);
		series.Parent=this;
		series.Parent=this;
		series.SetRender(SERIES_RENDER);
	}
	SetFont(&font1);
	menu1.LoadMenu(IDR_MENU2);
    OnPaintTimer.Start();	

	pThrd=AfxGetThread();	pWND=this;
	BckgBrush.CreateSolidBrush(RGB(140,140,140));
	InitBasicElements();
	OnSeriesUpdate(0,0);
	return ret;
}


void TChart::Clear()
{
	void *x;
	if((x=buffer.GainAcsess(WRITE))!=NULL)
	{
		BMPanvasGuard guard(x); BMPanvas& bmp(guard);
		bmp.PatBlt(WHITENESS);
	}	
	sbuf.PatBlt(WHITENESS);
}

void TChart::FindMinMax()
{
	int i,n=-1; void *x;
	if(SeriesIsChanged())
	{
		if((x=Series.GainAcsess(READ))!=0)
		{
			SeriesProtector Protector(x); TSeriesArray& series(Protector);
			MinMax=PointsRgn();
			for(i=0;i<series.GetSize() && n<0;i++) 
			{
				if(series[i]->FindMinMax()) n=i;
			}
			if(n>=0) MinMax=series[n]->MinMax;

			for(;i<series.GetSize();i++) 
			{
				if(series[i]->FindMinMax()) MinMax+=series[i]->MinMax;
			}					
		}
		if(LeftAxis!=NULL) LeftAxis->SetMinMax(MinMax.minY,MinMax.maxY);
		if(BottomAxis!=NULL) BottomAxis->SetMinMax(MinMax.minX,MinMax.maxX);		
	}
}

void TChart::CalcOriginScale()
{
	FindMinMax();
	
	if(BottomAxis!=NULL)
		if(BottomAxis->GetMinMax().IsNull()) 
			BottomAxis->SetInterval(BottomAxis->GetMinMax());	
	if(BottomAxis!=NULL)
		if(BottomAxis->GetInterval().IsNull()) 
			BottomAxis->SetInterval(BottomAxis->GetMinMax());
		
	if(LeftAxis!=NULL)
		if(LeftAxis->GetMinMax().IsNull()) 
			LeftAxis->SetInterval(LeftAxis->GetMinMax());
	if(LeftAxis!=NULL)
		if(LeftAxis->GetInterval().IsNull()) 
			LeftAxis->SetInterval(LeftAxis->GetMinMax());

	if(BottomAxis!=NULL && LeftAxis!=NULL)
	{
		TChartInterval xx=BottomAxis->GetInterval(); 
		TChartInterval yy=LeftAxis->GetInterval(); 
		CRect area;
		area=Frame->DrawArea; FrameRender.Set(xx,yy,area);
		area.OffsetRect(-area.TopLeft()); SeriesRender.Set(xx,yy,area); 
	}

}


void TChart::UpdateNow(RepaintSource src) 
{
	repaint=src; 
	RedrawWindow(0,0,RDW_INVALIDATE | RDW_NOERASE | RDW_NOFRAME | RDW_ALLCHILDREN);
}

void TChart::InitBasicElements()
{
	ColorsStyle tempStyle; TChartElement *elmnt=NULL; TChartAxis *taxis=NULL; AxisStyle style; 

	DestroyElements();
	
	void *x;
	if((x=buffer.GainAcsess(WRITE))!=NULL)
	{
		GetClientRect(&DrawArea); 
		BMPanvasGuard guard(x); BMPanvas& bmp(guard);
		bmp.Create(this,DrawArea.Width(),DrawArea.Height(),24);
				
		bmp.SetBkMode(TRANSPARENT);

		elmnt=new TChartBackground("Background"); elmnt->Create(this,ColorsStyle(RGB(140,140,140))); 	
		elmnt->SetVisible(true);RegisterChartElement(elmnt);
		Frame=new TChartFrame("Frame"); Frame->Create(this,ColorsStyle(RGB(80,80,80))); Frame->SetVisible(true);	
		RegisterChartElement(Frame);	

		style.axisVisbl=style.ticksVisbl=style.gridVisbl=TRUE;
		style.layout=LEFT_AXIS; style.tickstyle=TICK_OUT; style.gridColor.PColor=RGB(140,140,140);
		taxis=new TChartAxisVertical("LeftAxis"); taxis->Create(Frame,style); LeftAxis=taxis;
		RegisterChartElement(taxis);
		style.layout=BOTTOM_AXIS; 	
		taxis=new TChartAxisHorizontal("BottomAxis"); taxis->Create(Frame,style); BottomAxis=taxis;
		RegisterChartElement(taxis);	
		style.layout=RIGHT_AXIS; 
		style.ticksVisbl=style.gridVisbl=FALSE; taxis=new TChartAxisVertical("RightAxis"); taxis->Create(Frame,style); 
		RegisterChartElement(taxis);
		style.layout=TOP_AXIS;
		taxis=new TChartAxisHorizontal("TopAxis"); taxis->Create(Frame,style); 
		RegisterChartElement(taxis);
		if(LeftAxis!=NULL) LeftAxis->SetMinMax(MinMax.minY,MinMax.maxY);
		if(BottomAxis!=NULL) BottomAxis->SetMinMax(MinMax.minX,MinMax.maxX);

		sbuf.Create(this,Frame->DrawArea.Width(),Frame->DrawArea.Height(),24);

		AreaOfInterest tempArea;
		tempArea=Frame->DrawArea; tempArea.Proceed=&TChart::OnPlotDblClick; Clickers.Add(tempArea);
		tempArea=CRect(LeftAxis->End,LeftAxis->Beg); tempArea.left-=20; 		
		tempArea.Proceed=&TChart::OnAxisDblClick; Clickers.Add(tempArea);

		UpdateNow(REPAINT_DEFAULT);	
	}	
}


void TChart::OnPaint()
{
	CPaintDC canvas(this); void *x; CString T;
	 ms dt;
	
	if(visible)
	{			
		if((x=buffer.GainAcsess(WRITE))!=NULL)
		{			
			HDC hdc=canvas.GetSafeHdc();
			BMPanvasGuard guard(x); BMPanvas& bmp(guard);	
	
			if(bmp.HasImage())
			{
	
				if(repaint==REPAINT_DEFAULT) 
				{
					DefaultBufferRender(&bmp);
				}

		
	
				InfoOnCanvas.dt1=OnPaintTimer.StopStart();

				HGDIOBJ tf=bmp.SelectObject(font2); bmp.SetTextColor(clWHITE); bmp.SetBkMode(TRANSPARENT);

				if(CanvasErasers.GetSize()!=0)
				{
					for(int i=0;i<CanvasErasers.GetSize();i++) 	delete CanvasErasers[i];	
					CanvasErasers.RemoveAll();
				}
				TextCanvasErase *t;
				T.Format("%.2g ms",InfoOnCanvas.dt1.val()); CSize size=bmp.GetTextExtent(T); 
				t=new TextCanvasErase(&bmp,CPoint(bmp.w-size.cx,0),T); CanvasErasers.Add(t);
				T.Format("Render  %.2g ms",InfoOnCanvas.dt4.val());  
				t=new TextCanvasErase(&bmp,CPoint(0,0),T); CanvasErasers.Add(t);
				T.Format("X=%.6g",InfoOnCanvas.DataInfoPoint.x);  
				t=new TextCanvasErase(&bmp,Frame->DrawArea.TopLeft(),T); CanvasErasers.Add(t);
				T.Format("Y=%.3g",InfoOnCanvas.DataInfoPoint.y);  
				t=new TextCanvasErase(&bmp,Frame->DrawArea.TopLeft()+CPoint(0,15),T); CanvasErasers.Add(t);

				bmp.SelectObject(tf);	

				bmp.CopyTo(hdc,TOP_LEFT);
			}					
		}
	}	
	repaint=REPAINT_NONE;	
}


TextCanvasErase::TextCanvasErase(BMPanvas *master_buf,CPoint lt,CString T)
{
	master=master_buf;
	CSize size=master->GetTextExtent(T);	
	buf.Create(master,CRect(lt,size));
	master->CopyTo(&buf,CPoint(0,0),buf.Rgn);
	master->TextOut(lt.x,lt.y,T);
}

TextCanvasErase::~TextCanvasErase()
{
    if(buf.HasImage())
	{
		if(master!=NULL)
		{
			buf.CopyTo(master,buf.Rgn.TopLeft());
		}
	}
}


void TChart::DefaultBufferRender(BMPanvas* canvas)
{
	int i; void *x; //CString T; 
	BMPanvas &bmp=*canvas; 

	CalcOriginScale(); 	

	if(SeriesRenderIsChanged()==FALSE && SeriesIsChanged()==FALSE) return;
	LastSeriesRenderUpdateID=SeriesRender.GetModificationID();
	LastSeriesUpdateID=Series.GetModificationID();
	if((x=Series.GainAcsess(READ))!=0)
	{
		Timer1.Start(); 
		SeriesProtector Protector(x); TSeriesArray& series(Protector);
		Clear();	
		InfoOnCanvas.dt2=Timer1.StopStart();
		for(i=0;i<Elements.GetSize();i++) 
			Elements[i]->Draw(&bmp);									
		InfoOnCanvas.dt3=Timer1.StopStart();
		bmp.CopyTo(&sbuf,TOP_LEFT,Frame->DrawArea);
		for(i=0;i<series.GetSize();i++) series[i]->Draw(&sbuf);			
		sbuf.CopyTo(&bmp,Frame->DrawArea.TopLeft()); 
		
		
		InfoOnCanvas.dt4=Timer1.StopStart();
	}
	else
		ASSERT(0);

}

//////////////////////////////////////////////////////////////////////////
BOOL TChart::BeginZOOMIN( CPoint point ) {return BeginZOOMOUT(point);}
BOOL TChart::EndZOOMIN( CPoint point )
{
	SimplePoint pntL; LogicRect zoomed;
	if(SeriesRender.IsInit()==FALSE) return FALSE;
	if(Frame->PtInArea(&point)==FALSE) return FALSE;

	pntL=SeriesRender.ScreenToLogic(Frame->ScreenToClient(point));
	zoomed=SeriesRender.ZoomIN(pntL,2);	
	LeftAxis->SetInterval(TChartInterval(zoomed.b,zoomed.t));
	BottomAxis->SetInterval(TChartInterval(zoomed.l,zoomed.r));
	UpdateNow(REPAINT_DEFAULT); 

	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
BOOL TChart::BeginZOOMOUT( CPoint point )
{
	if(SeriesRender.IsInit()==FALSE) return FALSE;
	if(Frame->PtInArea(&point)==FALSE) return FALSE;
    return TRUE;
}
BOOL TChart::EndZOOMOUT( CPoint point )
{
	LogicRect zoomed; SimplePoint pntL; 
	if(SeriesRender.IsInit()==FALSE) return FALSE;

	pntL=SeriesRender.ScreenToLogic(Frame->ScreenToClient(point));
	zoomed=SeriesRender.ZoomOUT(pntL,2);
	LeftAxis->SetInterval(TChartInterval(zoomed.b,zoomed.t));
	BottomAxis->SetInterval(TChartInterval(zoomed.l,zoomed.r));
	UpdateNow(REPAINT_DEFAULT);		

	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
BOOL TChart::BeginAREAZOOM( CPoint point )
{
	CPoint dd;
	if(SeriesRender.IsInit()==FALSE) return FALSE;
	dd=Last.Left.BtnDown-Last.Mouse.Move;	
	if(abs(dd.x)<5 || abs(dd.y)<5) return FALSE;
	ZoomAreaRect.TopLeft=Last.Left.BtnDown;
	return TRUE;
}
BOOL TChart::OnAREAZOOM( CPoint point )
{
	void *x;
	if((x=buffer.GainAcsess(WRITE))==NULL) return FALSE;	
	BMPanvasGuard guard(x); BMPanvas& bmp(guard);
	ZoomAreaRect.BottomRight=Last.Mouse.Move; ZoomAreaRect.Draw(&bmp);
	UpdateNow(REPAINT_CUSTOM);
	return TRUE;	
}
BOOL TChart::EndAREAZOOM( CPoint point )
{
	SimplePoint pntF,pntL; void *x;
	if(SeriesRender.IsInit()==FALSE) return FALSE;
	if((x=buffer.GainAcsess(WRITE))==NULL) return FALSE;

	{ BMPanvasGuard guard(x); BMPanvas& bmp(guard); ZoomAreaRect.Erase(&bmp); }    
	CRect zoom_area(Frame->ScreenToClient(Last.Left.BtnDown),Frame->ScreenToClient(point)); 
	zoom_area.NormalizeRect();
	pntF=SeriesRender.ScreenToLogic(zoom_area.TopLeft()); 
	pntL=SeriesRender.ScreenToLogic(zoom_area.BottomRight());
	LeftAxis->SetInterval(TChartInterval(pntL.y,pntF.y));
	BottomAxis->SetInterval(TChartInterval(pntF.x,pntL.x));
	UpdateNow(REPAINT_DEFAULT);		
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
BOOL TChart::BeginMOVE( CPoint point )
{
	CPoint dd;
	if(SeriesRender.IsInit()==FALSE) return FALSE;
	dd=Last.Left.BtnDown-Last.Mouse.Move;	
	if(abs(dd.x)<5 || abs(dd.y)<5) return FALSE;
	LastMouseMove=Last.Right.BtnDown;
	return TRUE;
}
BOOL TChart::OnMOVE( CPoint point )
{
	CPoint dd; TChartInterval intrvl;	SimplePoint pntF,pntL,shift; 

	dd=Last.Mouse.Move-LastMouseMove;	
	if(abs(dd.x)<20 && abs(dd.y)<30) return TRUE;

	pntF=SeriesRender.ScreenToLogic(LastMouseMove); 
	pntL=SeriesRender.ScreenToLogic(Last.Mouse.Move);
	shift=pntL-pntF;
	intrvl=LeftAxis->GetInterval(); intrvl.Offset(-shift.y); LeftAxis->SetInterval(intrvl);
	intrvl=BottomAxis->GetInterval(); intrvl.Offset(-shift.x); BottomAxis->SetInterval(intrvl);
	UpdateNow(REPAINT_DEFAULT);	
	LastMouseMove=Last.Mouse.Move;
	return TRUE;
}
BOOL TChart::EndMOVE( CPoint point ) {return OnMOVE(point);}
//////////////////////////////////////////////////////////////////////////
void TChart::OnAxisDblClick( CPoint pnt )
{
	TChartInterval t=LeftAxis->GetInterval();
	SetRangeDlg.MinY=t.min; SetRangeDlg.MaxY=t.max;
	if(SetRangeDlg.DoModal()==IDOK)
	{
		t.min=SetRangeDlg.MinY; t.max=SetRangeDlg.MaxY;
		LeftAxis->SetInterval(t);
		PostMessage(REPAINT_DEFAULT);
	}	
}

void TChart::OnPlotDblClick( CPoint pnt )
{
	PostMessage(UM_CHART_SHOWALL);
}


BOOL TChart::OnZOOMALL( CPoint point )
{
	for(int i=0;i<Clickers.GetSize();i++) 
		if(Clickers[i].PtInRect(point)) 
			(this->*Clickers[i].Proceed)(point);	
	return true;
}

BOOL TChart::OnIDLEMOVE( CPoint point )
{
	InfoOnCanvas.DataInfoPoint=FrameRender.ScreenToLogic(point);
	UpdateNow(REPAINT_DEFAULT);
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
int TChart::RegisterChartElement( TChartElement* elmnt)
{
	return Elements.Add(elmnt);
}

BOOL TChart::PostParentMessage( UINT msg,WPARAM wParam, LPARAM lParam )
{
	if(m_hWnd!=NULL) return CWnd::PostMessage(msg,wParam, lParam);
	else return FALSE;
}

ChartRender* TChart::GetParentRender( int RenderID/*=0*/ )
{
	switch (RenderID)
	{
	case FRAME_RENDER: return &FrameRender;
	case SERIES_RENDER: return &SeriesRender;
	}
	ASSERT(0);
	return NULL;
}

ChartRender* TChart::SetRender( int RenderID/*=CHARTRENDER_DEFAULT*/ )
{
    return NULL;
}

void TChart::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CMenu* menu = menu1.GetSubMenu(0);
	menu->TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
}

void TChart::OnSeriesmenuShowvalues()
{	
	if(SeriesDataWnd!=NULL)
	{
		SeriesDataWnd->PostMessage(UM_SERIES_UPDATE,SW_SHOW,(LPARAM)&Series);
	}
}

void TChartZoomAreaRect::_Draw( BMPanvas* canvas )
{
	HGDIOBJ t1,t2; CBrush brsh;
	t2=GetStockObject(HOLLOW_BRUSH);
	t1=canvas->SelectObject(t2); canvas->SetROP2(R2_NOT);
	if(ToErase==TRUE) {canvas->Rectangle(TopLeft,LastMouseMoveDraw); ToErase=FALSE;}
	canvas->Rectangle(TopLeft,BottomRight); ToErase=TRUE;
	LastMouseMoveDraw=BottomRight; 
	canvas->SelectObject(t1); canvas->SetROP2(R2_COPYPEN); 		
	DeleteObject(t2);
}

void TChartZoomAreaRect::Erase( BMPanvas* canvas )
{
	HGDIOBJ t1,t2; CBrush brsh;
	t2=GetStockObject(HOLLOW_BRUSH);
	t1=canvas->SelectObject(t2); canvas->SetROP2(R2_NOT);
	if(ToErase==TRUE) {canvas->Rectangle(TopLeft,BottomRight); ToErase=FALSE;}
	canvas->SelectObject(t1); canvas->SetROP2(R2_COPYPEN); 		
	DeleteObject(t2);
}


BOOL TChart::SeriesIsChanged()
{
	return (LastSeriesUpdateID!=Series.GetModificationID() ? TRUE:FALSE);
}

LRESULT TChart::OnSeriesDataImport( WPARAM wParam, LPARAM lParam )
{
	MessagesInspectorSubject* msg=(MessagesInspectorSubject*)lParam; void *x;
	BYTE t=0;

	if(msg)
	{		
		if((x=Series.GainAcsess(WRITE))!=0)
		{
			SeriesProtector Protector(x); TSeriesArray& Series(Protector);
			for(int i=0;i<Series.GetSize();i++)
			{
				if(Series[i]->ImportData(msg,DO_NOT_DELETE_MSG)!=0) t++;				
			}
		}
		delete msg;
		if(t) UpdateNow(REPAINT_DEFAULT);
	}
	return 0;
}

LRESULT TChart::OnChartBufferUpdate( WPARAM wParam, LPARAM lParam )
{
	UpdateNow(REPAINT_DEFAULT);
	return 0;
}

LRESULT TChart::OnSeriesUpdate( WPARAM wParam, LPARAM lParam )
{
    if(SeriesDataWnd!=NULL) SeriesDataWnd->PostMessage(UM_SERIES_UPDATE,0,(LPARAM)&Series);		
	CWnd* mainfrm=AfxGetMainWnd();
	if(mainfrm!=NULL) mainfrm->PostMessage(UM_SERIES_UPDATE,0,(LPARAM)&Series);		
	UpdateNow(REPAINT_DEFAULT);
	return 0;
}

LRESULT TChart::OnChartShowAll( WPARAM wParam, LPARAM lParam )
{
	FindMinMax();
	BottomAxis->SetInterval(BottomAxis->GetMinMax());
	LeftAxis->SetInterval(LeftAxis->GetMinMax());
	UpdateNow(REPAINT_DEFAULT);
    return 0;
}


BOOL TChart::SeriesRenderIsChanged()
{
	return (LastSeriesRenderUpdateID!=SeriesRender.GetModificationID() ? TRUE:FALSE);
}

void TChart::OnSize(UINT nType, int cx, int cy)
{
	CRect cr; GetClientRect(&cr); 
	if(cr.Width()!=cx || cr.Height()!=cy) 
	{
		InitBasicElements();
	}
	__super::OnSize(nType, cx, cy);
}

void TChart::OnDestroy()
{
	font1.DeleteObject();
	font2.DeleteObject();
	DestroyElements();
	__super::OnDestroy();

}

