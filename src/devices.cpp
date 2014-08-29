#include "stdafx.h"
#include "devices.h"
#include "systemconfig.h"
#include "resource.h"

//ByteArray PGA::PGAGainValues;
//ByteArray PGA::ModeValues;

void CfgParamTypeBase::UpdateParent() {if(Parent!=NULL) Parent->ParamUpdate(this);}

void CfgParamTypeBase::FillCfgTree( CTreeCtrl* t,HTREEITEM l0 )
{
	CString T;
	FormatParam(T);
	ASSERT(Parent);
	TreeItem=t->InsertItem(T,l0); TPD r(Parent->id,(WORD)pid);
	t->SetItemData(TreeItem,r);
}

void YesNoCfgParam::GetParamValue(CString& t) {t=(val ? "YES":"NO");}
void DblCfgParam::GetParamValue(CString& t) {t.Format("%g",val);}
void IntCfgParam::GetParamValue(CString& t) {t.Format("%d",val);}
void ByteCfgParam::GetParamValue(CString& t) {t.Format("%d",val);}
void StrCfgParam::GetParamValue(CString& t) {t=val;}
/*
void wPicaCfgParam::GetParamValue(CString& t) {t.Format("%g",val.val());}
void sNanoSecCfgParam::GetParamValue(CString& t) {t.Format("%g",val.val());}
void wCNanoCfgParam::GetParamValue(CString& t) {nm t1; fm10 t2; t2=*this; t1=t2; t.Format("%g",t1.val());}
void msCfgParam::GetParamValue(CString& t) {t.Format("%g",val.val());}
void secCfgParam::GetParamValue(CString& t) {t.Format("%g",val.val());}
void FilterPosCfgParam::GetParamValue(CString& t) {t.Format("%d",char(val));}
*/
void YesNoCfgParam::SetParamValue(CString& t) {if(IsEditable) {val=(t=="YES" ? true:false);UpdateParent();}}
void DblCfgParam::SetParamValue(CString& t) {if(IsEditable) {val=atof(t);UpdateParent();}}
void IntCfgParam::SetParamValue(CString& t) {if(IsEditable) {val=atoi(t);UpdateParent();}}
void ByteCfgParam::SetParamValue(CString& t) {if(IsEditable) {val=(BYTE)atoi(t);UpdateParent();}}
void StrCfgParam::SetParamValue(CString& t) {if(IsEditable) {val=t;UpdateParent();}}
/*
void wPicaCfgParam::SetParamValue(CString& t) {if(IsEditable) {val=pm(atof(t));UpdateParent();}}
void sNanoSecCfgParam::SetParamValue(CString& t) {if(IsEditable) {val=sNanoSec(atof(t));UpdateParent();}}
void wCNanoCfgParam::SetParamValue(CString& t) {if(IsEditable) {val=nmC(atof(t));UpdateParent();}}
void msCfgParam::SetParamValue(CString& t) {if(IsEditable) {val=ms(atof(t));UpdateParent();}}
void secCfgParam::SetParamValue(CString& t) {if(IsEditable) {val=ms(atof(t));UpdateParent();}}
void FilterPosCfgParam::SetParamValue(CString& t) {if(IsEditable) {val=FilterHeadPos((char)atoi(t));UpdateParent();}}
*/
void YesNoCfgParam::Serialize(CArchive& ar) {if (ar.IsStoring()) ar << val; else ar >> val;}
void DblCfgParam::Serialize(CArchive& ar) {if (ar.IsStoring()) ar << val; else ar >> val;}
void IntCfgParam::Serialize(CArchive& ar) {if (ar.IsStoring()) ar << val; else ar >> val;}
void ByteCfgParam::Serialize(CArchive& ar) {if (ar.IsStoring()) ar << val; else ar >> val;}
/*
void wPicaCfgParam::Serialize(CArchive& ar) {val.Serialize(ar);}
void sNanoSecCfgParam::Serialize(CArchive& ar) {val.Serialize(ar);}
void wCNanoCfgParam::Serialize(CArchive& ar) {val.Serialize(ar);}
void msCfgParam::Serialize(CArchive& ar) {val.Serialize(ar);}
void secCfgParam::Serialize(CArchive& ar) {val.Serialize(ar);}
void FilterPosCfgParam::Serialize(CArchive& ar) {val.Serialize(ar);}
*/
/*
ADC::ADC(void):	
	num_mes("число измерений","",30),
	time_mes("время измерений","",sec(0.3)),
	AVCC("Опорное нарпяжение","В",5),
	Digits("Разрешение","",12)
{
	Name="Generic ADC"; 
	RegisterParam(&num_mes); RegisterParam(&time_mes); 
	RegisterParam(&AVCC); RegisterParam(&Digits); 
	num_mes.IsEditable=time_mes.IsEditable=true;
	AVCC.IsEditable=true;
}

StepMotor::StepMotor(void): 
	invert(YesNoCfgParam("реверс","",false))
{
	invert=0; RegisterParam(&invert); Name="Generic StepMotor";
	invert.IsEditable=true;
}

void StepMotor::RoundToStep( fm10& _l1 ) {fm10 t(GetStep()); _l1.MakeMultipleTo(t);}

StepMotor::Params StepMotor::Setup( AbstractSetupParams* pp)
{
	StepMotor::Params ret;
	StepMotor::SetupParams *p=(StepMotor::SetupParams*)pp;
	ret.dir=(p->st<0 ? (char)MOTOR_BACKWARD:(char)MOTOR_FORWARD);  
	if(invert!=0) ret.dir*=-1;
	ret.st=abs(p->st); 
	ret.period=p->period;
	return ret;
}

sNanoSec SDR::GetSpeed() {int c; Parent->Get(CFG_GRATING_COEF,c); sNanoSec ret; ret=Speed; ret*=c; return ret;}	
bool StepMotor::GetInvert() {return invert;}


SDR::SDR(void):
	step(wPicaCfgParam("шаг","пм",pm(4))), 
	Speed(sNanoSecCfgParam("скорость","нм/с",sNanoSec(2.)))
{
	Name="ШДР-711";
	RegisterParam(&step);  RegisterParam(&Speed); 
	step.IsEditable=Speed.IsEditable=true;
}
SDRSensor::SDRSensor(void)	
{
	Name="ШДР-ОС"; RegisterParam(&Sensor1.step); RegisterParam(&Sensor1.Jitter);
}

pm SDR::GetStep() {int c; Parent->Get(CFG_GRATING_COEF,c); pm ret; ret=step; ret*=c; return ret;}

StepMotor::Params SDR::Setup( AbstractSetupParams* pp)
{
	StepMotor::Params ret;
	SDR::SetupParams *p=(SDR::SetupParams*)pp;
	fm10 t1(GetStep()); RoundToStep(p->dl);
	if(Abs(p->dl)<t1) {Error* t=new Error(); RegisterError(t->Create(ERR_DL_LESS_MDL,Name));}	
	p->st=p->dl/t1; p->period=sec((pm)step/(sNanoSec)Speed);
	ret=StepMotor::Setup(p);
	return ret;
}

pm SDRSensor::GetStep() {int c; Parent->Get(CFG_GRATING_COEF,c); pm ret; ret=Sensor1.step; ret*=c; return ret;}

StepMotor::Params SDA::Setup(AbstractSetupParams* pp)
{
	StepMotor::Params ret;
	SDA::SetupParams* p=(SDA::SetupParams*)pp;
	int d=p->end-p->beg; if(d<0) d+=FILTER_MAX;
	p->st=d*StepsPerFase; 
	p->period=(ms)Period;
	ret=StepMotor::Setup(p);
	return ret;		
}

SDA::SDA(void): 
	StepsPerFase("Смена позиции","шаг",2), 
	Period("Период чередования шагов","мс",ms(60))	
{
	Name="ШДА"; RegisterParam(&StepsPerFase); RegisterParam(&Period); 
	Period.IsEditable=true; 
}

mdr23::mdr23(void):
	CounterMax(wCNanoCfgParam("Максимальная длина волны(сч","нм",nmC(1000.))),
	CounterMin(wCNanoCfgParam("Минимальная длина волны(сч)","нм",nmC(200.))),
	GratingCoef(IntCfgParam("Тип решетки","",1))
{
	Name="МДР-23"; CurWaveLen=nmC(500);
	RegisterParam(&CounterMax); RegisterParam(&CounterMin); RegisterParam(&GratingCoef);
	GratingCoef.IsEditable=true;
	CounterMin.IsEditable=true;
	CounterMax.IsEditable=true;
}
*/
/*
void AbstractDevice::FillCfgTreeSingleParam(CTreeCtrl* t,HTREEITEM l0, CfgParamTypeBase* t2)
{
	CString T;
	t2->FormatParam(T);
	t2->TreeItem=t->InsertItem(T,l0); TPD r(id,(WORD)t2->pid);
	t->SetItemData(t2->TreeItem,r);
}
*/
void AbstractDevice::FillCfgTree(CTreeCtrl* t,HTREEITEM l0)
{
	for(int i=0;i<Params.GetSize(); i++)
	{
		Params[i]->FillCfgTree(t,l0);
	}
}

