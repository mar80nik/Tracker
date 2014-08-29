// MouseTrackerWnd.cpp : implementation file
//

#include "stdafx.h"
#include "MouseTrackerWnd.h"


// MouseTrackerWnd

IMPLEMENT_DYNAMIC(MouseTrackerWnd, CWnd)
MouseTrackerWnd::MouseTrackerWnd()
{
}

MouseTrackerWnd::~MouseTrackerWnd()
{
}


BEGIN_MESSAGE_MAP(MouseTrackerWnd, CWnd)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()	
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()
//////////////////////////////////////////////////////////////////////////
void MouseTrackerWnd::OnLButtonDown( UINT nFlags, CPoint point )
{
	Last.Left.BtnDown=point;

	switch(nFlags)
	{
	case (MK_LBUTTON|MK_SHIFT|MK_CONTROL):	
		if(BeginZOOMIN(point)==TRUE) mouse_mode=TRACK_ZOOMIN;
		break;		
	}    	

	__super::OnLButtonDown(nFlags, point);
}
//////////////////////////////////////////////////////////////////////////
void MouseTrackerWnd::OnLButtonUp( UINT nFlags, CPoint point )
{
	Last.Left.BtnUp=point;

	switch (mouse_mode)
	{
	case TRACK_AREAZOOM: 
		if(EndAREAZOOM(point)==TRUE) mouse_mode=TRACK_IDLE; 
		break;	
	case TRACK_ZOOMIN:
		if(EndZOOMIN(point)==TRUE) mouse_mode=TRACK_IDLE; 
		break;	
	}
	__super::OnLButtonUp(nFlags, point);
}
//////////////////////////////////////////////////////////////////////////
void MouseTrackerWnd::OnLButtonDblClk( UINT nFlags, CPoint point )
{
	OnZOOMALL(point);	
	CWnd::OnLButtonDblClk(nFlags, point);
}
//////////////////////////////////////////////////////////////////////////
void MouseTrackerWnd::OnRButtonDown( UINT nFlags, CPoint point )
{
	Last.Right.BtnDown=point;

	switch(nFlags)
	{
	case (MK_RBUTTON|MK_SHIFT|MK_CONTROL):
		if(BeginZOOMOUT(point)==TRUE) mouse_mode=TRACK_ZOOMOUT;
		break;	
	}  
	__super::OnRButtonDown(nFlags, point);
}
//////////////////////////////////////////////////////////////////////////
void MouseTrackerWnd::OnRButtonUp( UINT nFlags, CPoint point )
{
	Last.Right.BtnUp=point;

	switch (mouse_mode)
	{
	case TRACK_ZOOMOUT:	
		if(EndZOOMOUT(point)==TRUE) mouse_mode=TRACK_IDLE; 
		break;
	case TRACK_MOVE: 
		if(EndMOVE(point)==TRUE) mouse_mode=TRACK_IDLE; 
		break;
	default: __super::OnRButtonUp(nFlags, point);
	}

}
//////////////////////////////////////////////////////////////////////////
void MouseTrackerWnd::OnMouseMove( UINT nFlags, CPoint point )
{
	Last.Mouse.Move=point;

	switch(mouse_mode)
	{
	case TRACK_AREAZOOM:
		if(OnAREAZOOM(point)==FALSE) mouse_mode=TRACK_IDLE;
		break;
	case TRACK_MOVE:
		if(OnMOVE(point)==FALSE) mouse_mode=TRACK_IDLE;
		break;
	default:
		switch(nFlags)
		{
		case MK_LBUTTON: 
			if(BeginAREAZOOM(point)==TRUE) mouse_mode=TRACK_AREAZOOM; 
			break;
		case (MK_RBUTTON|MK_SHIFT|MK_CONTROL):
			if (BeginMOVE(point)==TRUE) mouse_mode=TRACK_MOVE; 
			break;
		default: mouse_mode=TRACK_IDLE;
		}   
	}
	if(mouse_mode==TRACK_IDLE)
	{
		OnIDLEMOVE(point);
	}
	__super::OnMouseMove(nFlags, point);
}

BOOL MouseTrackerWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{	
	switch (mouse_mode)
	{
	case TRACK_MOVE: ::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEALL)); return TRUE;		
	case TRACK_IDLE: ::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS)); return TRUE;		
	}

	return __super::OnSetCursor(pWnd, nHitTest, message);
}
