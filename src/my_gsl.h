#pragma once

#include "mythread\MyTime.h"
#include "gsl\gsl_math.h"
#include "gsl\gsl_multimin.h"
#include "gsl\gsl_errno.h"
#include "gsl\gsl_math.h"
#include "gsl\gsl_roots.h"
#include "gsl\gsl_complex.h"
#include "gsl\gsl_complex_math.h"
#include "gsl\gsl_vector.h"
#include "gsl\gsl_blas.h"
#include "gsl\gsl_multifit_nlin.h"
#include "gsl\gsl_fft_real.h"
#include "gsl\gsl_fft_halfcomplex.h"

#define DEGREE (M_PI/180.)
class DoubleArray: public CArray<double>
{
public:
	DoubleArray() {};
	DoubleArray(DoubleArray&);
	DoubleArray& operator << (double d) {(*this).Add(d); return (*this);}
	DoubleArray& operator=(const DoubleArray& arr);
	operator double*() {return CArray<double>::GetData();}
	operator gsl_vector*();
	void operator= (const gsl_vector& vector);
	virtual void Serialize(CArchive& ar);
	BOOL operator==(const CArray<double> &ref);
};

struct SolverErrors 
{
	double abs, rel; 
	SolverErrors() {CleanUp();} SolverErrors(double _abs, double _rel = 0): abs(_abs), rel(_rel) {CleanUp();} 
	void CleanUp() {abs = rel = 0.;}
};

struct PerfomanceInfoMk1 
{
	SolverErrors err;
	struct Counters {size_t func_call, iter; Counters() {CleanUp();} void CleanUp() {func_call = iter = 0;}} cntr;
	ms dt; int status; size_t max_iter;
	PerfomanceInfoMk1() {status = GSL_FAILURE; max_iter = 0; CleanUp();}
	virtual ~PerfomanceInfoMk1() { CleanUp(); }
	virtual void CleanUp() {status = GSL_FAILURE; max_iter = 0; err.CleanUp(); cntr.CleanUp();}
};
//////////////////////////////////////////////////////////////////////////
struct ComplexImGSL {double Im; ComplexImGSL(double i=1.) {Im=i;}};
struct ComplexReGSL {double Re; ComplexReGSL(double r=0) {Re=r;}};

#define cJ ComplexImGSL()

class ComplexGSL
{
public:
	gsl_complex z;

	ComplexGSL(double Re=0, double Im=0) { z=gsl_complex_rect(Re,Im); }
	ComplexGSL(gsl_complex c) { z=c; }
	ComplexGSL(ComplexReGSL r) { z=gsl_complex_rect(r.Re,0); }
	ComplexGSL(ComplexImGSL i) { z=gsl_complex_rect(0,i.Im); }
	~ComplexGSL() {};
	ComplexGSL operator-(ComplexGSL& c)		{ return ComplexGSL(gsl_complex_sub(z,c.z)); }
	ComplexGSL operator-(double r)			{ return ComplexGSL(gsl_complex_sub_real(z,r)); }
	ComplexGSL operator+(ComplexGSL& c)		{ return ComplexGSL(gsl_complex_add(z,c.z)); }
	ComplexGSL operator+(double r)			{ return ComplexGSL(gsl_complex_add_real(z,r)); }
	ComplexGSL operator/(ComplexGSL& c)		{ return ComplexGSL(gsl_complex_div(z,c.z)); }
	ComplexGSL operator*(ComplexGSL& c)		{ return ComplexGSL(gsl_complex_mul(z,c.z)); }
	ComplexGSL operator*(double r)			{ return ComplexGSL(gsl_complex_mul_real(z,r)); }
	ComplexGSL operator*(ComplexImGSL& i)	{ return ComplexGSL(gsl_complex_mul_imag(z,i.Im)); }
	void operator*=(double r)				{ z=gsl_complex_mul_real(z,r); return;}
	void operator*=(ComplexImGSL i)			{ z=gsl_complex_mul_imag(z,i.Im); return;}
	double abs2()							{ return gsl_complex_abs2(z); }

};
ComplexGSL sqrt(ComplexGSL& c);
ComplexGSL pow2(ComplexGSL& c);
ComplexGSL exp(ComplexGSL& c);

