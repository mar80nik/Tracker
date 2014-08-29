#include <stdafx.h>
#include "ProtectedObject.h"


int Guardian::IncreaseReaders() 
{
	readers++; 
	WaitForReaders.ResetEvent(); 
	return readers;
}
int Guardian::DecreaseReaders() 
{
	readers--; 
	ASSERT(readers>=0);
	if(readers==0) WaitForReaders.SetEvent(); 
	return readers;
}
int Guardian::IncreaseWriters() 
{
	writers++; 
	return writers;
}
int Guardian::DecreaseWriters() 
{
	writers--; 
	ASSERT(writers>=0);
	if(writers==0) 
		ReadAcsess.SetEvent(); 
	return writers;
}
Guardian::Guardian():ReadAcsess(false,true), WaitForReaders(false,true)
{
	WAIT_TIME=1000; readers=writers=0;
	ReadAcsess.SetEvent(); WaitForReaders.SetEvent();
};
Guardian::~Guardian() 
{
	WaitForSingleObject(WaitForReaders,WAIT_TIME);
};
