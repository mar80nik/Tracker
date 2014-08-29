#pragma once


class MouseTrackerWnd : public CWnd
{
	DECLARE_DYNAMIC(MouseTrackerWnd)

public:
	MouseTrackerWnd();
	virtual ~MouseTrackerWnd();

protected:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_MESSAGE_MAP()
protected:
	enum MouseTracker {TRACK_IDLE,TRACK_AREAZOOM,TRACK_ZOOMIN,TRACK_ZOOMOUT, TRACK_MOVE} mouse_mode;

	struct MouseState
	{
		struct Button
		{
			CPoint BtnDown, BtnUp, DblClick, Move;
		} Left,Right,Middle,Mouse;		
	} Last,Current;
// Overrides
	virtual BOOL BeginZOOMIN(CPoint point) {return FALSE;};
	virtual BOOL		EndZOOMIN(CPoint point) {return TRUE;};
	virtual BOOL BeginZOOMOUT(CPoint point) {return FALSE;};
	virtual BOOL		EndZOOMOUT(CPoint point) {return TRUE;};
	virtual BOOL BeginAREAZOOM(CPoint point) {return FALSE;};
	virtual BOOL	OnAREAZOOM(CPoint point) {return FALSE;};
	virtual BOOL		EndAREAZOOM(CPoint point) {return TRUE;};
	virtual BOOL BeginMOVE(CPoint point) {return FALSE;};
	virtual BOOL	OnMOVE(CPoint point) {return FALSE;};
	virtual BOOL		EndMOVE(CPoint point) {return TRUE;};
	virtual BOOL OnZOOMALL(CPoint point) {return FALSE;};	
	virtual BOOL OnIDLEMOVE(CPoint point) {return FALSE;};	
};


