// MyThread.cpp : implementation file
//

#include "stdafx.h"
#include "MyThread.h"
#include "resource.h"
#include "errors.h"
//#include "..\KSVU3.h"
#include "GlobalHeader.h"
#include "MessageInspector.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/*
int ResourceMSG::classID=ResourceMSG_TYPE; 
int ErrorMessage::classID=ErrorMSG_TYPE;
int Error::classID=ErrorResMSG_TYPE;
int Warning::classID=WarningResMSG_TYPE;
int ErrorsArray::classID=ERR_ARR_MSG_TYPE;
int StartParams::classID=START_PARAMS_MSG_TYPE;
int StopParams::classID=STOP_PARAMS_MSG_TYPE;
int LogMessage::classID=LOG_MSG_TYPE;
int MyThread::ParentStatusReport::classID=PARENT_REPORT_MSG;
*/

/////////////////////////////////////////////////////////////////////////////
// MyThread
IMPLEMENT_DYNCREATE(MyThread, CWinThread)

BEGIN_MESSAGE_MAP(MyThread, CWinThread)
	//{{AFX_MSG_MAP(VoltmeterThread)
	ON_THREAD_MESSAGE(UM_EXIT_THREAD,OnExitThread)	
	ON_THREAD_MESSAGE(UM_START,OnStart)			
	//}}AFX_MSG_MAP
	// Standard file based document commands	
END_MESSAGE_MAP()

//---------------------------------------------
MyThread::MyThread(): RunEvent(true,true)
{	
	TerminateEvent=0;  WAIT_TIME=1000; pass_In=pass_Out=0; 
	Config.ParentWindow=MainFrame;
	LastConfig=Config; 
}
MyThread::~MyThread() 
{
}

void MyThread::Create(CWinThread* parent,int ID, CEvent* evnt) 
{
	Parent=parent; Config.PersonalID=ID; TerminateEvent=evnt;	
}

BOOL MyThread::InitInstance()
{	
	Config.ThreadState=Resumed; Config.Terminate=None;
	m_bAutoDelete=true; Config.ThreadStatus=STOPED; Config.Stop=Done;		
	TerminateEvent->ResetEvent();
	return TRUE;
}

int MyThread::ExitInstance()
{
	Config.Terminate=Done;
	int t=CWinThread::ExitInstance();
	TerminateEvent->SetEvent();
	return t;
}

void MyThread::MySuspend()
{
	if(Config.ThreadState==Suspended) return;
	Config.ThreadState=Suspended;
	CWinThread::SuspendThread();
}

void MyThread::MyResume()
{
	if(Config.ThreadState==Resumed) return;
	Config.ThreadState=Resumed;
	CWinThread::ResumeThread();
}
//-------------------------------------------------

void MyThread::BeforeExecute(WPARAM wParam, LPARAM lParam)
{	
	RunEvent.ResetEvent();
	pass_In++;
	Config.ThreadStatus=RUNNING;  Config.Stop=None;	
	Timer1.Start(); 
	ErrorsArray* err=new ErrorsArray(); 
	AttachErrors(err);
	Warning* t=new Warning(); RegisterError(t->Create(WRN_THREAD_START,Config.Name));
	PostParentMessage(UM_START);	
}

void MyThread::AfterExecute()
{
	Timer1.Stop(); 
	Config.ThreadStatus=STOPED;  Config.Stop=Done;
	RunEvent.SetEvent();
	pass_Out++;		
	ms tt(Timer1.GetValue()); CString T; T.Format("%s (%g ms)",Config.Name,tt.val());
	Warning* t=new Warning(); t->Create(WRN_THREAD_STOP,T); RegisterError(t);
	ErrorsArray* err=DetachErrors(); 
	err->Msg=UM_STOP; err->Reciver=Config.ParentWindow; 
	ASSERT(err->Reciver.pThrd);
	ASSERT(err->Reciver.pWND);
	PostParentMessage(UM_GENERIC_MESSAGE,err);	
}

void MyThread::PauseThread()
{	
	Config.ThreadStatus=PAUSED;
	PostParentMessage(UM_GENERIC_MESSAGE,new MessageForWindow(UM_PAUSE,Config.ParentWindow));	
	MySuspend();	
}

void MyThread::ContinueThread()
{
	MyResume();
	Config.ThreadStatus=RUNNING;  Config.Stop=None;	
	PostParentMessage(UM_GENERIC_MESSAGE,new MessageForWindow(UM_CONTINUE,Config.ParentWindow));	
}


