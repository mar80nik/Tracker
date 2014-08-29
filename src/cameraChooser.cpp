// cameraChooser.cpp : implementation file
//

#include "stdafx.h"
#include "cameraChooser.h"


// CameraChooser dialog

IMPLEMENT_DYNAMIC(CameraChooser, CDialog)
CameraChooser::CameraChooser(CWnd* pParent /*=NULL*/)
	: CDialog(CameraChooser::IDD, pParent)
{
}

CameraChooser::~CameraChooser()
{
}

void CameraChooser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST3, CamerasList);
}


BEGIN_MESSAGE_MAP(CameraChooser, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_LBN_DBLCLK(IDC_LIST3, OnLbnDblclkList3)
	ON_WM_SHOWWINDOW()
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CameraChooser message handlers

void CameraChooser::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
}

void CameraChooser::OnLbnDblclkList3()
{
	int i=CamerasList.GetCurSel(); CString T, T1;
	if(i!=LB_ERR) CamerasList.GetText(i,CaptureName);		
	OnOK();
}

void CameraChooser::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	if(bShow==TRUE)
	{
		MyEnumerator CaptureSrc; MonikersArray arr;
		CaptureSrc.FindSource(ANY_NAME,arr); 
		for (int i=0;i<arr.GetSize();i++)
		{
			CamerasList.AddString(arr[i].GetName());
		}
	}
	else
	{
	}
}

int CameraChooser::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}

BOOL CameraChooser::OnInitDialog()
{
	CDialog::OnInitDialog();
	return TRUE; 
}
