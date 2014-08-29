// MyListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "MyListCtrl.h"
#include "SeriesValuesDialog.h"
#include "tchartseries.h"
#include "mythread.h"
#include "mainfrm.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include ".\mylistctrl.h"

/////////////////////////////////////////////////////////////////////////////
// MyListCtrl

IMPLEMENT_DYNAMIC(MyListCtrl, CListCtrl)

//#define IDB_STATEICONS 0xffffff01

MyListCtrl::MyListCtrl()
{
	ImageList2.Detach();
	ImageList2.Create(IDB_STATEICONS,16,1,clRED);	
	SortStatus=1; SortMask=3;
    Series=0;
	bckg_color=GetSysColor(COLOR_BTNFACE);
	bckg.CreateSolidBrush(bckg_color);
}

MyListCtrl::~MyListCtrl()
{
}


BEGIN_MESSAGE_MAP(MyListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(MyListCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_SERIESMENU_DELETESERIES,OnDeleteSeries)
	ON_COMMAND(ID_SERIESMENU_SAVEASTEXT,OnSaveAsText)
	ON_COMMAND(ID_SERIESMENU_CHANGESTYLE,OnMenuChangeStyle)

	ON_WM_CONTEXTMENU()	
	ON_COMMAND(ID_CHANGESTATUS_ACTIVE, OnChangestatusActive)
	ON_COMMAND(ID_CHANGESTATUS_INACTIVE, OnChangestatusInactive)
	ON_COMMAND(ID_CHANGESTATUS_CALIBRATOR, OnChangestatusCalibrator)
	ON_COMMAND(ID_SERIESMENU_NORMALIZE, OnSeriesmenuNormalize)
	ON_WM_KEYDOWN()
	ON_WM_CTLCOLOR( )
	ON_COMMAND(ID_SERIESMENU_SHOWVALUES, OnSeriesmenuShowvalues)
//	ON_WM_CREATE()
	//}}AFX_MSG_MAP
//	ON_WM_CREATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MyListCtrl message handlers

LRESULT MyListCtrl::OnDataUpdate(WPARAM wParam, LPARAM lParam )
{
	UpdateSeriesList();	
	return 0;
}

void MyListCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	ProtectedSeriesArray& Series=*(MyListCtrl::Series); void *x;
	HitItem=HitTest(point);
	
	if (HitItem>=0)
	{
		CRect rect; GetItemRect(HitItem, rect, LVIR_ICON);
		if (point.x < rect.left)
		{
			BYTE state=GetState(HitItem);
			state++; if(state>2) state=1;
			SetState(HitItem,state); 			
			
			if((x=Series.GainAcsess(WRITE))!=0)
			{
				SeriesProtector Protector(x); TSeriesArray& Series(Protector);
				Series[GetItemData(HitItem)]->SetVisible(state-1);
			}
		}		
	}		
	CListCtrl::OnLButtonDown(nFlags, point);
}


void MyListCtrl::SetState(int num, BYTE state)
{
	LV_ITEM lvi;	
	lvi.mask = LVIF_STATE;
	lvi.iItem = num;
	lvi.iSubItem = 0;		
	lvi.stateMask = LVIS_STATEIMAGEMASK;
	lvi.state = INDEXTOSTATEIMAGEMASK(state);
	SetItemState(num,&lvi);	
}

BYTE MyListCtrl::GetState(int num)
{
	UINT state=GetItemState(num,LVIS_STATEIMAGEMASK);
	return (BYTE)(state>>12); 
}

