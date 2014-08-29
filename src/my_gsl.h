#pragma once

#include "mythread\MyTime.h"
#include "gsl\gsl_math.h"
#include "gsl\gsl_multimin.h"
#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_roots.h>
#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlin.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_halfcomplex.h>

#define DEGREE (M_PI/180.)
class DoubleArray: public CArray<double>
{
public:
	DoubleArray();
	DoubleArray(DoubleArray&);
	DoubleArray& operator << (double d) {(*this).Add(d); return (*this);}
	operator double*() {return GetData();}
	double* GetX() {return GetData();}
	DoubleArray& operator=(DoubleArray& arr);
};
struct GSLPerfomanceInformer 
{
	MyTimer Timer1;
	ms			dt;
};

struct PerfomanceInfoMk1 
{
	ms dt; int func_call_cntr; double epsabs,epsrel; int status, iter_num;
	PerfomanceInfoMk1() {func_call_cntr=0; epsrel=epsrel=0; status=GSL_FAILURE; iter_num=0;}
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
class Solver1d: public GSLPerfomanceInformer
{
protected:
	const gsl_root_fsolver_type *fsolver_type;
	gsl_root_fsolver *s;
	gsl_function F;
	size_t iter;

	int Init(double& x_lo, double& x_hi, double epsabs, double epsrel);
	int Main();
	void CleanUp();
public:
	double epsabs, epsrel, x_lo, x_hi;
	int status; size_t max_iter; 
public:
	Solver1d(int _max_iter=100) { fsolver_type=NULL; s=NULL; F.function = NULL; F.params = NULL; max_iter=_max_iter; }	
	virtual ~Solver1d() {CleanUp();}
};
//////////////////////////////////////////////////////////////////////////
template <class FuncParams>
class Solver1dTemplate: public Solver1d
{
public:
	FuncParams	fparams;
	double		root;
	static int	func_call_cntr;
	static double func (double x, void *params);
protected:	
	virtual int Init(double& x_lo, double& x_hi, double epsabs, double epsrel)
	{
		int ret=GSL_FAILURE; 
		if( (ret=Solver1d::Init(x_lo, x_hi, epsabs, epsrel))==GSL_SUCCESS)
		{
			F.function=func; F.params=&fparams; func_call_cntr=0;
		}
		return ret;
	}
public:
	Solver1dTemplate(FuncParams& p, int _max_iter=100): Solver1d(_max_iter), fparams(p)  {}
	virtual ~Solver1dTemplate() {}
	int Run(double x_lo, double x_hi, double epsabs, double epsrel=0)
	{
		int ret; Timer1.Start();
		if( (ret=Init(x_lo, x_hi, epsabs, epsrel))==GSL_SUCCESS)
		{
			ret=Main();
			root=gsl_root_fsolver_root (s); //root.status=status;
			CleanUp();
		}
		dt=Timer1.StopStart();
		return ret;
	}
};
//////////////////////////////////////////////////////////////////////////
template <class FuncParams>
class Solver1dMULTITemplate: public Solver1dTemplate<FuncParams>
{
	struct RootArea {double lo, hi;};
	typedef CArray<RootArea> RootsAray;
public:
	DoubleArray 		roots;
	static int	func_call_cntr;
	int			subs;
	int			max_roots, min_roots;
	static double func (double x, void *params);

