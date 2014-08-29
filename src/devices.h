#pragma once

//#include "newwavelen.h"
#include "mythread.h"
//#include "globalheader.h"
#include "metricon.h"

class SystemConfig; class AbstractDevice;
enum {DEV_ERR=0, CFG_PARAM_ERR=0};

union TreeParamData
{
	struct {WORD dev,param;};
	DWORD data;
	explicit TreeParamData(WORD _dev, WORD _param) {dev=_dev; param=_param;}
	explicit TreeParamData(DWORD t) {data=t;}
	operator DWORD() {return data;}
};

struct CfgParamTypeBase
{
	int pid; bool IsEditable;
	CString Name, Units;
	AbstractDevice* Parent;
	HTREEITEM TreeItem;

	CfgParamTypeBase() {pid=-1; IsEditable=false; Name="NoName"; Units="NoUnits";TreeItem=0;}
	virtual void GetParamValue(CString& t)=0;
	virtual void FormatParam(CString& t)=0;
	virtual void SetParamValue(CString& t)=0;
	virtual void GetParamTitle(CString& t) {t=(Units=="" ? Name: Name+" ["+Units+"]");};
	virtual void Serialize(CArchive& ar)=0;
	virtual void UpdateParent();
	virtual void FillCfgTree(CTreeCtrl* t,HTREEITEM l0);
};

template<typename T>
class CfgParamType: public CfgParamTypeBase
{
protected:
	T val;
public:
	CfgParamType(CString n, CString u, T InitValue) {Name=n; Units=u; val=InitValue;} 
	operator T() {return val;};
	CfgParamType& operator=(T t) {val=t; return *this;};
	virtual void GetParamValue(CString& t);
	virtual void FormatParam(CString& t)
	{
		CString T1; t=""; GetParamTitle(T1); t+=T1+" = "; GetParamValue(T1); t+=T1;
	}
	virtual void SetParamValue(CString& t);	
	virtual void SetParamValue(T t) {val=t;UpdateParent();}	
	virtual void Serialize(CArchive& ar);
};

class CfgCalibrationParams: public CfgParamTypeBase
{
protected:
	CalibrationParams val;
public:
	CfgCalibrationParams() {} 
	virtual void GetParamValue(CString& t) {};
	virtual void FormatParam(CString& t) {};
	virtual void SetParamValue(CString& t) {};
	virtual void GetParamTitle(CString& t) {};
	virtual void Serialize(CArchive& ar);
	virtual void FillCfgTree(CTreeCtrl* t,HTREEITEM l0);
};

//struct FilterHeadPos;

typedef CArray<CfgParamTypeBase*> ParamsArray;

typedef CfgParamType<bool> YesNoCfgParam;
typedef CfgParamType<BYTE> ByteCfgParam;
typedef CfgParamType<int> IntCfgParam;
typedef CfgParamType<double> DblCfgParam;
typedef CfgParamType<CString> StrCfgParam;
//typedef CfgParamType<pm> wPicaCfgParam;
//typedef CfgParamType<sNanoSec> sNanoSecCfgParam;
//typedef CfgParamType<nmC> wCNanoCfgParam;
//typedef CfgParamType<ms> msCfgParam;
//typedef CfgParamType<sec> secCfgParam;
//typedef CfgParamType<FilterHeadPos> FilterPosCfgParam;


typedef TreeParamData TPD;

class SystemConfig;

class AbstractDevice: public ErrorInspector
{
	friend class TConfigDialog;
	friend class SystemConfig;
	friend struct CfgParamTypeBase;
protected:
	WORD id;
    CString Name;
	SystemConfig* Parent;
	ParamsArray Params;

