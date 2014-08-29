#if !defined(AFX_MYTOOLBAR_H__FCB1F1FD_0648_44A6_863E_3FD70FF6A7FD__INCLUDED_)
#define AFX_MYTOOLBAR_H__FCB1F1FD_0648_44A6_863E_3FD70FF6A7FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MyToolBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// MyToolBar window

class MyToolBar : public CToolBar
{
// Construction
public:
	MyToolBar();

// Attributes
public:
	enum _refresh {Refresh, NoRefresh};
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MyToolBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	int AddButton(UINT buttonID,_refresh r=NoRefresh);
	virtual ~MyToolBar();

	// Generated message map functions
protected:
	CUIntArray buttons;
	//{{AFX_MSG(MyToolBar)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYTOOLBAR_H__FCB1F1FD_0648_44A6_863E_3FD70FF6A7FD__INCLUDED_)
