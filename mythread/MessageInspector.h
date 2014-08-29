// MessageContainers.h: interface for the MessageContainers class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MESSAGECONTAINERS_H__12619758_5BBC_4889_86E0_A56158C60436__INCLUDED_)
#define AFX_MESSAGECONTAINERS_H__12619758_5BBC_4889_86E0_A56158C60436__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <afxtempl.h>
#include <afxmt.h>
#include "ProtectedObject.h"
#include "GlobalHeader.h"

enum {MSG_ARRAY_TYPE=0xabcdeb,GENERIC_MSG_MSG_TYPE, WINDOW_MSG_TYPE};

#define REGISTER_OK 0
#define REGISTER_ERROR -1;
#define REGISTER_ERROR_THREAD_TERMINATE -2
#define REGISTER_ERROR_NULL_MSG -3

struct WindowAddress
{
	CWinThread	*pThrd;
	CWnd		*pWND;
	WindowAddress() {pThrd=AfxGetThread();;pWND=NULL;}
	bool operator==(WindowAddress& t) { return (pWND==t.pWND && pThrd==t.pThrd); }
};

class MessagesInspectorSubject
{
	friend class MessagesInspector;
protected:
	static int PIDnum; int PID;
	int ClassID; 
	MessagesInspectorSubject* Prev,*Next;
	MessagesInspector* Inspector;
public:
	CString Name; 
	static MessagesInspector* GlobalInspector;
protected:
	MessagesInspectorSubject();
public:
	virtual ~MessagesInspectorSubject();		
	virtual MessagesInspectorSubject& operator=(MessagesInspectorSubject&) {return (*this);}
	int GetPID() {return PID;}
	virtual BOOL ValidateClass(int classID);
	int GetClass() {return ClassID;};
	BOOL IsRegistered() {return (Inspector!=NULL);}
};

//---------------------------------------
class MessagesInspector
{	
	MessagesInspectorSubject NullMsg;	
public:
	CString Name;
	ProtectedObjectX<int> counter; 

	MessagesInspector();
	int RegisterMsg(MessagesInspectorSubject *);
	int CleanUp();
	~MessagesInspector();;
};
//---------------------------------------
class AbstractMessage: public MessagesInspectorSubject
{
public:
	CTime			Time; 
	CString			Sender, Subject, Body;
	AbstractMessage() 
	{
		Time=CTime::GetCurrentTime();
		GlobalInspector->RegisterMsg(this);
	}
	virtual ~AbstractMessage() 
	{
		int a=5; a++;
	}
	virtual CString Format();
};

class MessageForWindow: public AbstractMessage
{
protected:
	void DispatchMsg(void *t);
public:
	WindowAddress	Reciver;
	UINT			Msg;
	WPARAM			wParam;

	MessageForWindow() {Msg=0; wParam=0; ClassID=WINDOW_MSG_TYPE;}
	MessageForWindow(UINT m, WindowAddress r) {Msg=m; Reciver=r;}
	virtual ~MessageForWindow() 
	{
		int a=5; a++;
	};
	virtual void Dispatch();
	bool operator==(MessageForWindow& t) { return (Msg==t.Msg && Reciver==t.Reciver ); }
};

class AbstractMessagesArray: public CArray<AbstractMessage*> 
{
protected:
	AbstractMessagesArray() {}
	virtual ~AbstractMessagesArray() {RemoveAll();}
	virtual void RemoveAll() 
	{
		for(int i=0;i<GetSize();i++) delete operator[](i);
		CArray<AbstractMessage*>::RemoveAll();
	}
};

class UniformMessagesArray: public MessageForWindow, public AbstractMessagesArray
{
public:
	UniformMessagesArray() {Name="MessagesArray"; ClassID=MSG_ARRAY_TYPE; }
	virtual ~UniformMessagesArray() {RemoveAll();};
	virtual int Add(MessageForWindow* t)
	{
		int ret=-1;
		ASSERT(t->ValidateClass(WINDOW_MSG_TYPE));		
		if(GetSize()==0) 
		{
			(*(MessageForWindow*)this)=(*t); 
			ret=AbstractMessagesArray::Add(t);
		}
		else
		{
            if( *t==*this )
			{
				ret=AbstractMessagesArray::Add(t);
			}
			else ASSERT(0);
		}
		return ret;
	}
	UniformMessagesArray& operator << (MessageForWindow* t) {Add(t); return *this;}
	virtual void RemoveAll() 
	{
		for(int i=0;i<GetSize();i++) delete operator[](i);
        Msg=0; Reciver=WindowAddress(); wParam=0;
	}
};



#endif // !defined(AFX_MESSAGECONTAINERS_H__12619758_5BBC_4889_86E0_A56158C60436__INCLUDED_)
