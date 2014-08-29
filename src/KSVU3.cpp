// KSVU3.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "KSVU3.h"
#include "MainFrm.h"
#include "KSVU3Doc.h"
#include "KSVU3View.h"
//#include "externals.h"
//#include "chipthread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

////////////////////////////////////////////////////////////////////////////
// CKSVU3App
//MVThread* MV, *Assistant;
//ChipThread *ChipThrd=NULL; ChipDriver *ATmega=NULL;
MainChartWnd GlobalChart;
//CEvent TerminateEvent1(true), TerminateEvent2(true), TerminateEvent3(true), TerminateEvent4(true);
//CEvent TerminateEvent[4]={(true),(true),(true),(true)};
///HICON StartIcon, PauseIcon, ResumeIcon, StopIcon, Start1Icon, Stop1Icon;	
SystemConfig MainCfg;
//CWnd* AbstractButtonVsFeedback::RequestManagerWnd;
WindowAddress EventsLog, TerminalWindow, ControllerWindow, PGADialog, MainFrame;
MessagesInspector GlobalInspector;
MessagesInspector* MessagesInspectorSubject::GlobalInspector=&::GlobalInspector;

BEGIN_MESSAGE_MAP(CKSVU3App, CWinApp)
	//{{AFX_MSG_MAP(CKSVU3App)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_THREAD_MESSAGE(UM_BACKUP_SAVE,OnBackupSave)	
	ON_THREAD_MESSAGE(UM_START,OnStart)	
//	ON_THREAD_MESSAGE(UM_STOP,OnStop)	
	ON_THREAD_MESSAGE(UM_UPDATE_INDICATORS,OnUpdateIndicators)	
	ON_THREAD_MESSAGE(UM_WAVELEN_SET,OnWavelenSet)	
	ON_THREAD_MESSAGE(UM_UPDATE_CONFIG,OnUpdateConfig)	
	ON_THREAD_MESSAGE(UM_DATA_UPDATE,OnDataUpdate)	
//	ON_THREAD_MESSAGE(UM_SERIES_UPDATE,OnSeriesUpdate)	
	ON_THREAD_MESSAGE(UM_GENERIC_MESSAGE,OnGenericMessage)	
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	//}}AFX_MSG_MAP
	// Standard file based document commands
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKSVU3App construction

CKSVU3App::CKSVU3App()
{	
	myThread.Create(this,0,0);
	myThread.Config.PersonalID=MainTaskID;
}

CKSVU3App::~CKSVU3App()
{	
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CKSVU3App object

CKSVU3App theApp;

/////////////////////////////////////////////////////////////////////////////
// CKSVU3App initialization

BOOL CKSVU3App::InitInstance()
{
	AfxEnableControlContainer(); //myThread.Config.Terminate=None;
	CoInitialize(NULL);

//	StartIcon=LoadIcon(IDI_START_ICON); Start1Icon=LoadIcon(IDI_START_ICON1); 
//	PauseIcon=LoadIcon(IDI_PAUSE_ICON);	
//	ResumeIcon=LoadIcon(IDI_RESUME_ICON);
//	StopIcon=LoadIcon(IDI_STOP_ICON); Stop1Icon=LoadIcon(IDI_STOP_ICON1);	

	// Change the registry key under which our settings are stored.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	MainCfg.Parent=this;

	// Register document templates

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CKSVU3Doc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CKSVU3View));
	AddDocTemplate(pDocTemplate);
	
//	icon=theApp.LoadIcon(IDI_START_ICON);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	CMainFrame* MainWnd=(CMainFrame*)m_pMainWnd;
//	EventsLog.pThrd=AfxGetThread(); EventsLog.pWND=&MainWnd->EventLog1;
//	TerminalWindow.pThrd=AfxGetThread(); TerminalWindow.pWND=&MainWnd->Tab4;
//	ControllerWindow.pThrd=AfxGetThread(); ControllerWindow.pWND=&MainWnd->CntrlerWnd; 
//	PGADialog.pThrd=AfxGetThread(); PGADialog.pWND=&MainWnd->PGADlg;
	MainFrame.pThrd=AfxGetThread(); MainFrame.pWND=MainWnd;
	MainCfg.LoadConfig(); 
	MainWnd->InitChart(); 
	if(cmdInfo.m_strFileName=="") MainWnd->TabCtrl1.ChangeTab(1);	
	else MainWnd->TabCtrl1.ChangeTab(0);	
	CString ProgPortName;

