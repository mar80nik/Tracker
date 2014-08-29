// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "KSVU3.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "ksvu3doc.h"
#include ".\mainfrm.h"
/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_MAIN_TAB, OnMainTabAccel)
	ON_COMMAND(ID_IMAGE_TAB, OnImageTabAccel)
	ON_COMMAND(IDB_LOG_BUTTON, OnEventlog)
	ON_COMMAND(ID_EVENT_LOG, OnEventlog)	
	ON_COMMAND(ID_VIEW_CONFIG, OnConfig)	
	
	ON_NOTIFY(TCN_SELCHANGE,IDC_TAB1,OnTabChange)
	ON_WM_SHOWWINDOW()
	ON_WM_CLOSE()
//	ON_MESSAGE(UM_UPDATE_INDICATORS,OnUpdateIndicators)	
//	ON_MESSAGE(UM_UPDATE_CONFIG,OnUpdateConfig)	
//	ON_MESSAGE(UM_WAVELEN_SET,OnWavelenSet)	
//	ON_MESSAGE(UM_DATA_UPDATE,OnDataUpdate)	
	ON_MESSAGE(UM_SERIES_UPDATE,OnSeriesUpdate)	
//	ON_MESSAGE(UM_STOP,OnStop)	
//	ON_MESSAGE(UM_START,OnStart)	
//	ON_MESSAGE(UM_PAUSE,OnPause)	
//	ON_MESSAGE(UM_CONTINUE,OnContinue)	
//	ON_MESSAGE(UM_ACCESS_REQUEST, OnAccessRequest)
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame(): TabCtrl1(&MainBar,IDC_TAB1), Chart1(GlobalChart)
{
}

