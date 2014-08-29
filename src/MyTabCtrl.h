#if !defined(AFX_MYTABCTRL_H__1A2BF618_BA52_45D3_92D8_065A01F04895__INCLUDED_)
#define AFX_MYTABCTRL_H__1A2BF618_BA52_45D3_92D8_065A01F04895__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MyTabCtrl.h : header file
//
#include "globalheader.h"

/////////////////////////////////////////////////////////////////////////////
// MyTabCtrl window

class MyTabCtrl 
{
// Construction
public:
	MyTabCtrl(CWnd *Parent, int selfID);

// Attributes
protected:
	int selfID;
	CWnd *Parent;
	CTabCtrl *TabCtrl;
	CArray<CDialog*,CDialog*> Tabs;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MyTabCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	void ChangeTab(int num=-1);
	int curTab;
	TCITEM item;
	void AddTab(CString name,CDialog *, int idd);
	virtual ~MyTabCtrl();
	int SetCurSel(int nItem);
	int FindTab(CString name);
	// Generated message map functions

	
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYTABCTRL_H__1A2BF618_BA52_45D3_92D8_065A01F04895__INCLUDED_)
