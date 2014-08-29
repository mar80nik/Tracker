// MyToolBar.cpp : implementation file
//

#include "stdafx.h"
#include "MyToolBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MyToolBar

MyToolBar::MyToolBar()
{
}

MyToolBar::~MyToolBar()
{
}


BEGIN_MESSAGE_MAP(MyToolBar, CToolBar)
	//{{AFX_MSG_MAP(MyToolBar)
		// NOTE - the ClassWizard will add and remove mapping macros here.

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MyToolBar message handlers

int MyToolBar::AddButton(UINT buttonID,_refresh r)
{
	int ret=buttons.Add(buttonID);
	if(r==Refresh)
	{
		SetButtons(buttons.GetData(), buttons.GetSize());	
	}
	return ret;
}
