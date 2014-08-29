// TConfigDialog.cpp : implementation file
#include "stdafx.h"
#include "TConfigDialog.h"
#include "systemconfig.h"
#include "modifyvaluedialog.h"

// TConfigDialog dialog

IMPLEMENT_DYNAMIC(TConfigDialog, CDialog)

TConfigDialog::TConfigDialog(CWnd* pParent /*=NULL*/)
	: CDialog(TConfigDialog::IDD, pParent) {}
TConfigDialog::~TConfigDialog() {}

void TConfigDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, Params);
}


BEGIN_MESSAGE_MAP(TConfigDialog, CDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, OnNMDblclkTree1)
	ON_MESSAGE(UM_UPDATE_CONFIG,OnUpdateConfig)	
END_MESSAGE_MAP()

BOOL TConfigDialog::OnInitDialog() 
{
	CDialog::OnInitDialog(); AbstractDevice *t;

	CString T; HTREEITEM l0;
	for(int i=0;i<MainCfg.Devices.GetSize();i++)
	{
		t=MainCfg.Devices[i];
		l0=Params.InsertItem(t->Name); Params.SetItemData(l0,t->id); 
		t->FillCfgTree(&Params,l0);
	}
//	MainCfg.Generic.FillCfgTree(&Params,TVI_ROOT);
	return TRUE;
}

void TConfigDialog::OnNMDblclkTree1(NMHDR *pNMHDR, LRESULT *pResult)
{	
	*pResult = 0;
	HTREEITEM item; AbstractDevice* t1; CfgParamTypeBase* t2;
	
	item=Params.GetSelectedItem(); if(!item) return;
	TPD t(Params.GetItemData(item)); 
	if(t.dev==DEV_ERR) return; if(t.param==CFG_PARAM_ERR) return;
	t1=MainCfg.Devices[t.dev-1]; t2=t1->Params[t.param-1];
	if(t2->IsEditable)
	{
		ModifyValueDialog dlg1; 
		t2->GetParamValue(dlg1.ValueBefore);
		if(dlg1.DoModal()==IDOK) 
		{	
			t2->SetParamValue(dlg1.Value); 
			t2->FormatParam(dlg1.Value);
			Params.SetItemText(item,dlg1.Value);
		}	
	}

}

LRESULT TConfigDialog::OnUpdateConfig( WPARAM wParam,LPARAM lParam)
{
	AbstractDevice* d=(AbstractDevice*)wParam; CfgParamTypeBase* p=(CfgParamTypeBase*)lParam;
	CString T; AbstractDevice *t; CfgParamTypeBase* t2;
	
	if(wParam!=0 && lParam!=0)
	{
		HTREEITEM hCurrent = Params.GetChildItem(TVI_ROOT),hChild; 
		while(hCurrent!=NULL)
		{
		if(Params.GetItemData(hCurrent)==d->id)
		{
			hChild=Params.GetChildItem(hCurrent); 
			while(hChild!=NULL)
			{
				TPD data(Params.GetItemData(hChild));	
				if(data.param==p->pid)
				{
					p->FormatParam(T); 
					Params.SetItemText(hChild,T);
				}
				hChild=Params.GetChildItem(hChild); 
			}			
		}
		hCurrent=Params.GetNextItem(hCurrent,TVGN_NEXT);
		}
	}
	else
	{
		for(int i=0;i<MainCfg.Devices.GetSize();i++)
		{
			t=MainCfg.Devices[i];
			for(int j=0;j<t->Params.GetSize(); j++)
			{
				t2=t->Params[j];
				t2->FormatParam(T); 
				Params.SetItemText(t2->TreeItem,T);
			}
		}
	}
	return 0;
}
