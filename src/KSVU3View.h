// KSVU3View.h : interface of the CKSVU3View class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_KSVU3VIEW_H__83291C22_1644_4ED8_BFFD_3F6EB69B4A81__INCLUDED_)
#define AFX_KSVU3VIEW_H__83291C22_1644_4ED8_BFFD_3F6EB69B4A81__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CKSVU3View : public CScrollView
{
protected: // create from serialization only
	CKSVU3View();
	DECLARE_DYNCREATE(CKSVU3View)

// Attributes
public:
	CKSVU3Doc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKSVU3View)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CKSVU3View();

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CKSVU3View)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnInitialUpdate();
};

#ifndef _DEBUG  // debug version in KSVU3View.cpp
inline CKSVU3Doc* CKSVU3View::GetDocument()
   { return (CKSVU3Doc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KSVU3VIEW_H__83291C22_1644_4ED8_BFFD_3F6EB69B4A81__INCLUDED_)
