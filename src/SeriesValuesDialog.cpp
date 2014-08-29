// SeriesValuesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SeriesValuesDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SeriesValuesDialog dialog


SeriesValuesDialog::SeriesValuesDialog(CWnd* pParent /*=NULL*/)
	: CDialog(SeriesValuesDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(SeriesValuesDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void SeriesValuesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SeriesValuesDialog)
	DDX_Control(pDX, IDC_LIST2, ListBox2);
	DDX_Control(pDX, IDC_LIST1, ListBox1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SeriesValuesDialog, CDialog)
	//{{AFX_MSG_MAP(SeriesValuesDialog)
	//}}AFX_MSG_MAP
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST2, &SeriesValuesDialog::OnLvnKeydownList2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SeriesValuesDialog message handlers
void SeriesValuesDialog::InitDialog(TChartSeries* Series1)
{	/*
	CString T;
	if(Series1)
	{
		Series=Series1;		
		Series->GetInfo(strs); int n=Series->GetSize();
		for(int i=0; i<n; i++)
		{	
			Series->FormatElement(i,T); 
			strsDATA.Add(T);
		}	
	}
	*/
}


BOOL SeriesValuesDialog::OnInitDialog() 
{
	int i, j; CString T, T1;

	CDialog::OnInitDialog();
	
	void *x;
	if((x=Series->GainAcsess(READ))!=NULL)
	{
		SeriesProtector guard(x); TSeriesArray& series(guard);
		TChartSeries *graph=series[graph_num];
		graph->GetInfo(strs); ListBox1.ResetContent();  for(i=0;i<strs.GetSize();i++) ListBox1.AddString(strs[i]);			

		LV_ITEM lvi; lvi.mask = LVIF_TEXT;
		ListBox2.InsertColumn(0,"N",LVCFMT_CENTER,40,0);
		ListBox2.InsertColumn(1,"X",LVCFMT_CENTER,50,1);
		ListBox2.InsertColumn(2,"Y",LVCFMT_CENTER,60,2);
		ListBox2.InsertColumn(3,"dY",LVCFMT_CENTER,60,3);	
		ListBox2.InsertColumn(4,"Type",LVCFMT_CENTER,60,3);		
		ListBox2.DeleteAllItems();

		for(i=0;i<graph->GetSize();i++)
		{
			lvi.iItem = i; lvi.iSubItem = 0; T.Format("%d",i);		
			lvi.pszText = (char*)LPCTSTR(T);
			ListBox2.InsertItem(&lvi);
			graph->FormatElement(i,T); 
			int j_last=0, j_cur;
			for(j=0;j<4;j++)
			{			
				if((j_cur=T.Find(9,j_last))>0)
				{
					T1=T.Mid(j_last,j_cur-j_last);
					ListBox2.SetItemText(i,j+1,(char*)LPCTSTR(T1));					
					j_last=j_cur+1;
				}
			}	
		}
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void SeriesValuesDialog::OnLvnKeydownList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
	int i,n; POSITION pos; CArray<int,const int&> arr; CString temp; CString T,T1;
	void *x; TChartSeries *graph;

	switch(pLVKeyDow->wVKey)
	{
	case VK_DELETE:		
		if((x=Series->GainAcsess(WRITE))!=NULL)
		{
			SeriesProtector guard(x); TSeriesArray& series(guard);
			graph=series[graph_num];
			n=graph->GetSize();
			pos=ListBox2.GetFirstSelectedItemPosition();				
			while(pos) arr.Add(ListBox2.GetNextSelectedItem(pos));
			graph->ParentUpdate(UPD_OFF);
			for(i=0;i<arr.GetSize();i++)
			{
				n=arr[i];
				graph->RemoveAt(n-i);
				ListBox2.DeleteItem(n-i);
			}
			graph->ParentUpdate(UPD_ON);
			graph->PostParentMessage(UM_SERIES_UPDATE);	
		}
		break;
	}
	*pResult = 0;
}