//#ifndef DEBUG_PORT 
//	ProgPortName="COM3";
//#else
//	ProgPortName="COM21";	
//#endif
	/*
	CEvent* TempEvent=TerminateEvent;
	MV=0; 
	if((MV=(MVThread*)AfxBeginThread(RUNTIME_CLASS(MVThread),THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED))!=0)
	{
		MV->Create(this,MVThreadID,TempEvent++); MV->ResumeThread();		
		Threads.Add(MV);
	}
	Assistant=0; 
	if((Assistant=(MVThread*)AfxBeginThread(RUNTIME_CLASS(MVThread),THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED))!=0)
	{
		Assistant->Create(this,AssistntID,TempEvent++); Assistant->ResumeThread(); 
		Assistant->Config.Name=CString("Assistatnt"); 		
		Threads.Add(Assistant);
	}
	*/
//	MainWnd->WorkThread=MV;
//	AbstractButtonVsFeedback::RequestManagerWnd=MainWnd;
//	MainWnd->TDlg1.WorkThread=MV;
//	MainWnd->Scope1.WorkThread=MV;
	
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CKSVU3App::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CKSVU3App message handlers
/*
void CKSVU3App::TerminateThreads()
{
	int i,ret,num=Threads.GetSize(); myThread.Config.SetTerminate(ToDo);
	if(ATmega!=NULL) ATmega->StopOperation();
	for(i=0;i<num;i++) 
	{
		ret=(Threads[i])->TerminateThread();	
		ASSERT(ret==0);
	}	
	return;
}
*/
int CKSVU3App::ExitInstance() 
{
	// TODO: Add your specialized code here and/or call the base class
	MainCfg.SaveConfig();
	
//	DestroyIcon(StartIcon);DestroyIcon(PauseIcon); DestroyIcon(StopIcon); 
//	DestroyIcon(Start1Icon); DestroyIcon(Stop1Icon); DestroyIcon(ResumeIcon);	
	myThread.Config.SetTerminate(Done);

	CoUninitialize();
	
	return CWinApp::ExitInstance();
}

void CKSVU3App::OnBackupSave(WPARAM wParam, LPARAM lParam )
{	
	CFile f;
	if(f.Open("backup.dat",CFile::modeCreate | CFile::modeWrite))
	{
		{
			CArchive ar(&f,CArchive::store);
			((CKSVU3Doc*)(((CMainFrame*)MainFrame.pWND)->GetActiveDocument()))->Serialize(ar);
		}
		f.Close();
	}
	return;
}

void CKSVU3App::OnWavelenSet(WPARAM wParam, LPARAM lParam )
{
	if(myThread.Config.GetTerminate()==None) MainFrame.pWND->PostMessage(UM_WAVELEN_SET,0,0);
	return;
}

void CKSVU3App::OnStart(WPARAM wParam, LPARAM lParam )
	{if(myThread.Config.GetTerminate()==None) MainFrame.pWND->PostMessage(UM_START,wParam,lParam);}
void CKSVU3App::OnUpdateIndicators(WPARAM wParam, LPARAM lParam )
	{if(myThread.Config.GetTerminate()==None) MainFrame.pWND->PostMessage(UM_UPDATE_INDICATORS,wParam,lParam);}
void CKSVU3App::OnUpdateConfig(WPARAM wParam, LPARAM lParam )
	{if(myThread.Config.GetTerminate()==None) 
	MainFrame.pWND->PostMessage(UM_UPDATE_CONFIG,wParam,lParam);
	}
void CKSVU3App::OnDataUpdate(WPARAM wParam, LPARAM lParam )
	{if(myThread.Config.GetTerminate()==None) m_pMainWnd->PostMessage(UM_DATA_UPDATE,wParam,lParam);}

void CKSVU3App::OnGenericMessage( WPARAM wParam, LPARAM lParam )
{
	MessageForWindow* msg=(MessageForWindow*)lParam;

	if(myThread.Config.GetTerminate()==None)
	{
		msg->wParam=wParam;
//		msg->Dispatch();
	}
}