void MyListCtrl::UpdateSeriesList()
{
	int n=HitItem; LV_ITEM lvi; CString T;	void *x; CBitmap * tmpbmp; BMPanvas icon;

	if((x=Series->GainAcsess(READ))!=0)
	{
		SeriesProtector Protector(x); TSeriesArray& Series(Protector);
		int n1=Series.GetSize(); if(n>=n1) n=n-1;
//		if(LastN!=n1) 
		{			
			DeleteAllItems(); Items.RemoveAll();
			ImageList1.Detach(); 
			ImageList1.Create(16,16,ILC_COLOR,Series.GetSize(),1);
			
			for(int i=0;i<Series.GetSize();i++)
			{
				icon.Create(CreateCompatibleDC(NULL),16,16,24);

				HGDIOBJ brsh=icon.SelectObject(bckg);
				icon.PatBlt(PATCOPY);				
				icon.SelectObject(brsh);	

				Series[i]->UpdateIcon(icon);
				HBITMAP hbmp=icon.GetHBMP(BMP_DETACH);
				tmpbmp=CBitmap::FromHandle(hbmp);
				ImageList1.Add(tmpbmp,(CBitmap*)NULL);
				lvi.mask = LVIF_PARAM; lvi.iItem = i;lvi.iSubItem = 0; lvi.lParam=i;			
				InsertItem(&lvi); Items.Add(ListItemData());
			}		
			SetImageList(&ImageList1,LVSIL_SMALL);
		}
		for(int i=0;i<GetItemCount();i++)
		{
			int k=GetItemData(i); TChartSeries* series=Series[k];
			lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
			lvi.iItem = i; lvi.iSubItem = 0;
			lvi.pszText = (char*)LPCTSTR(series->Name); 
			lvi.iImage = i;		
			lvi.stateMask = LVIS_STATEIMAGEMASK;
			lvi.state = INDEXTOSTATEIMAGEMASK(series->IsVisible()+1);
			SetItem(&lvi);	

			lvi.mask = LVIF_TEXT; lvi.iSubItem++;		
			T.Format("%d",series->GetSize()); SetItemText(i,lvi.iSubItem++,(char*)LPCTSTR(T)); 
			series->GetStatus(T); SetItemText(i,lvi.iSubItem++,(char*)LPCTSTR(T));	
			T.Format("%d",series->PID); SetItemText(i,lvi.iSubItem++,(char*)LPCTSTR(T)); 

			lvi.mask=LVIF_STATE; lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;		
			lvi.state = 0; SetItemState(i,&lvi);		

			Items[k].Name=series->Name; Items[k].Size=series->GetSize(); Items[k].Status=series->GetStatus();	
			Items[k].PID=series->PID;			
		}
		LastN=Series.GetSize();
		if(n>=0)
		{	
			_ms_lvi.mask=LVIF_STATE; _ms_lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
			_ms_lvi.state = LVIS_SELECTED | LVIS_FOCUSED;			
			LV_ITEM FAR * temp=&_ms_lvi;
			this->PostMessage(LVM_SETITEMSTATE ,(WPARAM)(n), (LPARAM)(temp));
			EnsureVisible(n, false);	
		}	
		//SortItems(CompareFunc1,(LPARAM) (this));	
	}
}

void MyListCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	HitItem=HitTest(point); bool err=true; 
	SeriesValuesDialog SeriesValues1;
	if(HitItem>=0)
	{		
		SeriesValues1.Series=Series; SeriesValues1.graph_num=GetItemData(HitItem);
		SeriesValues1.DoModal();		
	}
	
	CListCtrl::OnLButtonDblClk(nFlags, point);
}

int MyListCtrl::HitTest(CPoint point)
{
	UINT f = LVHT_ONITEM; 
	return CListCtrl::HitTest(point, &f);
}

int MyListCtrl::HitTest()
{
	int ret=-1;
	POSITION pos=GetFirstSelectedItemPosition(); 
	if(pos) ret=GetItemData(GetNextSelectedItem(pos));
	return ret;
}

char Compare(int v1,int v2, char inv)
{
	char ret;
	if(v1==v2) ret=0;
	else ret=inv*(v1>v2 ? -1:1);	
	return ret;
}