void MyThread::StopThread(bool Wait) 
{
	if(Config.ThreadStatus==STOPED) 
	{
		ErrorsArray* err=new ErrorsArray(); err->Msg=UM_STOP; err->Reciver=Config.ParentWindow;
		PostParentMessage(UM_GENERIC_MESSAGE,err);
		return;
	}	

	Config.Stop=ToDo; 	
	if(Wait)
	{
		MyResume();
		if(WaitForSingleObject(RunEvent,WAIT_TIME)!=WAIT_OBJECT_0) 
		{
			ErrorsArray* err=new ErrorsArray(); 
			ErrorInspector Inspector; Inspector.AttachErrors(err);
			Error* t=new Error(); Inspector.RegisterError(t->Create(ERR_THREAD_STOP,CString("Assistant")));
			err->Msg=UM_STOP; err->Reciver=Config.ParentWindow; 
			PostParentMessage(UM_GENERIC_MESSAGE,err);	
			Inspector.DetachErrors();
		}
	}
	else 
	{
		StopParams* params=new StopParams(this); RegisterMsg(params);
		AfxBeginThread(StopThreadFunction,params);
	}
}

UINT StopThreadFunction( LPVOID pParam )
{
	StopParams* params=(StopParams*)pParam; MyThread *T=params->T; T->MyResume(); 
	if(WaitForSingleObject(T->RunEvent,T->WAIT_TIME)!=WAIT_OBJECT_0) 
	{
		ErrorsArray* err=new ErrorsArray(); 
		ErrorInspector Inspector; Inspector.AttachErrors(err);
		Error* t=new Error(); Inspector.RegisterError(t->Create(ERR_THREAD_STOP,CString("Assistant")));
		err->Msg=UM_STOP; err->Reciver=T->Config.ParentWindow; 
		T->PostParentMessage(UM_GENERIC_MESSAGE,err);
		Inspector.DetachErrors();
	}
	delete params;
    return 0;
}

void MyThread::StartThread(WPARAM wParam,LPARAM lParam) 
{	
	StartParams *t=new StartParams(); t->Create(wParam,lParam,this); RegisterMsg(t);
	AfxBeginThread(StartThreadFunction,t);
}

UINT StartThreadFunction( LPVOID pParam )
{
	StartParams *P=(StartParams*)pParam; MyThread *T=(MyThread *)(P->dest);		

	if(WaitForSingleObject(T->RunEvent,T->WAIT_TIME)!=WAIT_OBJECT_0) 
	{
		ErrorsArray* err=new ErrorsArray(); T->RegisterMsg(err);
		ErrorInspector Inspector; Inspector.AttachErrors(err);
		Error* t=new Error(); Inspector.RegisterError(t->Create(ERR_THRD_ALRD_RUNNING,CString("Assistant")));
		
		MyThread::ParentStatusReport* rep=new MyThread::ParentStatusReport(MyThread::ThreadConfig()); T->RegisterMsg(rep);
		rep->ConfigCopy.PersonalID=AssistantID;

		P->src->PostThreadMessage(UM_START,(WPARAM)rep,(LPARAM)err);
	}
	else
	{		
		T->PostThreadMessage(UM_START,P->wParam,P->lParam);
	}
	delete P;
    return 0;
}

void MyThread::OnStart(WPARAM wParam, LPARAM lParam )
{	
	BeforeExecute(wParam,lParam);
	if(Errors()==0) Main(wParam, lParam);
	AfterExecute();		
}

void MyThread::OnExitThread(WPARAM wParam, LPARAM lParam ) 
{PostQuitMessage(Config.PersonalID);}

CString GetString(enum MyThread::_ThreadState a)
{
	CString T;
	switch (a)
	{
	case MyThread::Resumed: T="Resumed"; break;
	case MyThread::Suspended: T="Suspended"; break;	
	default: T="Unknown";
	}
	return T;
}

CString GetString(enum MyThread::_ThreadStatus a)
{
	CString T;
	switch (a)
	{
	case MyThread::STOPED: T="Stoped"; break;
	case MyThread::PAUSED: T="Paused"; break;
	case MyThread::RUNNING: T="Running"; break;
	case MyThread::THRD_ERROR: T="Error"; break;
	default: T="Unknown";
	}
	return T;
}