void AbstractDevice::Serialize(CArchive& ar)
{
		if (ar.IsStoring()) ar << Name;
		else ar >> Name;
		for(int i=0;i<Params.GetSize();i++) Params[i]->Serialize(ar);
}	


int AbstractDevice::FormatParam(int id,CString& T) {return 0;}

int AbstractDevice::RegisterParam(CfgParamTypeBase* t) {int ret=Params.Add(t); t->pid=ret+1; t->Parent=this; return ret;}

void AbstractDevice::ParamUpdate( CfgParamTypeBase* t )
{
	 Parent->DeviceUpdate(this,t);
}

/*
bool mdr23::CheckWavelength(nmC val) {return (val>=CounterMin && val<=CounterMax);}
*/
/*
bool mdr23::SetWavelength(nmC val)
{
	bool ret=false;
	if(CheckWavelength(val)) {CurWaveLen=val; ret=true;}
	return ret;
}
*/
/*
AbFilter::AbFilter(nm l1, nm l2, int p): AbstractHeadElement(p), lmin(l1), lmax(l2)
{
	code=ERR; 
}

AbFilter::AbFilter(FiltersCodes _code, int p): AbstractHeadElement(p)
{
	code=_code; 
	switch (code)
	{
	case ERR:
	case UV: lmin=nm(180); lmax=nm(360);	break;
	case VBL: lmin=nm(360); lmax=nm(600);	break;
	case NIR: lmin=nm(600); lmax=nm(1000);	break;
	case IR: lmin=nm(1000); lmax=nm(1500);	break;
	default: code=ERR; lmin=nm(0); lmax=nm(0);
	}
}

bool AbFilter::CheckWithin(nm l)
{
	bool ret=false;
	if(l>=lmin && l<=lmax) ret=true;
	return ret;
}

filterHead::filterHead(): 
	RevolvHeadPos("Текущий фильтр","",3), 
	OperationMode("Автоматический режим","",false)
{
	int i=0; EmptyHeadPos* ep; SolidHeadPos *sp; AbFilter *af; Name="Головка с фильтрами"; 
	RevolvHeadPos.IsEditable=true; RegisterParam(&RevolvHeadPos);
	OperationMode.IsEditable=true; RegisterParam(&OperationMode);
	ep=new EmptyHeadPos(i++); arr.Add(ep); af=new AbFilter(UV,i++); arr.Add(af);
	sp=new SolidHeadPos(i++); arr.Add(sp); af=new AbFilter(VBL,i++); arr.Add(af);
	sp=new SolidHeadPos(i++); arr.Add(sp); af=new AbFilter(NIR,i++); arr.Add(af);
	sp=new SolidHeadPos(i++); arr.Add(sp); af=new AbFilter(IR,i++); arr.Add(af);
}

filterHead::~filterHead()
{
	for(int i=0;i<arr.GetSize(); i++) delete arr[i];
	arr.RemoveAll();
}

int filterHead::FilterForWavelen( nm w, FilterHeadPos init_pos)
{
	int ret=FILTER_ERR;
	if (arr[init_pos]->CheckWithin(w)) ret=FILTER_OK;
	return ret;
}

FilterHeadPos filterHead::FindFilter( nm w, FilterHeadPos init_pos)
{
	int i; filtersArray _arr;	
	for(i=0;i<arr.GetSize();i++) 
	{
		if(arr[i]->CheckWithin(w)) 
			_arr.Add(arr[i]);
	}

	int d=2*FILTER_MAX,p=ERR,tp,fp;

	for(i=0;i<_arr.GetSize();i++)
	{
		fp=_arr[i]->GetPos();		
		tp=fp-init_pos;
		if(fp<init_pos) tp+=FILTER_MAX;
		if(tp<d) p=fp;
	}
	return FilterHeadPos((char)p);
}

FilterHeadPos::FilterHeadPos( FiltersCodes code )
{
	filterHead& fh=MainCfg.FilterHead;

	int i, j=(FilterHeadPos)fh.RevolvHeadPos; pos=ERR;
	switch(code)
	{
	case EMPTY:
	case SOLID:
	case UV:
	case VBL:
	case NIR:
	case IR:
		for(i=0;i<fh.arr.GetSize();i++)
		{
			if(fh.arr[j]->GetCode()==code) {pos=(char)(fh.arr[j]->GetPos()); break;}
			if(++j>=FILTER_MAX) j=0;
		}
		break;
	case CUR_POS: pos=(char)j; break;
	}
}

FilterHeadPos::operator char()
{
	return (char)pos;
}

Sensor::Sensor(): 
	step(wPicaCfgParam("шаг датчика","пм",pm(100.))), 
	Jitter(IntCfgParam("джитер","",20)) 
{
	step.IsEditable=Jitter.IsEditable=true;
};

GenericDevice::GenericDevice(): 
	SensorStatus(YesNoCfgParam("Включить датчик перестройки","",false)),
	ExchangePGAsControl(YesNoCfgParam("Поменять каналы управления PGA","",false)),
	ExchangeADCChnls(YesNoCfgParam("Поменять каналы ADC","",false)),
	GainAdjust(YesNoCfgParam("Учитывать усиление","",false)),
	AutoGain(YesNoCfgParam("Автоматическая регулировка усиления","",false))
{
	SensorStatus.IsEditable=true; RegisterParam(&SensorStatus);
	ExchangePGAsControl.IsEditable=true; RegisterParam(&ExchangePGAsControl);
	ExchangeADCChnls.IsEditable=true; RegisterParam(&ExchangeADCChnls);
	GainAdjust.IsEditable=true; RegisterParam(&GainAdjust);
	AutoGain.IsEditable=true; RegisterParam(&AutoGain);
};

PGA::RealGain& PGA::RealGain::operator=( LogicGain lg )
{
    value=PGAGainValues[lg.value];    
	return *this;
}

PGA::RealGain::operator PGA::LogicGain()
{
	PGA::LogicGain ret;
	switch (value)
	{
	case 1: ret.value=0;	break;
	case 2: ret.value=1;	break;
	case 5: ret.value=2;	break;
	case 10: ret.value=3;	break;
	case 20: ret.value=4;	break;
	case 50: ret.value=5;	break;
	case 100: ret.value=6;	break;
	case 200: ret.value=7;	break;
	default: ret.value=0;
	}
	return ret;
}

PGA::RealMode& PGA::RealMode::operator=( LogicMode lm )
{
	value=ModeValues[lm.value];    
	return *this;
}

PGA::RealMode::operator PGA::LogicMode()
{
	PGA::LogicMode ret;
	switch (value)
	{
	case 0: ret.value=0;	break;
	case 1: ret.value=1;	break;
	case 12: ret.value=2;	break;
	case 13: ret.value=3;	break;
	case 14: ret.value=4;	break;
	default: ret.value=0;
	}
	return ret;
}
*/
CalibrationData::CalibrationData( void ):
	fi("fi","град",0), d("d","отн.ед",0), L("L","отн.ед",0), N("N","отн.ед",0)//,
{
	Name="Calibration";
	RegisterParam(&fi); RegisterParam(&d); RegisterParam(&L); RegisterParam(&N);   

	CString T;
	for(int i=0;i<CALIBRATION_MODES_NUM; i++) 
	{ 
		T.Format("Nexp[%d]",i); N_exp[i]=new DblCfgParam(T,"пикс",0); RegisterParam(N_exp[i]);
		T.Format("teta[%d]",i); teta[i]=new DblCfgParam(T,"пикс",0); RegisterParam(teta[i]);
	}
}