CMainFrame::~CMainFrame()
{
	Chart1.DestroyWindow();
	SeriesList.DestroyWindow();	
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{	
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	if (!m_wndStatusBar.Create(this,WS_CHILD | WS_VISIBLE | CBRS_BOTTOM | CBRS_NOALIGN))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	MainBar.Create(this,IDD_DIALOGBAR,CBRS_ALIGN_BOTTOM,IDD_DIALOGBAR);

	Toolbar1.Create(this,WS_CHILD | WS_VISIBLE | CBRS_RIGHT); Toolbar1.LoadToolBar(IDR_TOOLBAR2);
	Toolbar1.AddButton(IDB_LOG_BUTTON,MyToolBar::Refresh);
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnClose() 
{
	EventLog1.DestroyWindow();
	Config.DestroyWindow();
	Img.DestroyWindow();
	Chart1.DestroyWindow();
	CFrameWnd::OnClose();
}

void CMainFrame::OnConfig() {Config.ShowWindow(SW_SHOW);}

void CMainFrame::OnEventlog() 
{
	EventLog1.ShowWindow(SW_SHOW);
}

void CMainFrame::OnMainTabAccel() 
{
	TabCtrl1.ChangeTab(0);
}

void CMainFrame::OnImageTabAccel()
{
	TabCtrl1.ChangeTab(1);
}

void CMainFrame::OnTabChange( NMHDR * pNotifyStruct, LRESULT * result )
{	
	TabCtrl1.ChangeTab();
	if(pNotifyStruct) *result=0;
}

void CMainFrame::InitChart() 
{
	CScrollView* pFirstView = (CScrollView*)GetActiveView();
	CRect r; pFirstView->GetClientRect(r);		

//	Tab4.Parent=&Tab4.LogWnd;

	TabCtrl1.AddTab("Main control",&Chart1.Panel,IDD_DIALOGBARTAB11);	
	TabCtrl1.AddTab("Image control",&Img.Ctrls,IDD_DIALOGBARTAB1);		

	Img.Create(0, "ImageWnd", WS_CHILD, r, pFirstView, ID_MV_WND+1, 0);	

#ifdef DEBUG
//	Img.CameraWnd.SelectCaptureSrc(CString("A4 tech USB2.0 Camera"));
#else
	Img.CameraWnd.SelectCaptureSrc(CString("ScopeTek DCM800"));
#endif

	Chart1.Create(pFirstView,r); Chart1.SetVisible(true); Chart1.SeriesDataWnd=&SeriesList;

//	CntrlerWnd.Create(0, "MV_MainWnd", WS_CHILD | WS_BORDER, r, pFirstView, ID_MV_WND+2, 0);	
//	Scope1.Create(pFirstView,r);	

//	DWORD styleEx=WS_EX_DLGMODALFRAME | WS_EX_NOPARENTNOTIFY;
//	CntrlerWnd.volt_out_ctrl1.ModifyStyleEx(0,styleEx);

//	Chart1.Panel.StartButton.SetFocus();
//	m_wndStatusBar.Tab1=&(Chart1.Panel);
}

void CMainFrame::Serialize(CArchive& ar)
{		
	Chart1.Serialize(ar);
}

void CMainFrame::OnShowWindow(BOOL bShow, UINT nStatus) 
{	
	ShowWindow(SW_SHOWMAXIMIZED);
	CFrameWnd::OnShowWindow(bShow, nStatus);

//	ModifyStyle(0,WS_MAXIMIZE,SWP_FRAMECHANGED);
	
	if(bShow)
	{		

		EventLog1.Create(IDD_DIALOG5,this);
//		TDlg1.Create(IDD_DIALOG12,this);
		Config.Create(IDD_CONFIG,this);
		SeriesList.Create(IDD_DIALOGBARTAB2,this);	
//		PGADlg.Create(IDD_DIALOG11,this);
	}
}
		// t1 - num of indicator
		// t2 - what to do	0 - show/hide (t3=1/0)
		//					1 - set text (t3=string resource id)
		// t3 - information depend on t2			
/*
LRESULT CMainFrame::OnUpdateIndicators(WPARAM wParam,LPARAM lParam)
{
	CString T;
	MyThread::ParentStatusReport* rep=(MyThread::ParentStatusReport*)wParam;
	wParam=rep->ConfigCopy.PersonalID; 
	if(lParam!=NULL)
	{
		UniformMessagesArray *Msgs=(UniformMessagesArray*)lParam; 
		for(int i=0;i<Msgs->GetSize();i++) 
		{
			StatusBarMsg& mes=*((StatusBarMsg*)(*Msgs)[i]);
			if(mes.Pane>=0)
			{
				switch(mes.Action)
				{
				case MyStatusBar::SHOW: m_wndStatusBar.Show(mes.Pane);break;
				case MyStatusBar::HIDE: m_wndStatusBar.Hide(mes.Pane); break;
				case MyStatusBar::SET: 	m_wndStatusBar.SetText(mes.Pane,mes.Text);	break;
				}					
			}
		}
		delete Msgs;
	}
	else
	{
		m_wndStatusBar.SetText(0,rep->ConfigCopy.ActionDescription); 
	}
	delete rep;
	return 0;
}
*/
/*
LRESULT CMainFrame::OnWavelenSet(WPARAM wParam, LPARAM lParam )
{
	CString T; nmC t2; t2=MainCfg.GetWavelength();
	nm t1; t1=t2;
	T.Format("%4.2lf",t1.val());
	m_wndStatusBar.SetText(IND_WAVELEN,T);	
	CntrlerWnd.PostMessage(UM_WAVELEN_SET,0,0);
	return 0;
}
*/
/*
LRESULT CMainFrame::OnUpdateConfig(WPARAM wParam, LPARAM lParam )
{
//	m_wndStatusBar.SendMessage(UM_UPDATE_CONFIG,wParam,lParam);
//	CntrlerWnd.SendMessage(UM_UPDATE_CONFIG,wParam,lParam);
	Config.SendMessage(UM_UPDATE_CONFIG,wParam,lParam);
	return 0;
}
*/
/*
LRESULT CMainFrame::OnDataUpdate(WPARAM wParam, LPARAM lParam )
{
	MyThread::ParentStatusReport* rep=(MyThread::ParentStatusReport*)wParam;
	wParam=rep->ConfigCopy.PersonalID; delete rep;
	switch(wParam)
	{
	case MVThreadID:	
	case MVThreadID1:	
	case MVThreadID2:			
		CntrlerWnd.PostMessage(UM_DATA_UPDATE,0,lParam); break;	
	case MainTaskID:
		Chart1.PostMessage(UM_DATA_UPDATE,0,lParam);
		SeriesList.List2.UpdateSeriesList();	break;
	case TThreadID:		
	case TThreadID1:		
	case TThreadID2:				
//		TDlg1.PostMessage(UM_DATA_UPDATE,wParam,lParam);	break;
	case OscilThreadID:	Scope1.PostMessage(UM_DATA_UPDATE,0,lParam); break;
	}	
	return 0;
}
*/
LRESULT CMainFrame::OnSeriesUpdate(WPARAM wParam, LPARAM lParam )
{
	Img.Ctrls.CalibratorDlg.PostMessage(UM_SERIES_UPDATE,wParam,lParam);	
	Img.Ctrls.CalcTEDlg.PostMessage(UM_SERIES_UPDATE,wParam,lParam);	
	Img.Ctrls.CalcTMDlg.PostMessage(UM_SERIES_UPDATE,wParam,lParam);	
	Chart1.Panel.PostMessage(UM_SERIES_UPDATE,wParam,lParam);	
	return 0;
}
/*
LRESULT CMainFrame::OnStop(WPARAM wParam, LPARAM lParam )
{
	ErrorsArray* err=(ErrorsArray*)lParam;
	if(err!=NULL) 
	{
		LogMessage *log=new LogMessage(); log->CreateEntry(err);
        delete err; lParam=0;		
		log->Dispatch();		
	}
	MyThread::ParentStatusReport* rep=(MyThread::ParentStatusReport*)wParam;
	wParam=rep->ConfigCopy.PersonalID; delete rep;

	m_wndStatusBar.SetText(0,CString("")); 
	SearchForMsgRequest(UM_STOP,wParam,lParam);

	return 0;
}

LRESULT CMainFrame::OnStart(WPARAM wParam, LPARAM lParam )
{
	MyThread::ParentStatusReport* rep=(MyThread::ParentStatusReport*)wParam;
	wParam=rep->ConfigCopy.PersonalID; delete rep;
	switch(wParam)
	{
	case MVThreadID:	
	case MVThreadID1:	
	case MVThreadID2:	
		CntrlerWnd.SendMessage(UM_START,wParam,lParam); 
		break;	
	case MainTaskID:	Chart1.SendMessage(UM_START,wParam,lParam); break;
	case TThreadID:		TDlg1.SendMessage(UM_START,wParam,lParam); break;
	case OscilThreadID:	Scope1.SendMessage(UM_START,wParam,lParam); break;
	}
	return 0;
}

LRESULT CMainFrame::OnPause(WPARAM wParam, LPARAM lParam )
{
	MyThread::ParentStatusReport* rep=(MyThread::ParentStatusReport*)wParam;
	wParam=rep->ConfigCopy.PersonalID; 
	MessageForWindow* msg=(MessageForWindow*)lParam; delete msg; lParam=0;

	m_wndStatusBar.SetText(0,rep->ConfigCopy.ActionDescription+CString(" [PAUSED]")); 

	SearchForMsgRequest(UM_PAUSE,wParam,lParam);
	delete rep;
	return 0;
}

LRESULT CMainFrame::OnContinue(WPARAM wParam, LPARAM lParam )
{
	MyThread::ParentStatusReport* rep=(MyThread::ParentStatusReport*)wParam;
	wParam=rep->ConfigCopy.PersonalID; 
	MessageForWindow* msg=(MessageForWindow*)lParam; delete msg; lParam=0;

	m_wndStatusBar.SetText(0,rep->ConfigCopy.ActionDescription); 

	SearchForMsgRequest(UM_CONTINUE,wParam,lParam);
	delete rep;
	return 0;
}

LRESULT CMainFrame::OnAccessRequest( WPARAM wParam,LPARAM lParam)
{
	RequestForAcsessMsg* msg=(RequestForAcsessMsg*)wParam;
	ReqArr.Add(msg->data);
	switch (msg->data.msg)
	{
	case UM_STOP: WorkThread->StopThread(false); break;
	case UM_PAUSE: WorkThread->PauseThread(); break;
	case UM_CONTINUE: WorkThread->ContinueThread(); break;
	default: ASSERT(0);
	}
	delete msg;
	RequestForAcsess tt; tt=ReqArr[0];
	return 0;	
}


bool CMainFrame::SearchForMsgRequest(DWORD msg, WPARAM wParam, LPARAM lParam )
{
	bool find=false;
	if(ReqArr.GetSize()==0) return find;	
	for (int i=0;i<ReqArr.GetSize();i++) 
	{
		if(ReqArr[i].msg==msg)
		{
			if(ReqArr[i].WorkThreadID==wParam || ReqArr[i].WorkThreadID==DONT_CARE)
			{
				if(!find) 
				{
					ReqArr[i].wnd->PostMessage(UM_ACCESS_GRANTED,(WPARAM)WorkThread,0); 
					ReqArr.RemoveAt(i);
					find=true;
				}
				else 
				{
					ReqArr[i].wnd->PostMessage(UM_ACCESS_DEINIED,0,0); 
					ReqArr.RemoveAt(i);
				}
			}			
		}		
	}
	return find;
}
*/

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);
}

void * CMainFrame::GetChartFromParent()
{
	return ((void*)&Chart1);
}
