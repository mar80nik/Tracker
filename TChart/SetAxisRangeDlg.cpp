// SetAxisRangeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SetAxisRangeDlg.h"


// SetAxisRangeDlg dialog

IMPLEMENT_DYNAMIC(SetAxisRangeDlg, CDialog)

SetAxisRangeDlg::SetAxisRangeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(SetAxisRangeDlg::IDD, pParent)
	, MaxY(0)
	, MinY(0)
{

}

SetAxisRangeDlg::~SetAxisRangeDlg()
{
}

void SetAxisRangeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, MaxY);
	DDX_Text(pDX, IDC_EDIT2, MinY);
}


BEGIN_MESSAGE_MAP(SetAxisRangeDlg, CDialog)
END_MESSAGE_MAP()


// SetAxisRangeDlg message handlers