CalibrationData::~CalibrationData()
{
	for(int i=0;i<CALIBRATION_MODES_NUM; i++) 
	{
		delete N_exp[i]; delete teta[i];
	}
}

void CalibrationData::Serialize( CArchive& ar )
{	
	if(ar.IsStoring())
	{
		cal.Serialize(ar);
	}
	else
	{
		CalibrationParams _cal; 
		_cal.Serialize(ar);
		SetCalibration(_cal);
	}
}

void CalibrationData::FillCfgTree( CTreeCtrl* t,HTREEITEM l0 )
{
	int i; HTREEITEM l1,l2;
	for(i=0;i<Params.GetSize() && i<4; i++)
	{
		Params[i]->FillCfgTree(t,l0);
	}
	l1=t->InsertItem("Nexp",l0); l2=t->InsertItem("teta",l0); 
	for(;i<Params.GetSize();)
	{
		Params[i++]->FillCfgTree(t,l1);
		Params[i++]->FillCfgTree(t,l2);
	}
}

void CalibrationData::SetCalibration( CalibrationParams& _cal )
{
	cal=_cal;
	fi.SetParamValue(cal.fi0); 
	d.SetParamValue(cal.d0); 
	N.SetParamValue(cal.N0); 
	L.SetParamValue(cal.L);
	for(int i=0;i<cal.Nexp.GetSize();i++)
	{
		N_exp[i]->SetParamValue(cal.Nexp[i]);
	}
	for(int i=0;i<cal.teta.GetSize();i++)
	{
		teta[i]->SetParamValue(cal.teta[i]);
	}
}