int CALLBACK CompareFunc1(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	int ret=0; 
	MyListCtrl* pListCtrl = (MyListCtrl*) lParamSort;
	ListItemData Item1=pListCtrl->Items[lParam1];
	ListItemData Item2=pListCtrl->Items[lParam2];

	int& mask=pListCtrl->SortMask; int &Status=pListCtrl->SortStatus;
	int n=1,v1,v2; while(mask>>n) n++; n--; 
	char inv=(Status&mask ? -1:1);

	switch(n)
	{
	case 0:
		ret=inv*strcmp(Item1.Name,Item2.Name); break;		
	case 1:
		v1=Item1.Size; v2=Item2.Size; ret=Compare(v1,v2,inv); break;		
	case 2: 		
		v1=Item1.Status; v2=Item2.Status; ret=Compare(v1,v2,inv); break;		
	case 3: 		
		v1=Item1.PID; v2=Item2.PID; ret=Compare(v1,v2,inv); break;		
	default:	;
	}
	return ret;
}

void MyListCtrl::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CMenu menu1; menu1.LoadMenu(IDR_MENU1);
	CMenu *menu2=menu1.GetSubMenu(0); UINT mode;

	POSITION pos=GetFirstSelectedItemPosition();
	if(pos)
	{
		int n=GetItemData(GetNextSelectedItem(pos));
		if(Items[n].Status==SER_LOCKED) mode=MF_BYPOSITION | MF_GRAYED;
		else mode=MF_BYPOSITION | MF_ENABLED;
        
		menu2->EnableMenuItem(2,mode);
		menu2->EnableMenuItem(3,mode);
	}

	menu2->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON,point.x,point.y,pWnd);	
}

int compfunc(const void *t1, const void *t2)
{
	int& e1=*((int*)t1); int& e2=*((int*)t2); int ret=1;
	if(e1==e2) ret=0;
	if(e1>e2) ret=-1;
	return ret;
}

void MyListCtrl::OnDeleteSeries()
{
	POSITION pos=GetFirstSelectedItemPosition(); bool upd=false; void *x;
	if(pos)
	{
		CArray<int,const int&> ToDel; int i; CString temp; 
		while(pos) ToDel.Add(GetItemData(GetNextSelectedItem(pos)));
		qsort(ToDel.GetData(),ToDel.GetSize(),sizeof(int),compfunc);

		for(i=0;i<ToDel.GetSize();i++)
		{
			int n=ToDel[i];			
			temp.Format("Series %s contains %d points. Remove it?",Items[n].Name,Items[n].Size);
			if(AfxMessageBox(temp,MB_YESNO)==IDNO) ToDel[i]=-1;		
		}
		
		if((x=Series->GainAcsess(WRITE))!=0)
		{
			upd=true;
			SeriesProtector Protector(x); TSeriesArray& Series(Protector);
			{
				for(i=0;i<ToDel.GetSize();i++)
					if(ToDel[i]>=0) Series.DeleteItem(ToDel[i]);
			}
		}
		if(upd) UpdateSeriesList();
	}		
}