	virtual void ParamUpdate(CfgParamTypeBase* t);
	int RegisterParam(CfgParamTypeBase* t);
public:
	AbstractDevice(void) {Name="NoName"; id=DEV_ERR;}
	virtual void Serialize(CArchive& ar);
	virtual int FormatParam(int id,CString& T);
	virtual void FillCfgTree(CTreeCtrl* t,HTREEITEM l0);
	void FillCfgTreeSingleParam(CTreeCtrl* t,HTREEITEM l0, CfgParamTypeBase* t2);
};

typedef CArray<AbstractDevice*> DevicesArray;

#define CALIBRATION_MODES_NUM 4
class CalibrationData: public AbstractDevice
{
	friend class SystemConfig;
protected:	
	DblCfgParam fi,d,N,L;
	DblCfgParam* N_exp[4], *teta[4];
	
	CalibrationParams cal;
public:
	CalibrationData(void);
	virtual ~CalibrationData();
	virtual void FillCfgTree(CTreeCtrl* t,HTREEITEM l0);
	virtual void Serialize(CArchive& ar);
	void SetCalibration(CalibrationParams& cal);
};
/*
class ADC: public AbstractDevice
{
	friend class SystemConfig;
protected:
	IntCfgParam num_mes; secCfgParam time_mes; 
	DblCfgParam AVCC; ByteCfgParam Digits;
public:
	ADC(void);
};

class AD7887: public ADC
{
public:
	AD7887(void): ADC() {Name="AD7887";}    
};

class StepMotor: public AbstractDevice
{
	friend class TConfigDialog;
public:
	struct Params
	{
		char dir;
		unsigned int st;
		us10 period;
	};
	enum Direction {MOTOR_BACKWARD=-1, MOTOR_FORWARD=1};

	struct SetupParams: public AbstractSetupParams
	{
		int st;
		us10 period;
	};

protected:
	YesNoCfgParam invert;
public:
	StepMotor(void);
	virtual Params Setup(AbstractSetupParams*);
	void RoundToStep(fm10& _l1);
	pm virtual GetStep() {return pm(0);};
	sNanoSec virtual GetSpeed() {return sNanoSec(0);};	
	bool GetInvert();
};

class SDR: public StepMotor
{
	friend class SystemConfig;
	friend class TConfigDialog;
public:
	struct SetupParams: public StepMotor::SetupParams
	{
		fm10 dl;
		SetupParams(fm10 _dl): dl(_dl)  {}
	};
protected:
	wPicaCfgParam step; sNanoSecCfgParam Speed;	
public:
	SDR(void);
	pm virtual GetStep();
	virtual Params Setup(AbstractSetupParams*);
	sNanoSec virtual GetSpeed();	
};

struct Sensor 
{
	wPicaCfgParam step; IntCfgParam Jitter;
	Sensor();
};

class SDRSensor: public SDR
{
	friend class SystemConfig;
protected:
	Sensor Sensor1;
public:
	SDRSensor(void);
	pm virtual GetStep();
};


enum FiltersCodes {ERR=-1, CUR_POS, EMPTY, SOLID, UV,VBL,NIR,IR};

struct FilterHeadPos
{
protected:
	char pos;
public:
	FilterHeadPos(int _p=ERR) {pos=(char)_p;};
	FilterHeadPos(FiltersCodes code);
	operator char();
	void operator +=(char i) {pos=pos+i;}
	BOOL operator !=(FilterHeadPos t) {return (pos!=t.pos);}
	virtual void Serialize(CArchive &ar)
	{
		if (ar.IsStoring()) ar << pos;
		else ar >> pos;
	}	
};

class AbstractHeadElement
{
protected:
	int pos;
	FiltersCodes code;		
public:
	AbstractHeadElement(int p=-1) {pos=p; code=ERR;}
	virtual bool CheckWithin(nm) {return false;}
	virtual ~AbstractHeadElement() {}
	int GetPos() {return pos;}
	int GetCode() {return code;}
};

class EmptyHeadPos: public AbstractHeadElement
{
public:
	EmptyHeadPos(int p=-1): AbstractHeadElement(p) {code=EMPTY;}
	virtual ~EmptyHeadPos() {}
	virtual bool CheckWithin(nm) {return true;}
};

class SolidHeadPos: public AbstractHeadElement
{
public:
	SolidHeadPos(int p=-1): AbstractHeadElement(p) {code=SOLID;}
	virtual ~SolidHeadPos() {}
};

class AbFilter: public AbstractHeadElement
{
protected:
	nm lmin,lmax;	
public:
	AbFilter(nm, nm, int p=-1);
	AbFilter(FiltersCodes, int p=-1);
	virtual bool CheckWithin(nm);
	virtual ~AbFilter() {}
};

typedef CArray<AbstractHeadElement*,AbstractHeadElement*> filtersArray;

enum {FILTER_ERR, FILTER_OK, FILTER_MAX=8, FILTER_MANUAL=0, FILTER_AUTOMATIC=1};

class filterHead: public AbstractDevice
{
	friend class SystemConfig;
	friend struct FilterHeadPos;
protected:
	filtersArray arr;
	FilterPosCfgParam  RevolvHeadPos; YesNoCfgParam OperationMode;
public:
	
	filterHead();
	virtual ~filterHead();
	int FilterForWavelen(nm w, FilterHeadPos init_pos=CUR_POS);
	FilterHeadPos FindFilter(nm w, FilterHeadPos init_pos=CUR_POS);
};


class SDA: public StepMotor
{
	friend class SystemConfig;
public:
	struct SetupParams: public StepMotor::SetupParams
	{
		BYTE end, beg;
		SetupParams(BYTE _e,BYTE _b) {beg=_b; end=_e;}
	};
protected:
	IntCfgParam StepsPerFase; msCfgParam Period;
public:
	SDA(void);
	virtual StepMotor::Params Setup(AbstractSetupParams*);
	int GetStepsPerFase() {return StepsPerFase;}
};

class mdr23: public AbstractDevice
{
	friend class SystemConfig;
protected:
	nmC CurWaveLen; 
	wCNanoCfgParam CounterMax, CounterMin;
	IntCfgParam GratingCoef;
public:
	mdr23(void);

	bool CheckWavelength(nmC val);
	virtual void Serialize(CArchive& ar) {AbstractDevice::Serialize(ar); CurWaveLen.Serialize(ar);}	
};
	
class GenericDevice: public AbstractDevice
{
	friend class SystemConfig;
protected:
	YesNoCfgParam SensorStatus, ExchangePGAsControl, ExchangeADCChnls, AutoGain, GainAdjust;
public:
	GenericDevice();
};
*/
class ByteArray: public CByteArray
{
public:
	ByteArray& operator << (BYTE t) {Add(t); return *this;};
	ByteArray() {};
};
/*	
class PGA: public AbstractDevice
{
	friend class SystemConfig;
//	friend class PGAControlWnd;
public:
	struct LogicGain
	{
		BYTE value; 
		explicit LogicGain(BYTE t=0) {value=t;}
		bool operator==(LogicGain& g) {return (value==g.value);}
		bool operator!=(LogicGain& g) {return (value!=g.value);}
	};
	struct RealGain
	{
		BYTE value;
		explicit RealGain(BYTE t=0) {value=t;}
		RealGain& operator=(LogicGain lg); 
		operator LogicGain();
	};
	struct LogicMode
	{
		BYTE value; 
		explicit LogicMode(BYTE t=0) {value=t;}
		bool operator==(LogicMode& m) {return (value==m.value);}		
	};
	struct RealMode
	{
		BYTE value;
		explicit RealMode(BYTE t=0) {value=t;}
		RealMode& operator=(LogicMode lg);
		operator LogicMode();
		bool operator!=(RealMode& m) {return (value!=m.value);}
	};
protected:	
	BYTE PGAnum; 
public:
	LogicGain gain; LogicMode mode;

	static	ByteArray PGAGainValues, ModeValues;	
	PGA() {};
};
*/