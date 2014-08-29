#if !defined(AFX_MODIFYVALUEDIALOG_H__1C2C6341_077E_4B0E_BB58_FE4946EB93C3__INCLUDED_)
#define AFX_MODIFYVALUEDIALOG_H__1C2C6341_077E_4B0E_BB58_FE4946EB93C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModifyValueDialog.h : header file
//
/////////////////////////////////////////////////////////////////////////////
#include "resource.h"
#include "globalheader.h"
#include "externals.h"
#include "systemconfig.h"

class ModifyValueDialog : public CDialog
{
public:
	CString ValueBefore, Value, Caption;
	ModifyValueDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ModifyValueDialog)
	enum { IDD = IDD_DIALOG7 };
	double	value1;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ModifyValueDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ModifyValueDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODIFYVALUEDIALOG_H__1C2C6341_077E_4B0E_BB58_FE4946EB93C3__INCLUDED_)
