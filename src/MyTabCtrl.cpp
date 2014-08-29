// MyTabCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "MyTabCtrl.h"
#include "mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MyTabCtrl

MyTabCtrl::MyTabCtrl(CWnd *_Parent, int _selfID)
{
	Parent=_Parent;selfID=_selfID; curTab=0;
}

MyTabCtrl::~MyTabCtrl()
{
	for(int i=0;i<Tabs.GetSize();i++)
		if(Tabs[i]->m_hWnd) Tabs[i]->DestroyWindow();
}


/////////////////////////////////////////////////////////////////////////////
// MyTabCtrl message handlers

void MyTabCtrl::AddTab(CString name,CDialog *temp, int id)
{
	TabCtrl=(CTabCtrl*)Parent->GetDlgItem(selfID);
	if(TabCtrl)
	{
		item.mask=TCIF_TEXT; item.pszText=(char *)LPCTSTR(name); 
		
		int u=Tabs.Add(temp); TabCtrl->InsertItem(u,&item);
		temp->Create(id,TabCtrl);	
		CRect rect;
		TabCtrl->GetClientRect(&rect); TabCtrl->AdjustRect(false,&rect);
		temp->MoveWindow(&rect);	
	}
}



void MyTabCtrl::ChangeTab(int num)
{
	Tabs[curTab]->ShowWindow(SW_HIDE);
	TabCtrl=(CTabCtrl*)Parent->GetDlgItem(selfID);
	if(TabCtrl)
	{
		if(num>=0) 
		{ 
			if(num>=Tabs.GetSize()) num=Tabs.GetSize()-1; 			
			TabCtrl->SetCurSel(num);	
		}
		curTab=TabCtrl->GetCurSel();
		Tabs[curTab]->ShowWindow(SW_SHOW);
	}	
}

int MyTabCtrl::SetCurSel(int nItem)
{
	int ret=-1;
	TabCtrl=(CTabCtrl*)Parent->GetDlgItem(selfID);
	if(TabCtrl)
	{
		ret=TabCtrl->SetCurSel(nItem);		
	}	
	return ret;
}

int MyTabCtrl::FindTab( CString name )
{
	int ret=-1; TCITEM t; BYTE buf[100];
	t.mask=TCIF_TEXT; t.cchTextMax=100; t.pszText=(LPTSTR)buf;
	TabCtrl=(CTabCtrl*)Parent->GetDlgItem(selfID);
	if(TabCtrl)
	{
		for(int i=0;i<TabCtrl->GetItemCount();i++)
		{
			TabCtrl->GetItem(i,&t);
			if(name==t.pszText)
			{
				ret=i; break;
			}
		}		
	}	
	return ret;
}
