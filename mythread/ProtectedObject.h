#pragma once


#include <afxmt.h>
#include "mytime.h"

enum AcsessTypes {NONE=0,WRITE, READ,READ_EX};

template <typename T> class ProtectorX;

class Guardian
{
	template <typename T> friend class ProtectorX;
protected:
	CMutex WriteAcsess;
	int WAIT_TIME;

	int readers, writers;
	CEvent ReadAcsess, WaitForReaders;

	int IncreaseReaders(); int DecreaseReaders();
	int IncreaseWriters(); int DecreaseWriters();
public:
	Guardian(); ~Guardian();
};

template <typename T>
class ProtectedObjectX: public Guardian
{
	int modifID;	
protected:
	T obj; MyTimer GainTimer; sec time;
public:
	ProtectedObjectX() {modifID=0;};
	~ProtectedObjectX() {}
	ProtectorX<T>* GainAcsess(AcsessTypes type)
	{
		ProtectorX<T> *p=NULL;
		switch(type)
		{
		case READ:
		case READ_EX:	
			GainTimer.Start();
			if(WaitForSingleObject(ReadAcsess,WAIT_TIME)==WAIT_OBJECT_0)
			{
				p=new ProtectorX<T>(NULL); GainTimer.Stop(); time=GainTimer.GetValue();
				IncreaseReaders(); p->data=&obj; p->type=type; p->Parent=this; 
			}
			else ASSERT(0);
			break;
		case WRITE:
			GainTimer.Start(); ReadAcsess.ResetEvent();		
			if(WaitForSingleObject(WaitForReaders,WAIT_TIME)==WAIT_OBJECT_0)
			{
				p=new ProtectorX<T>(NULL); 
				p->Lock=new CSingleLock(&WriteAcsess);
				if(p->Lock->Lock(WAIT_TIME))
				{
					GainTimer.Stop(); time=GainTimer.GetValue();
					IncreaseWriters(); p->data=&obj; p->type=type; modifID++; p->Parent=this; 
				}
				else 
				{
					delete p; p=0;
				}
			}
			if(p==NULL) 
				ReadAcsess.SetEvent();
			break;
		}
		return p;
	}	
    int GetModificationID() {return modifID;}
	bool Set(T d)
	{
		void *x; bool ret=false;
		if((x=GainAcsess(WRITE))!=NULL)
		{
			ProtectorX<T> guard(x); *guard.data=d; ret=true;
		}	
		return ret;
	}
	bool Get(T& d)
	{
		void *x; bool ret=false;
		if((x=GainAcsess(READ))!=NULL)
		{
			ProtectorX<T> guard(x); d=*guard.data; ret=true;
		}	
		return ret;
	}

};



template <typename T>
class ProtectorX
{
	friend class Guardian;
	template <typename T> friend class ProtectedObjectX;
protected:
	MyTimer AcsessTimer;

	AcsessTypes type;
	Guardian *Parent;	
	CSingleLock* Lock;
public:
    T *data;    

	ProtectorX(void* _t) 
	{
		ProtectorX* t=(ProtectorX*)_t;
		Lock=0; data=0; type=NONE; Parent=0;
		if(t!=NULL)
		{
			Lock=t->Lock; data=t->data; type=t->type; Parent=t->Parent;
			t->Lock=0; t->type=NONE; AcsessTimer.Start();
			delete t;
		}
	};
	operator T&() 
	{
		return *data;
	}
	ProtectorX& operator=(T& t) {*data=t; return (*this);}
	~ProtectorX() 
	{
		Unlock();
		AcsessTimer.Stop();
	}
	void Unlock()
	{
		switch(type)
		{
		case READ:
		case READ_EX: 
			Parent->DecreaseReaders(); break;
		case WRITE: 
			if(Lock)
			{
				delete Lock; 
				Parent->DecreaseWriters(); 
			}
			break;	
		}		
	}
};