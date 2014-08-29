#pragma once
#ifndef SystemConfigH
#define SystemConfigH

#include "MyThread.h"
//#include "globalheader.h"
//#include "newwavelen.h"
#include "devices.h"
#include "metricon.h"

enum {CFG_dV, CFG_SENSOR_STATUS, CFG_SENSOR_JITTER, CFG_MESUREMENT_NUM, CFG_MESUREMENT_TIME, CFG_GRATING_COEF,
	CFG_FILTER,CFG_FILTER_POS,CFG_FILTER_MODE, CFG_PGA_INVERT, CFG_ADC_INVERT};
enum {CFG_DISABLE, CFG_ENABLE};

class SystemConfig; 
/*
struct SimpleCoordinates
{
	us10 t; fm10 l; int st; 

	SimpleCoordinates(void) {st=0;} 
	SimpleCoordinates(fm10 _l,int _st, us10 _t) {l=_l; st=_st, t=_t;}
	virtual ~SimpleCoordinates() {}
	void Reset() {t=us10(); l=fm10(); st=0;}
	SimpleCoordinates& operator +=(SimpleCoordinates& src) {t+=src.t; l+=src.l; st+=src.st; return (*this);}
	SimpleCoordinates operator +(SimpleCoordinates& src) 
		{SimpleCoordinates ret; ret.t=t+src.t; ret.l=l+src.l; ret.st=st+src.st; return ret;}
	SimpleCoordinates operator *(int& n) 
		{SimpleCoordinates ret; ret.t=t*n; ret.l=l*n; ret.st=st*n; return ret;}
	SimpleCoordinates operator *(double d) 
		{SimpleCoordinates ret; ret.t=t*d; ret.l=l*d; ret.st=(int)(st*d); return ret;}
};

typedef SimpleCoordinates LStT;
typedef SimpleCoordinates StageInterval;
typedef SimpleCoordinates ScenarioInterval;
*/
//****************************************
//****************************************
class SystemConfig: public ErrorInspector
{
	friend class TConfigDialog;
	int WAIT_TIME;
protected:
	CMutex SystemConfigUpd;
	CString Name;

	int RegisterDevice(AbstractDevice* t);
	DevicesArray Devices;
public:
    CWinApp* Parent;
//	SDR MotorR; SDRSensor MotorS; SDA MotorA; AD7887 ADC1; mdr23 MDR23; filterHead FilterHead;
//	GenericDevice Generic; PGA PGAChnl0, PGAChnl1;
	CalibrationData cal;

	SystemConfig();   
	~SystemConfig();
	void LoadConfig(); void SaveConfig();
	virtual int Serialize(CArchive &ar);
//	bool SetWavelength(nmC val);
//	bool IncrementWavelength(nmC val);
	bool SetFilterPos(BYTE val);
//	nmC GetWavelength();
	void Get(int,int&); void Get(int, double&); void Get(int, BYTE&);
	void Get(int,sec&); //void Get(int,FilterHeadPos&);
	void DeviceUpdate(AbstractDevice *d,CfgParamTypeBase* p);
//	BYTE GetPGAnum(PGA*);
	void SetCalibration(CalibrationParams &cal);
	void GetCalibration(CalibrationParams*);
};

#endif
