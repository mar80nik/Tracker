#pragma once

#include "mylistbox.h"
#include "resource.h"
#include "globalheader.h"
#include "MyThread.h"
/////////////////////////////////////////////////////////////////////////////
// EventLogDialog dialog

class EventLogDialog : public CDialog
{
// Construction
public:
	EventLogDialog(CWnd* pParent = NULL);   // standard constructor
	void Add(LogMessage&);

// Dialog Data
	//{{AFX_DATA(EventLogDialog)
	enum { IDD = IDD_DIALOG5 };
protected:
	MyListBox	m_ListBox1;
	CMenu menu1;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EventLogDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(EventLogDialog)
	afx_msg void OnClearBtn();
	afx_msg void OnTestButton();
	afx_msg void OnSaveButton();
	afx_msg LRESULT OnEventLog(WPARAM wParam, LPARAM lParam );	
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
};
