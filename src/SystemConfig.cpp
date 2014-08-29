#include "stdafx.h"
#include "SystemConfig.h"
#include "resource.h"
#include <math.h>

#define SpN (Coeff1*StepPerNanometer)


int SystemConfig::RegisterDevice(AbstractDevice* t) 
{
	int ret=Devices.Add(t); t->id=(WORD)(ret+1); t->Parent=this; 
	Name=CString("SystemConfig");
	return ret;
}

SystemConfig::SystemConfig()  
{
	WAIT_TIME=1000; 

//	PGA::PGAGainValues << 1 << 2 << 5 << 10 << 20 << 50 << 100 << 200;
//	PGA::ModeValues << 0 << 1 << 12 << 13 << 14;
	
//	RegisterDevice(&MotorR); RegisterDevice(&MotorS); RegisterDevice(&MotorA); 
//	RegisterDevice(&ADC1); 
//	RegisterDevice(&MDR23);  RegisterDevice(&FilterHead);
//	RegisterDevice(&Generic); 
	RegisterDevice(&cal);

//	PGAChnl0.PGAnum=0; PGAChnl1.PGAnum=1;
}

SystemConfig::~SystemConfig() {}

void SystemConfig::LoadConfig()
{
	CFile ConfigFile; ErrorsArray err; AttachErrors(&err); 

	if(ConfigFile.Open("config.dat",CFile::modeRead))
	{
		CArchive ar(&ConfigFile,CArchive::load); 
		Serialize(ar); ar.Close();		
		ConfigFile.Close();
	}
	else 
	{
		Error* t=new Error(); RegisterError(t->Create(ERR_CONFIG_NOFILE,Name));
	}
	Parent->PostThreadMessage(UM_WAVELEN_SET,0,0);
	Parent->PostThreadMessage(UM_UPDATE_CONFIG,0,0);

	DispatchErrors();
	DetachErrors();
}

void SystemConfig::SaveConfig()
{
	CFile ConfigFile;
	if(ConfigFile.Open("config.dat",CFile::modeCreate | CFile::modeWrite ))
	{
		{CArchive ar(&ConfigFile,CArchive::store); Serialize(ar);}
		ConfigFile.Close(); 
	}	
}

int SystemConfig::Serialize(CArchive &ar)
{
	int ret=0;
	CSingleLock Lock1(&SystemConfigUpd); 
	if(Lock1.Lock(WAIT_TIME))
	{
		try
		{
		for(int i=0;i<Devices.GetSize();i++) Devices[i]->Serialize(ar);
		}
		catch(CArchiveException *ex)
		{
			ex=NULL;
			Error* t=new Error(); RegisterError(t->Create(ERR_CONFIG_ARCHERR,Name));
		}
		catch(CFileException *ex)
		{
			ex=NULL;
			Error* t=new Error(); RegisterError(t->Create(ERR_CONFIG_FILEERR,Name));
		}
		catch(CMemoryException *ex)
		{			
			ex=NULL;
			Error* t=new Error(); RegisterError(t->Create(ERR_CONFIG_MEMERR,Name));
		}
	}
	return ret;
}

void SystemConfig::Get(int mode, int& val)
{
	switch(mode)
	{
//	case CFG_PGA_INVERT: val=Generic.ExchangePGAsControl; break;
//	case CFG_ADC_INVERT: val=Generic.ExchangeADCChnls; break;
//	case CFG_SENSOR_STATUS: val=Generic.SensorStatus; break;
//	case CFG_SENSOR_JITTER: val=MotorS.Sensor1.Jitter; break;
//	case CFG_MESUREMENT_NUM: val=ADC1.num_mes; break;
//	case CFG_GRATING_COEF: val=MDR23.GratingCoef; break;	
	default: ASSERT(0);
	}
	return;
}

void SystemConfig::Get(int mode, BYTE& val)
{
	switch(mode)
	{
//	case CFG_FILTER_MODE: val=FilterHead.OperationMode; break;
	default: ASSERT(0);
	}
	return;
}


void SystemConfig::Get(int mode, double& val)
{
	switch(mode)
	{
//	case CFG_dV: val=ADC1.AVCC/(1<<ADC1.Digits);	break;
	default: ASSERT(0);
	}
	return;
}

void SystemConfig::Get( int mode,sec& val)
{
	switch(mode)
	{
//	case CFG_MESUREMENT_TIME: val=ADC1.time_mes; break;
	default: ASSERT(0);
	}
	return;
}
/*
void SystemConfig::Get( int mode,FilterHeadPos& val)
{
	switch(mode)
	{
//	case CFG_FILTER_POS: val=FilterHead.RevolvHeadPos; break;
	default: ASSERT(0);
	}
	return;
}

bool SystemConfig::SetWavelength(nmC val)
{
	bool ret=false;

	if(MDR23.CheckWavelength(val))
	{
		if(MDR23.CurWaveLen!=val)
		{
			MDR23.CurWaveLen=val;
			SaveConfig(); ret=true; Parent->PostThreadMessage(UM_WAVELEN_SET,0,0);
		}
	}
	else
	{
		if(GetErrorsArray()==NULL)
		{
			ErrorsArray err; AttachErrors(&err); 
			Error* t=new Error(); RegisterError(t->Create(ERR_L_OUTOFRANGE,Name));
			DispatchErrors();
			DetachErrors();
		}
		else
		{
			Error* t=new Error(); RegisterError(t->Create(ERR_L_OUTOFRANGE,Name));
		}
	}
	return ret;
}
*/
//nmC SystemConfig::GetWavelength() {return MDR23.CurWaveLen;}

void SystemConfig::DeviceUpdate( AbstractDevice *d,CfgParamTypeBase* p )
{
	Parent->PostThreadMessage(UM_UPDATE_CONFIG,(WPARAM)d,LPARAM(p));	
}
/*
bool SystemConfig::SetFilterPos( BYTE val )
{
	bool ret=false; FilterHeadPos t(val);
	if(val>=0 && val<FILTER_MAX)
	{
		if(t!=FilterHead.RevolvHeadPos)
		{
			FilterHead.RevolvHeadPos.SetParamValue(t); 
			ret=true;
		}		
	}
	return ret;
}

BYTE SystemConfig::GetPGAnum( PGA* p)
{
	BYTE ret=0xff;
	if(Generic.ExchangePGAsControl)
	{
		switch (p->PGAnum)
		{
		case 0: ret=1; break;
		case 1: ret=0; break;
		default: ASSERT(0);
		}
	}
	else ret=p->PGAnum;
	return ret;
}

bool SystemConfig::IncrementWavelength( nmC val )
{
	bool ret=false; nmC tl;
	tl=MDR23.CurWaveLen+val;

	if(MDR23.CheckWavelength(tl))
	{
		if(MDR23.CurWaveLen!=tl)
		{
			MDR23.CurWaveLen=tl;
			SaveConfig(); ret=true; Parent->PostThreadMessage(UM_WAVELEN_SET,0,0);
		}
	}
	else
	{
		if(GetErrorsArray()==NULL)
		{
			ErrorsArray err; AttachErrors(&err); 
			Error* t=new Error(); RegisterError(t->Create(ERR_L_OUTOFRANGE,Name));
			DispatchErrors();
			DetachErrors();
		}
		else
		{
			Error* t=new Error(); RegisterError(t->Create(ERR_L_OUTOFRANGE,Name));
		}
	}
	return ret;
}
*/
void SystemConfig::SetCalibration( CalibrationParams &_cal )
{
	cal.SetCalibration(_cal);
}

void SystemConfig::GetCalibration(CalibrationParams* tt)
{
	*tt=cal.cal;
}