bool MyThread::PostParentMessage(UINT msg, MessagesInspectorSubject* lParam)
{
	if(Config.Terminate!=None) 
	{
		if(lParam!=NULL) delete lParam;
		return false;
	}
	int n=RegisterMsg(lParam);
	if(n>=REGISTER_OK || n==REGISTER_ERROR_NULL_MSG)
	{
		ParentStatusReport* rpt=GetParentReport();
		Parent->PostThreadMessage(msg,(WPARAM)rpt,(LPARAM)lParam); 
		return true;
	}
	else
	{
		if(lParam!=NULL) delete lParam;
		return false;
	}		
}

CString Error::Format() 
{
	CString ret,id,text;
	HINSTANCE hinst=AfxGetInstanceHandle();
	if(!(text.LoadString(hinst,Body_ID))) text="Неопознаная ошибка";
	ret.Format("[%02d:%02d:%02d] %s -> %s: %d", Time.GetHour(),Time.GetMinute(), Time.GetSecond(),Sender, text,Body_ID);	

	return ret;
};

int MyThread::RegisterMsg(MessagesInspectorSubject *msg)
{
	int ret=REGISTER_ERROR;
	if(Config.Terminate!=None)
	{
		ret=REGISTER_ERROR_THREAD_TERMINATE; return ret;
	}
	if(msg==NULL) 
	{
		ret=REGISTER_ERROR_NULL_MSG; return ret;
	}
	ret=REGISTER_OK;
	//ret=Inspector1.RegisterMsg(msg);
	return ret;
}

int MyThread::TerminateThread()
{
	int ret=0, WAIT_TIME=1000;	
	CEvent* TE=GetTerminateEvent(); 

	Config.Terminate=ToDo; 
	StopThread(true);	
	PostThreadMessage(UM_EXIT_THREAD,0,0);				
	if(WaitForSingleObject(*TE,WAIT_TIME)!=WAIT_OBJECT_0) ret=ERR_THREAD_TERMINATE;	
	return ret;
}

void MyThread::UpdateParent()
{
	if(Config.UpdateParent==true)
	{
//		PostParentMessage(UM_UPDATE_INDICATORS); 
	}
}

MyThread::ParentStatusReport* MyThread::GetParentReport()
{
	MyThread::ParentStatusReport* ret=NULL;
	if(Config.UpdateParent==true)
	{
		ret=new MyThread::ParentStatusReport(Config);
		switch (Config.ThreadStatus)
		{
		case PAUSED: ret->ConfigCopy.ActionDescription=Config.PauseComment;
		}
	}

	if(RegisterMsg(ret)<REGISTER_OK) {delete ret; ret=NULL;}
	return ret;
}

MyThread::ThreadConfig MyThread::ChangeConfig( ThreadConfig cfg )
{	
	LastConfig=Config; Config=cfg;
	UpdateParent();
	return LastConfig;
}

void MyThread::RestoreLastConfig(MyThread::ThreadConfig* cfg) 
{
	if(cfg!=NULL) Config=*cfg;
	else Config=LastConfig;
}

int ErrorsArray::Add( AbstractMessage* t )
{
	int ret=-1;
	if(t==NULL) return ret;
	switch(t->GetClass())
	{
	case ErrorMSG_TYPE:
	case ErrorResMSG_TYPE: err_num++; break;
	case WarningResMSG_TYPE: warn_num++; break;
	}
	ret=AbstractMessagesArray::Add(t);
	return ret;
}

ResourceMSG& ResourceMSG::operator=(const ResourceMSG& T)
{
	Time=T.Time; Body_ID=T.Body_ID; Sender=T.Sender; Subject=T.Subject; Body=T.Body;
	return *this;
}


ErrorInspector::~ErrorInspector() 
{
	ASSERT(ErrArr.IsEmpty());
}
void ErrorInspector::RegisterError(AbstractMessage* _err) 
{
	ErrorsArray* err=GetErrorsArray();
	ASSERT(err);
	if(err!=NULL) err->Add(_err);
	else delete _err;
}
int ErrorInspector::Errors() 
{
	ErrorsArray* err=GetErrorsArray();
	ASSERT(err);
	if (err != NULL)
	{
		return err->err_num;    
	}
	else
	{
		return -1;
	}    
}
int ErrorInspector::Warnings() 
{
	int ret=-1;
	ErrorsArray* err=GetErrorsArray();
	if(err!=NULL) ret=err->warn_num;	
	else ASSERT(0);
	return ret;    
}
ErrorsArray* ErrorInspector::GetErrorsArray() 
{
	ErrorsArray* ret;
    if(ErrArr.IsEmpty()) ret=0;
	else ret=ErrArr.GetTail();
	return ret;
}
void ErrorInspector::AttachErrorsOF(ErrorInspector* src) 
{	
	ErrorsArray* err;
	if(src==NULL) return;
	if((err=src->GetErrorsArray())==NULL) return;
	ErrArr.AddTail(err);
}
void ErrorInspector::AttachErrors(ErrorsArray* src)
{
	if(src!=NULL) ErrArr.AddTail(src);
}

