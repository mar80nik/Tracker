#if !defined(AFX_DIALOGBARTAB2_H__3A9AA990_81DA_45CA_8B8A_C10F79C29AF7__INCLUDED_)
#define AFX_DIALOGBARTAB2_H__3A9AA990_81DA_45CA_8B8A_C10F79C29AF7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DialogBarTab2.h : header file
//
#include "mylistctrl.h"
#include "tchart\tchartglobal.h"
#include "resource.h"
#include "BarTemplate.h"

/////////////////////////////////////////////////////////////////////////////
// DialogBarTab2 dialog
//class SeriesArray;

class DialogBarTab2 : public BarTemplate
{
// Construction
public:
	int cursel;
	DialogBarTab2(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DialogBarTab2)
	enum { IDD = IDD_DIALOGBARTAB2 };
	MyListCtrl	List2;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DialogBarTab2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DialogBarTab2)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL OnInitDialog();
	virtual void OnOK() {};
	afx_msg void OnLvnColumnclickList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnEndlabeleditList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnSeriesUpdate(WPARAM wParam, LPARAM lParam );
	afx_msg HBRUSH OnCtlColor( CDC*, CWnd*, UINT );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNMCustomdrawList2(NMHDR *pNMHDR, LRESULT *pResult);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIALOGBARTAB2_H__3A9AA990_81DA_45CA_8B8A_C10F79C29AF7__INCLUDED_)
