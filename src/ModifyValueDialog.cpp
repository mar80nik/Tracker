// ModifyValueDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ModifyValueDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ModifyValueDialog dialog


ModifyValueDialog::ModifyValueDialog(CWnd* pParent /*=NULL*/)
	: CDialog(ModifyValueDialog::IDD, pParent) 
{
	Caption="Modify value";
}


void ModifyValueDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ModifyValueDialog)
	DDX_Text(pDX, IDC_EDIT1, Value);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ModifyValueDialog, CDialog)
	//{{AFX_MSG_MAP(ModifyValueDialog)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ModifyValueDialog message handlers

BOOL ModifyValueDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();	
	SetWindowPos(&(CWnd::wndTopMost),100,85,0,0, SWP_NOSIZE | SWP_NOACTIVATE );		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void ModifyValueDialog::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);	
	Value=ValueBefore; UpdateData(0);
	SetWindowText(Caption);
	if(bShow) {CWnd *t=GetDlgItem(IDC_EDIT1); CDialog::GotoDlgCtrl(t);}	
}