ErrorsArray* ErrorInspector::DetachErrors()
{
	ErrorsArray* ret=NULL;
	if(!(ErrArr.IsEmpty())) ret=ErrArr.RemoveTail();
	ASSERT(ret);
	return ret;    
}

void ErrorInspector::ResetErrors() 
{
	ErrorsArray* err=GetErrorsArray();
	if(err!=NULL) err->RemoveAll();
}

int ErrorInspector::DispatchErrors()
{
	CWinThread* mapp=AfxGetApp();
	CWinThread* thrd=AfxGetThread();
	MyThread tempThread; tempThread.Create(mapp,0,0);
	MyThread *Sender=NULL;
	if(Errors()!=0 || Warnings()!=0)
	{		
		LogMessage* log=new LogMessage(); log->CreateEntry(GetErrorsArray()); 
		if(thrd!=mapp) Sender=(MyThread*)thrd;
		else Sender=&tempThread;
		Sender->PostParentMessage(UM_GENERIC_MESSAGE,log); 
	}
	return Errors();
}

//------------------------------------------------
//------------------------------------------------
LogMessage::LogMessage()
{
	priority=low_pr; Name="LogMessage"; ClassID=LOG_MSG_TYPE; Msg=UM_EVENTLOG; Reciver=EventsLog;
}
LogMessage* LogMessage::CreateEntry(const CString& obj, const CString Message,int _priority)
{
	if(priority!=high_pr) priority=_priority; 
	CTime Time1; Time1=CTime::GetCurrentTime();	
	CString T;
	T.Format("[%02d:%02d:%02d] %s:%s",Time1.GetHour(),Time1.GetMinute(), Time1.GetSecond(),obj,Message);	
	Msgs.Add(T);
	return this;
}
LogMessage* LogMessage::CreateEntry(ErrorsArray* Err)
{
	CTime Time1; Time1=CTime::GetCurrentTime();	
	CString T,T1, name; ErrorInspector Insp; Insp.AttachErrors(Err);
	if(Err==0) {priority=low_pr; return this;}
	T.Format("[%02d:%02d:%02d] %s--->%s ",
		Time1.GetHour(),Time1.GetMinute(), Time1.GetSecond(),Err->Name,"Report");	

	if(Err)
	{
		T1.Format("[E/W: %d/%d]",Insp.Errors(),Insp.Warnings()); Msgs.Add(T+T1);            		
		for(int i=0;i<Err->GetSize(); i++) 
		{
			AbstractMessage* err=(AbstractMessage*)(*Err)[i];
			{
				Msgs.Add(err->Format());
			}
		}
		if(Insp.Errors()!=0) priority=high_pr;
		else priority=low_pr;
	}
	else 
	{
		T1=CString("Errors list empty!?"); Msgs.Add(T+T1);   
	}
	Msgs.Add(" "); 
	Insp.DetachErrors();
	return this;
}



MyThread::ParentStatusReport::ParentStatusReport(ThreadConfig& Config)
{
	Name="ParentStatusReport"; ClassID=PARENT_REPORT_MSG; 
	ConfigCopy.PersonalID=Config.PersonalID; 
	ConfigCopy.Name=Config.Name; 
	ConfigCopy.ActionDescription=Config.ActionDescription;
}

MyThread::ThreadConfig& MyThread::ThreadConfig::operator=(ThreadConfig& newcfg)
{
	PersonalID=newcfg.PersonalID;
	Name=newcfg.Name;
	UpdateParent=newcfg.UpdateParent;
	ActionDescription=newcfg.ActionDescription;
	PauseComment=newcfg.PauseComment;
	ParentWindow=newcfg.ParentWindow;
	return *this;
}

MyThread::ThreadConfig::ThreadConfig()
{
	ThreadState=Suspended; 
	ThreadStatus=STOPED; Stop=None;	Terminate=None;
	PersonalID=-1; 
	Name="MyThread"; 
	UpdateParent=true; NotifyUser=false;
	ActionDescription=CString("Я ничего не делаю");
	PauseComment=CString("Ну вот, на самом интересном месте");
}