	virtual int Init(double& x_lo, double& x_hi, double epsabs, double epsrel)
	{
		int ret=GSL_FAILURE; 
		if( (ret=Solver1dTemplate<FuncParams>::Init(x_lo, x_hi, epsabs, epsrel))==GSL_SUCCESS)
		{
			F.function=func; func_call_cntr=0;
		}
		return ret;
	}
	int HowManyRoots(RootsAray& arr)
	{
		SimplePoint *buf=new SimplePoint[subs]; SimplePoint tpnt; arr.RemoveAll();
		double t=(x_lo-x_hi)/(subs-1); RootArea ra; 
		for(int i=0;i<subs;i++)
		{
			buf[i].x=x_hi+i*t;		
			buf[i].y=func(buf[i].x,&fparams);
		}
		for(int i=0;i<subs-1;i++)
		{
			if( (buf[i].y*buf[i+1].y) <0 )
			{
				ra.hi=buf[i].x; ra.lo=buf[i+1].x; arr.Add(ra);
			}
		}
		delete[] buf;	
		return arr.GetSize();
	}
public:
	Solver1dMULTITemplate(FuncParams& p, int _max_iter=100): Solver1dTemplate<FuncParams>(p,_max_iter)  
	{
		subs=50; max_roots=4, min_roots=4;
	}
	virtual ~Solver1dMULTITemplate() {}
	int Run(double x_lo, double x_hi, double epsabs, double epsrel=0)
	{
		Timer1.Start();
		if( (status=Init(x_lo, x_hi, epsabs, epsrel))==GSL_SUCCESS)
		{
			RootsAray arr; RootArea ra; roots.RemoveAll();
			int roots_n=HowManyRoots(arr); 
			if(roots_n>=min_roots && roots_n<=max_roots)
			{
				for(int i=0;i<arr.GetSize() && status==GSL_SUCCESS;i++)
				{
					ra=arr[i];
					status=Solver1dTemplate<FuncParams>::Run(arr[i].lo,arr[i].hi,1e-6);
					if(status==GSL_SUCCESS) roots << root;
				}
			}
			else 
				status=100+roots_n;
		}
		dt=Timer1.StopStart();
		return status;
	}
};
//////////////////////////////////////////////////////////////////////////
class MultiDimMinimizer: public GSLPerfomanceInformer
{
protected:
	const gsl_multimin_fminimizer_type *fminimizer_type;
	gsl_multimin_fminimizer *s;
	gsl_vector *ss, *x;
	gsl_multimin_function F;
	size_t iter, np; 

	int Init(double epsabs);
	int Main();
	virtual int GetParamsNum()=0;
	void CleanUp();
public:
	double epsabs;
	int status; size_t max_iter;
public:
	MultiDimMinimizer(int _max_iter=100) 
	{ 
		fminimizer_type=NULL; s=NULL; F.f = NULL; F.n = 0; F.params = NULL; 
		max_iter=_max_iter;
	}
	virtual ~MultiDimMinimizer() {CleanUp();}
};
//////////////////////////////////////////////////////////////////////////
template <class Params, class FuncParams>
class MultiDimMinimizerTemplate: public MultiDimMinimizer
{
public:
	FuncParams&	fparams;
	Params		roots;
	double		fval, size;
	static int	func_call_cntr;
protected:
	static double func(const gsl_vector * x, void * params);	

	virtual int Init(Params& start, Params& init_step, double epsabs)
	{
		if( (status=MultiDimMinimizer::Init(epsabs))==GSL_SUCCESS)
		{
			if( (ss = init_step)==NULL ) return (status=GSL_FAILURE);
			if( (x =  start)==NULL ) { gsl_vector_free(ss); return (status=GSL_FAILURE); }
			F.f=func; F.params=&fparams; F.n=GetParamsNum();
			fval=size=0; func_call_cntr=0;
		}
		return status;
	}
	virtual int GetParamsNum() {return roots.GetParamsNum();}
public:
	MultiDimMinimizerTemplate(FuncParams& p, int _max_iter=100): MultiDimMinimizer(_max_iter), fparams(p)  {}
	virtual ~MultiDimMinimizerTemplate() {}
	int Run(Params& start, Params& init_step, double epsabs)
	{
		Timer1.Start();
		if( (status=Init(start, init_step, epsabs))==GSL_SUCCESS)
		{
			status=Main();
			roots=s->x; fval=s->fval; size=s->size; roots.status=status;
			CleanUp();
		}
		dt=Timer1.StopStart();
		return status;
	}
};
//////////////////////////////////////////////////////////////////////////
enum MultiFitter_mode {F_MODE, FDF_MODE};

class MultiFitter: public GSLPerfomanceInformer
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

class FFTRealTransform: public GSLPerfomanceInformer
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