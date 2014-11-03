// MyTime.h: interface for the MyTime class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYTIME_H__14DD8CA7_2E45_45E7_8E4E_414F4A3164CD__INCLUDED_)
#define AFX_MYTIME_H__14DD8CA7_2E45_45E7_8E4E_414F4A3164CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


template<typename T, int _b>
class MyBaseTimeType
{
	friend class MyBaseTimeType<double,1>; friend class MyBaseTimeType<int,100>; friend class MyBaseTimeType<int,10>;
	friend class MyBaseTimeType<double,1000>; friend class MyBaseTimeType<double,1000000>;
protected:
    T v; double base;
public:
	explicit MyBaseTimeType(T _l=0) {v=_l; base=_b;}
	virtual ~MyBaseTimeType() {};
	virtual void Serialize(CArchive &ar)
	{
		if (ar.IsStoring()) ar << v;
		else ar >> v;
	}	
	MyBaseTimeType& operator=(MyBaseTimeType _l) {v=_l.v; return (*this);};
	MyBaseTimeType& operator+=(MyBaseTimeType _l) {v+=_l.v; return (*this);};
	MyBaseTimeType& operator-=(MyBaseTimeType _l) {v-=_l.v; return (*this);};

	T val() {return v;}
	operator MyBaseTimeType<double,1>() {MyBaseTimeType<double,1> ret; ret.v=v*(base/ret.base); return ret;}
	operator MyBaseTimeType<int,10>() {MyBaseTimeType<int,10> ret; ret.v=MyRound(v*(base/ret.base));	return ret;}
	operator MyBaseTimeType<int,100>() {MyBaseTimeType<int,100> ret; ret.v=MyRound(v*(base/ret.base));	return ret;}
	operator MyBaseTimeType<double,1000>() {MyBaseTimeType<double,1000> ret; ret.v=v*(base/ret.base);	return ret;}
	operator MyBaseTimeType<double,1000000>() 
	{
		MyBaseTimeType<double,1000000> ret; 
		ret.v=v*(base/ret.base);	
		return ret;
	}

	MyBaseTimeType operator*(double _l);
	MyBaseTimeType operator*(int _l) {MyBaseTimeType ret; ret.v=v*_l; return ret;};
	MyBaseTimeType operator/(int _l) {MyBaseTimeType ret; ret.v=v/_l; return ret;};
	MyBaseTimeType operator+(MyBaseTimeType _l) {MyBaseTimeType ret; ret.v=v+_l.v; return ret;};
	MyBaseTimeType operator-(MyBaseTimeType _l) {MyBaseTimeType ret; ret.v=v-_l.v; return ret;};
	T operator/(MyBaseTimeType _l) {T ret; ret=v/_l.v; return ret;};	

	bool operator < (MyBaseTimeType _l) {return (v<_l.v);}
	bool operator >= (MyBaseTimeType _l) {return (v>=_l.v);}
	bool operator != (MyBaseTimeType _l) {return (v!=_l.v);}
	static int MyRound(double x)
	{
		int ret=(int)floor(x);
		double dd=fabs(1.0-(x-ret));
		if(dd<1e-3) ret++;
		return ret;
	}
};

typedef MyBaseTimeType<double,1> us;
typedef MyBaseTimeType<int,10> us10;
typedef MyBaseTimeType<int,100> us100;
typedef MyBaseTimeType<double,1000> ms;
typedef MyBaseTimeType<double,1000000> sec;

class MyTime  
{
public:
	int msec;
	CString GetString();
	void Init(double);
	int Sec,min,hour;
	MyTime();
	virtual ~MyTime();
	static CString ConvertToStr(sec time, int precision=0);

};

CString ConvTimeToStr(sec time);

struct MyTimer
{
LARGE_INTEGER start, stop;
double freq;
LARGE_INTEGER _checkPoints[100];
BYTE _checkPointsID[100];
int _checkPointsNumber;

MyTimer();
void Start();
void Stop();
sec StopStart();
sec GetValue();
CString GetString(double dim=1e-6);
CString GetStringF(double dim=1e-6);
};

enum StopState;

class Delayer
{
MyTimer T1, T2;
int NumberOfTimes, i;
sec CorrectionTime, TimeLeft;

public:
	CString Name;
	
	Delayer(sec _CorrectionTime=sec(10e-6));
	void Start(sec time);
	void Start(sec time, StopState* stop);
};

#endif // !defined(AFX_MYTIME_H__14DD8CA7_2E45_45E7_8E4E_414F4A3164CD__INCLUDED_)
