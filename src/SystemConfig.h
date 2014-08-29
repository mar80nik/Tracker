#pragma once
#ifndef SystemConfigH
#define SystemConfigH

#include "MyThread.h"
#include "devices.h"
#include "metricon.h"

enum {CFG_dV, CFG_SENSOR_STATUS, CFG_SENSOR_JITTER, CFG_MESUREMENT_NUM, CFG_MESUREMENT_TIME, CFG_GRATING_COEF,
	CFG_FILTER,CFG_FILTER_POS,CFG_FILTER_MODE, CFG_PGA_INVERT, CFG_ADC_INVERT};
enum {CFG_DISABLE, CFG_ENABLE};

class SystemConfig; 

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
	CalibrationData cal;

	SystemConfig();   
	~SystemConfig();
	void LoadConfig(); void SaveConfig();
	virtual int Serialize(CArchive &ar);
	void Get(int,int&); void Get(int, double&); void Get(int, BYTE&);
	void DeviceUpdate(AbstractDevice *d,CfgParamTypeBase* p);
	void SetCalibration(CalibrationParams &cal);
	void GetCalibration(CalibrationParams*);
};

#endif
