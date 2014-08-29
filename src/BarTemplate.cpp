// BarTemplate.cpp : implementation file
//

#include "stdafx.h"
#include "KSVU3.h"
#include "BarTemplate.h"

// BarTemplate dialog

IMPLEMENT_DYNAMIC(BarTemplate, CDialog)
BarTemplate::BarTemplate(CWnd* pParent /*=NULL*/)
: CDialog() {Parent=0;}

BarTemplate::~BarTemplate() {}

void BarTemplate::DoDataExchange(CDataExchange* pDX) {CDialog::DoDataExchange(pDX);}


BEGIN_MESSAGE_MAP(BarTemplate, CDialog)
	ON_WM_SHOWWINDOW()	
END_MESSAGE_MAP()


// BarTemplate message handlers

void BarTemplate::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	if(Parent!=0)
	{
		if(Parent->m_hWnd!=0) 
		{
			Parent->ShowWindow(bShow);
			CScrollView* pFirstView=(CScrollView*)(Parent->GetParent());
			CRect r; Parent->GetWindowRect(&r);
			pFirstView->SetScrollSizes(MM_TEXT, CSize(r.Width(), r.Height()));
		}
	}
}