//////////////////////////////////////////////////////////////////////////
//	This class is not allowed for use.
//	It is necessary to define [F.function] and [F.params] to be passed in this function.

struct SolverData 
{
	SolverErrors err;
	struct Counters 
	{
		size_t func_call, iter; 
		Counters() {CleanUp();} 
		void CleanUp() {func_call = iter = 0;}
	} cntr;
	ms dt; int status; size_t max_iter;

	SolverData(size_t _max_iter = 100): max_iter(_max_iter)	
	{
		status = GSL_FAILURE; CleanUp();
	}
	virtual ~SolverData() { CleanUp(); }
	virtual void CleanUp() {status = GSL_FAILURE; max_iter = 0; err.CleanUp(); cntr.CleanUp();}
};

enum SolverRegime {SINGLE_ROOT, MULTI_ROOT};

class BaseForFuncParams
{
public:
	int func_call_cntr;

	virtual void PrepareBuffers() {}
	virtual void DestroyBuffers() {}
	BaseForFuncParams() {CleanUp();}
	virtual ~BaseForFuncParams() {DestroyBuffers();}
	virtual void CleanUp() {func_call_cntr = 0;}
};

struct BoundaryConditions
{
	double min, max;
	BoundaryConditions() {min = max = 0; }
	BoundaryConditions(double _min, double _max) {min = _min; max = _max; }
};

template <class FuncParams>
class Solver1dTemplate: public SolverData
{
//************************************************//	
	static double func(double x, void * data)
	{
		Solver1dTemplate<FuncParams>* solver = (Solver1dTemplate<FuncParams>*)data;
		solver->params->func_call_cntr++;
		return solver->params->func(x);
	};
	typedef CArray<BoundaryConditions> BoundaryConditionsArray;
//************************************************//
private:
	gsl_root_fsolver *s;
	const gsl_root_fsolver_type *fsolver_type;
	gsl_function F; int iter;
	SolverRegime rgm; int subrgns_max;	
	BoundaryConditionsArray SubRgns;
	FuncParams* params;
public:	
	DoubleArray Roots;	
public:
	Solver1dTemplate(SolverRegime _rgm, int _max_iter=100);
	virtual int Run(FuncParams* params, BoundaryConditions X, SolverErrors Err);
	virtual void CleanUp();
protected:
	int FindSubRgns(BoundaryConditions X, BoundaryConditionsArray& SubRgns);
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
template <class FuncParams>
class MultiDimMinimizerTemplate: public SolverData
{
//************************************************//
public:
	static double func(const gsl_vector * x, void * data)
	{
		MultiDimMinimizerTemplate<FuncParams>* solver = (MultiDimMinimizerTemplate<FuncParams>*)data;
		solver->params->func_call_cntr++;
		return solver->params->func(x);
	};
//************************************************//
private:
	gsl_vector *X, *dX;
	gsl_multimin_fminimizer *s;	
	const gsl_multimin_fminimizer_type *fminimizer_type;	
	gsl_multimin_function F;
	FuncParams* params;
public:
	DoubleArray Roots; double minimum_value;
//************************************************//
	MultiDimMinimizerTemplate(int _max_iter=100);
	int Run(FuncParams* params, DoubleArray& intiX, DoubleArray& initdX, SolverErrors Err);
	virtual void CleanUp();
};
//////////////////////////////////////////////////////////////////////////
typedef double (*pFunc)(const double &x, const double *a, const size_t &p);
typedef double (*pDerivFunc)(const double &x, const double *a, const size_t &p, double *c);
struct BaseForMultiFitterFuncParams:public BaseForFuncParams
{
public:
	size_t n, p; const double *y, *sigma; double leftmostX, rightmostX, dx;
	pDerivFunc *pDerivatives; pFunc pFunction;

