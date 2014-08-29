#pragma once

#include "globalheader.h"
#include "resource.h"
#include "externals.h"
#include "afxcmn.h"


class TConfigDialog : public CDialog
{
	DECLARE_DYNAMIC(TConfigDialog)
public:
	TConfigDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~TConfigDialog();

// Dialog Data
	enum { IDD = IDD_CONFIG };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnUpdateConfig(WPARAM,LPARAM);
public:
	virtual BOOL OnInitDialog();
	CTreeCtrl Params;
	afx_msg void OnNMDblclkTree1(NMHDR *pNMHDR, LRESULT *pResult);
	void BeginEditItem(DWORD);
};
