// MessageContainers.cpp: implementation of the MessageContainers class.
//
//////////////////////////////////////////////////////////////////////

#include <stdafx.h>
#include "MessageInspector.h"
#include "MyThread.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
int MessagesInspectorSubject::PIDnum=0;

MessagesInspectorSubject::MessagesInspectorSubject() 
{
	Prev=Next=0; Inspector=0; Name="Noname";PID=PIDnum++; ClassID=-1;
}
MessagesInspectorSubject::~MessagesInspectorSubject() 
{
	int ret=-1; void *x;
	if(Inspector)
	{	
		if((x=Inspector->counter.GainAcsess(WRITE))!=0)
		{
			ProtectorX<int> i_guard(x); int &t(i_guard);
			if(t!=0)
			{
				t--; ret=t;
				if(Prev) 
				{
					ASSERT(Prev->Next==this);
					Prev->Next=Next;
				}
				if(Next) 
				{
					ASSERT(Next->Prev==this);
					Next->Prev=Prev;	
				}
			}	
			else ASSERT(0);
		}
	}
}		

BOOL MessagesInspectorSubject::ValidateClass(int classID) { return (ClassID==classID); }
//////////////////////////////////////////////////////////////////////////
MessagesInspector::MessagesInspector() 
{	
	Name="Noname"; counter.Set(0);
};

int MessagesInspector::RegisterMsg(MessagesInspectorSubject *msg)
{
	int ret=-1; void *x;
	MessagesInspectorSubject* Null=&NullMsg, *First;
	ASSERT(msg->Inspector==NULL); 
	msg->Inspector=this;

	if((x=counter.GainAcsess(WRITE))!=0)
	{
		ProtectorX<int> i_guard(x); int &t(i_guard);
		t++; ret=t; 
		First=Null->Next;
		Null->Next=msg; msg->Prev=Null;
		if(First) First->Prev=msg; 
		msg->Next=First;			
	}
	return ret;
}

int MessagesInspector::CleanUp()
{
	MessagesInspectorSubject *msg=NullMsg.Next,*t; int ret=0;		
	while(msg)
	{
		t=msg;            
		delete t; ret++;
		msg=NullMsg.Next;		
	}	
	return ret;
}

MessagesInspector::~MessagesInspector()
{
	CleanUp();
}
//////////////////////////////////////////////////////////////////////////
CString AbstractMessage::Format() 
{
	CString ret;
	ret.Format("[%02d:%02d:%02d] %s -> %s: %d", Time.GetHour(),Time.GetMinute(), Time.GetSecond(),Sender, Body);	
	return ret;
};

void MessageForWindow::Dispatch()
{
	DispatchMsg(this);
}

void MessageForWindow::DispatchMsg( void *t )
{
	ASSERT(Msg);
	ASSERT(Reciver.pThrd);
	ASSERT(Reciver.pWND);
	CWinThread *thrd=AfxGetThread();
	if(thrd==Reciver.pThrd)
	{
		Reciver.pWND->PostMessage(Msg,wParam,(LPARAM)t);
	}
	else Reciver.pThrd->PostThreadMessage(UM_GENERIC_MESSAGE,wParam,(LPARAM)t);
}