	BaseForMultiFitterFuncParams(const size_t _p, const DoubleArray &_x, const DoubleArray &_y, const DoubleArray &_sigma);
	virtual ~BaseForMultiFitterFuncParams();
	virtual double * PrepareDerivBuf(const double &x, const double *a, const size_t &p) { return NULL; };
	size_t GetPointsNum() {return n;}
	int f(const gsl_vector * x, gsl_vector * f);
	int df(const gsl_vector * x, gsl_matrix * J);
};


struct BaseForFitFunc: public SolverData
{
	double leftmostX, rightmostX, dx;
	DoubleArray a, da; pFunc pFunction;

	BaseForFitFunc(): SolverData() { leftmostX = rightmostX = dx = 0; pFunction = NULL;}
	void InitFrom(const BaseForMultiFitterFuncParams &params);
	void InitFrom(const SolverData &data);
	double GetXabsY(const double &x);
	double GetXrelY(double &x);
};

template <class FuncParams>
class MultiFitterTemplate: public SolverData
{
private:
	const gsl_multifit_fdfsolver_type *multifit_fdfsolver_type; 
	gsl_multifit_fdfsolver *s; 
	size_t n, p;	
	gsl_multifit_function_fdf F;
	gsl_vector* initX;
	FuncParams*	params;
protected:
	static int f(const gsl_vector * x, void *data, gsl_vector * f)
	{
		MultiFitterTemplate<FuncParams>* solver = (MultiFitterTemplate<FuncParams>*)data;
		solver->params->func_call_cntr++;
		return solver->params->f(x, f);
	}
	static int df(const gsl_vector * x, void *data, gsl_matrix * J)
	{
		MultiFitterTemplate<FuncParams>* solver = (MultiFitterTemplate<FuncParams>*)data;
		solver->params->func_call_cntr++;
		return solver->params->df(x, J);
	}
	static int fdf(const gsl_vector * x, void *data, gsl_vector * f, gsl_matrix * J)	
	{
		MultiFitterTemplate<FuncParams>* solver = (MultiFitterTemplate<FuncParams>*)data;
		solver->params->f(x, f);
		solver->params->df(x, J);
		return GSL_SUCCESS;
	}
public:	
	DoubleArray a, da;

public:
	MultiFitterTemplate(int _max_iter = 100): SolverData(_max_iter)
	{
		multifit_fdfsolver_type=gsl_multifit_fdfsolver_lmsder; s = NULL; initX = NULL;
		F.f = f; F.df = df; F.fdf = fdf; F.params = this; 		
	}
	virtual ~MultiFitterTemplate() {CleanUp();}
	virtual void CleanUp();
	int Run(FuncParams* params, const DoubleArray& init_a, const SolverErrors Err);
};
//////////////////////////////////////////////////////////////////////////

class FFTRealTransform: public PerfomanceInfoMk1
{
public:
	enum Direction {FORWARD, BACKWARD};
	struct Params: public PerfomanceInfoMk1
	{
		DoubleArray* y; Direction dir;
		Params() { y=NULL; dir=FORWARD;}
		Params(DoubleArray& _y) { y=&_y; }
	} params;
protected:
	gsl_fft_real_wavetable * real;
	gsl_fft_halfcomplex_wavetable * hc;
	gsl_fft_real_workspace * work;
	double *y; int n;

	int Init(Params& _params, Direction _dir);
	int Main();
	void CleanUp();
public:
	int status;
public:
	FFTRealTransform(int _max_iter=100) { real=NULL; hc=NULL; work=NULL; }
	virtual ~FFTRealTransform() {CleanUp();}
	int Run(Params& in,Direction dir);
};