void MyListCtrl::OnSaveAsText()
{

	CFileDialog fd(false,"txt"); BYTE cr[3]; cr[0]=0xd;cr[1]=0xa;cr[2]=0;
	int i,j, match, err=0, num;	

	POSITION pos=GetFirstSelectedItemPosition();		
	CArray<int,const int&> arr; CString temp; CString T,T1;
	while(pos) arr.Add(GetItemData(GetNextSelectedItem(pos)));
	for(i=0;i<arr.GetSize() && !err;i++)
	{
		match=0;
		for(j=i+1;j<arr.GetSize();j++) 
		{
			T=Items[arr[j]].Name; T1=Items[arr[i]].Name;
			if(!(strcmp(Items[arr[j]].Name,Items[arr[i]].Name))) match++;
		}
		if(match>0) err=1;
	}
	
	if(!(arr.GetSize()) || err)
	{
		AfxMessageBox("ВНИМАНИЕ!!! Один или более Series имеют одинаковые имена. Выполнение операции невозможно!");
		return;	
	}
	else
	{
		CString temp;	CFile f; err=0; num=0; void *x;		
		if((x=Series->GainAcsess(READ))!=0)
		{
			SeriesProtector Protector(x); TSeriesArray& Series(Protector);
			for(i=0;i<arr.GetSize();i++)
			{
				TChartSeries *series=Series[arr[i]];
				CMainFrame* MW=(CMainFrame*)AfxGetMainWnd();
				temp=MW->GetActiveDocument()->GetPathName();
				int t; if((t=temp.ReverseFind('\\'))>=0) temp=temp.Left(++t);	
				temp+=series->Name+".xls";		

				HANDLE SearchHandle;
				WIN32_FIND_DATA FindFileData; TCHAR szError[1024];
				if( (SearchHandle=FindFirstFile(LPCSTR(temp),&FindFileData) )==INVALID_HANDLE_VALUE)
				{
					CFileException ex;
					if(f.Open(temp,CFile::modeCreate | CFile::modeWrite,&ex))
					{						
						temp.Format("%s: points=%d",series->Name,series->GetSize());  //temp+=cr;					
						//f.Write(LPCSTR(temp),temp.GetLength());f.Write(cr,sizeof(cr));
						
						for(int i=0;i<series->GetSize();i++)
						{
							series->FormatElement(i,temp);

							char* fs;
							while((fs=const_cast<char*>(strchr(temp,'.')))!=NULL) 
								(*fs)=',';
                            

							f.Write(LPCSTR(temp),temp.GetLength());
							f.Write(cr,sizeof(cr));
						}
						f.Close(); num++;
					}
					else 
					{
						ex.GetErrorMessage(szError, 1024);
					}
				}
				else err++;
			}
		}
		
		if(err) 
		{
			temp.Format("ВНИМАНИЕ!!! %d из %d файлов не были созданы. Файлы с такими именами уже существуют.",err, num+err);
			AfxMessageBox(temp);		
		}
		else
		{
			temp.Format("Операция прошла успешно. Создано %d файлов", num);		
			AfxMessageBox(temp);		
		}
	}		
}

void MyListCtrl::ChangeStatus(int t)
{
	POSITION pos=GetFirstSelectedItemPosition(); void *x;
	if(pos)
	{		
		if((x=Series->GainAcsess(READ_EX))!=0)
		{
			SeriesProtector Protector(x); TSeriesArray& Series(Protector);
			while(pos)
			{
				int n=GetItemData(GetNextSelectedItem(pos)); 			
				TChartSeries *series=Series[n];		
				series->SetStatus(t);
			}
		}
		UpdateSeriesList();
	}	
}

void MyListCtrl::OnChangeStatus()
{
}

void MyListCtrl::OnChangestatusActive() {ChangeStatus(SER_ACTIVE);}
void MyListCtrl::OnChangestatusInactive() {ChangeStatus(SER_INACTIVE);}
void MyListCtrl::OnChangestatusCalibrator() {ChangeStatus(SER_CALIBRATOR);}


void CreateCalibratedSeries() {};

void MyListCtrl::OnSeriesmenuNormalize()
{
  
	return;
}

void MyListCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_DELETE: OnDeleteSeries(); break;
	case 'A': HitItem=HitTest(); OnChangestatusActive(); break;
	case 'I': OnChangestatusInactive(); break;
	case 'C': OnChangestatusCalibrator(); break;
	}
	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}


void MyListCtrl::OnSeriesmenuShowvalues()
{
	POSITION pos=GetFirstSelectedItemPosition(); int n; BOOL show=FALSE;
	SeriesValuesDialog SeriesValues1;
	if(pos)
	{
		SeriesValues1.Series=Series; SeriesValues1.graph_num=n=GetItemData(GetNextSelectedItem(pos));
		SeriesValues1.DoModal();		
	}
}

HBRUSH MyListCtrl::OnCtlColor( CDC*, CWnd*, UINT )
{
	return bckg;
}

void MyListCtrl::OnMenuChangeStyle()
{
	POSITION pos=GetFirstSelectedItemPosition(); int n; BOOL show=FALSE;
	SeriesStyleDialog dlg1;
	
	if(pos)
	{
		dlg1.Series=Series; dlg1.graph_num=n=GetItemData(GetNextSelectedItem(pos));
		dlg1.DoModal();		
	}
}

