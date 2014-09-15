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
	operator double*() {return GetData();}
	double* GetX() {return GetData();}
	DoubleArray& operator=(DoubleArray& arr);
	operator gsl_vector*();
	void operator= (gsl_vector& vector);
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
class Solver1d: public PerfomanceInfoMk1
{
public:
	struct BoundaryConditions 
	{
		double min, max;
		BoundaryConditions() {min = max = 0; }
		BoundaryConditions(double _min, double _max) {min = _min; max = _max; }
	};
private:
	gsl_root_fsolver *s;
protected:
	const gsl_root_fsolver_type *fsolver_type;
	gsl_function F;
public:
	double root;
public:
	Solver1d(int _max_iter=100) 
	{ 
		fsolver_type=gsl_root_fsolver_brent; 
		s = NULL; F.function = NULL; F.params = NULL; 
		max_iter = _max_iter; 
	}	
public:
	virtual int Run(BoundaryConditions X, SolverErrors Err);
	virtual void CleanUp();
	//This function is reminder that FuncParam class should be provided in childs
	void virtual DefineME() = 0;
};
//////////////////////////////////////////////////////////////////////////
//  [F.params] is a pointer to instance of FuncParams class passed in cntor.
//  [F.function] is a static function of FuncParams class. 
enum SolverRegime {SINGLE_ROOT, MULTI_ROOT};
typedef CArray<Solver1d::BoundaryConditions> BoundaryConditionsArray;

class BaseForFuncParams
{
public:
	virtual void PrepareBuffers() {}
	virtual void DestroyBuffers() {}
	BaseForFuncParams() {}
	virtual ~BaseForFuncParams() {DestroyBuffers();}
};

template <class FuncParams>
class Solver1dTemplate: public Solver1d
{
protected:
	SolverRegime rgm; int subrgns_max;	
public:
	DoubleArray Roots;	

	static double func (double x, void *params) 
	{
		cntr.func_call++; 
		FuncParams::func(x, params);
	};
	Solver1dTemplate(SolverRegime _rgm, FuncParams* p, int _max_iter=100): 
		Solver1d(_max_iter), rgm(_rgm)
	{
		F.params = p; F.function = func; subrgns_max = 50;
	}
	virtual int Run(BoundaryConditions X, SolverErrors Err);
	virtual void CleanUp();
	void virtual DefineME() {}	
	int FindSubRgns(BoundaryConditions X, BoundaryConditionsArray& SubRgns);
	virtual int GetRoot(double * root) 
	{
		int status = GSL_SUCCESS;
		if (Roots.GetSize() != 0) 
			*root = Roots[0]; 
		else 
			status = GSL_FAILURE;
		return status;
	}
};

//////////////////////////////////////////////////////////////////////////
class MultiDimMinimizer: public PerfomanceInfoMk1
{
private:
	gsl_vector *X, *dX;
	gsl_multimin_fminimizer *s;	
protected:
	const gsl_multimin_fminimizer_type *fminimizer_type;	
	gsl_multimin_function F;
public:
	struct {DoubleArray x; double y; void CleanUp() {x.RemoveAll(); y = 0;}} Minimum;
public:
	MultiDimMinimizer(int _max_iter=100) 
	{ 
		fminimizer_type = gsl_multimin_fminimizer_nmsimplex; 
		s = NULL; X = dX = NULL; 
		F.f = NULL; F.n = 0; F.params = NULL; 
		max_iter=_max_iter;
	}
	int Run(DoubleArray& intiX, DoubleArray& initdX, SolverErrors Err);
	virtual void CleanUp();
	//This function is reminder that FuncParam class should be provided in childs
	void virtual DefineME() = 0;
};
//////////////////////////////////////////////////////////////////////////
template <class FuncParams>
class MultiDimMinimizerTemplate: public MultiDimMinimizer
{
public:
	static double func(const gsl_vector * x, void * params)
	{
		cntr.func_call++; 
		FuncParams::func(x, params);
	}

	MultiDimMinimizerTemplate(FuncParams* p, int _max_iter=100): MultiDimMinimizer(_max_iter)
	{		
		F.f = func; F.params = p; 
	}
	void virtual DefineME() {}
};
//////////////////////////////////////////////////////////////////////////
enum MultiFitter_mode {F_MODE, FDF_MODE};

class MultiFitter: public PerfomanceInfoMk1
{
protected:
	const gsl_multifit_fdfsolver_type *multifit_fdfsolver_type; 
	const gsl_multifit_fsolver_type *multifit_fsolver_type; 
	gsl_multifit_fdfsolver *s; 
	gsl_multifit_fsolver *sf; 
	size_t n, p;	
	gsl_multifit_function_fdf F;
	gsl_multifit_function Ff;
	gsl_vector_view x;

	int Init(double _epsabs, double _epsrel);
	int (MultiFitter::*Main)();
	int MainFDF();
	int MainF();
	virtual int GetParamsNum()=0;
	void CleanUp();
public:
	double epsabs, epsrel;
	int status; size_t max_iter, iter;
public:
	MultiFitter(MultiFitter_mode mode=FDF_MODE, int _max_iter=50) 
	{ 
		multifit_fdfsolver_type=NULL; s=NULL; sf=NULL; epsabs=epsrel=0;
		F.f = NULL; F.df = NULL; F.fdf = NULL; F.n=F.p=0; F.params = NULL; 
		max_iter=_max_iter; 
		if(mode==FDF_MODE) Main=&MultiFitter::MainFDF;
		else Main=&MultiFitter::MainF;
	}
	virtual ~MultiFitter() {CleanUp();}
};
//////////////////////////////////////////////////////////////////////////
template <class Params, class FuncParams>
class MultiFitterTemplate: public MultiFitter
{
public:
	FuncParams	fparams;
	Params		roots;

	static int	func_call_cntr;
protected:
	static int f (const gsl_vector * x, void *data, gsl_vector * f);
	static int df (const gsl_vector * x, void *data, gsl_matrix * J);
	static int fdf (const gsl_vector * x, void *data, gsl_vector * f, gsl_matrix * J)	
	{
		MultiFitterTemplate::f(x, data, f);
		MultiFitterTemplate::df(x, data, J);
		return GSL_SUCCESS;
	}

	virtual int Init(Params& start, double epsabs, double epsrel)
	{
		if( (status=MultiFitter::Init(epsabs,epsrel))==GSL_SUCCESS)
		{
			p=GetParamsNum(); n=GetPointsNum(); roots.n=n;
			F.f=f; F.df=df; F.fdf=fdf; F.params=&fparams; F.p=p; F.n=n;
			x = gsl_vector_view_array (start, p);	
			func_call_cntr=0;
		}
		return status;
	}
	virtual int GetParamsNum() {return roots.GetParamsNum();}
	virtual int GetPointsNum() {return fparams.GetPointsNum();}
public:
	MultiFitterTemplate(FuncParams& p, int _max_iter): MultiFitter(FDF_MODE,_max_iter), fparams(p)  {}
	virtual ~MultiFitterTemplate() {}
	int Run(Params& start, double epsabs, double epsrel)
	{		
		Timer1.Start();
		if( (status=Init(start, epsabs,epsrel))==GSL_SUCCESS)
		{
			status=(this->*Main)();
			roots.GetSolverResults(s); 
			roots.leftmostX=fparams.leftmostX; roots.rightmostX=fparams.rightmostX; 
			roots.status=status; roots.dx=fparams.dx;			
			CleanUp();
		}
		dt=Timer1.StopStart();
		return status;
	}
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