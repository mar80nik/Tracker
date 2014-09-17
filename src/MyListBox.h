#if !defined(AFX_MYLISTBOX_H__6E4C072E_73DB_4FAE_B54A_B6DC0D3A2A37__INCLUDED_)
#define AFX_MYLISTBOX_H__6E4C072E_73DB_4FAE_B54A_B6DC0D3A2A37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MyListBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// MyListBox window

class MyListBox : public CListBox
{
// Construction
public:
	MyListBox();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MyListBox)
	//}}AFX_VIRTUAL

// Implementation
public:
	int width;

	virtual ~MyListBox();
	int AddString(LPCTSTR s, COLORREF col=0);

	// Generated message map functions
protected:
	void SetNewHExtent(LPCTSTR lpszNewString);
	int GetTextLen(LPCTSTR lpszText);
	void ResetHExtent();
	//{{AFX_MSG(MyListBox)
	LRESULT OnAddString(WPARAM wParam, LPARAM lParam);
	virtual void DrawItem(LPDRAWITEMSTRUCT /*lpDrawItemStruct*/);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT /*lpMeasureItemStruct*/);
	LRESULT OnResetContent(WPARAM wParam, LPARAM lParam);
	LRESULT OnDir(WPARAM wParam, LPARAM lParam);
	LRESULT OnDeleteString(WPARAM wParam, LPARAM lParam);
	LRESULT OnInsertString(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
public:

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	BOOL OnInit();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYLISTBOX_H__6E4C072E_73DB_4FAE_B54A_B6DC0D3A2A37__INCLUDED_)
