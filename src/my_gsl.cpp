#include "stdafx.h"
#include "my_gsl.h"


int Solver1d::Main ()
{
	fsolver_type=gsl_root_fsolver_brent;
	s = gsl_root_fsolver_alloc (fsolver_type);
	gsl_root_fsolver_set (s, &F, x_lo, x_hi);
	do
	{
		iter++;
		status = gsl_root_fsolver_iterate (s);
		x_lo = gsl_root_fsolver_x_lower (s);
		x_hi = gsl_root_fsolver_x_upper (s);
		status = gsl_root_test_interval (x_lo, x_hi, epsabs, epsrel);
	}
	while (status == GSL_CONTINUE && iter < max_iter);	
	return status;
}

int Solver1d::Init( double& _x_lo, double& _x_hi, double _epsabs, double _epsrel )
{
    x_lo=_x_lo; x_hi=_x_hi; epsabs=_epsabs; epsrel=_epsrel; iter=0;
	return GSL_SUCCESS;
}

void Solver1d::CleanUp()
{
	if(s!=NULL) { gsl_root_fsolver_free (s); s=NULL; }
}
//////////////////////////////////////////////////////////////////////////
int MultiDimMinimizer::Main(void)
{
	double size;
	fminimizer_type = gsl_multimin_fminimizer_nmsimplex; 
	s = gsl_multimin_fminimizer_alloc (fminimizer_type, np);
	gsl_multimin_fminimizer_set (s, &F, x, ss);
	do
	{
		iter++;
		status = gsl_multimin_fminimizer_iterate(s);
		if (status) break;
		size = gsl_multimin_fminimizer_size (s);
		status = gsl_multimin_test_size (size, epsabs);
	}
	while (status == GSL_CONTINUE && iter < max_iter);
	return status;
}

int MultiDimMinimizer::Init( double _epsabs )
{
	epsabs=_epsabs;	s = NULL; iter=0; np=GetParamsNum();
	return GSL_SUCCESS;
}

void MultiDimMinimizer::CleanUp()
{
	if(x!=NULL)		{gsl_vector_free(x); x=NULL;}
	if(ss!=NULL)	{gsl_vector_free(ss);ss=NULL;}
	if(s!=NULL)		{gsl_multimin_fminimizer_free (s); s=NULL;}
}

ComplexGSL sqrt( ComplexGSL& c )	{ return ComplexGSL(gsl_complex_sqrt(c.z)); }
ComplexGSL pow2( ComplexGSL& c )	{ return ComplexGSL(gsl_complex_mul(c.z,c.z)); }
ComplexGSL exp( ComplexGSL& c )		{ return ComplexGSL(gsl_complex_exp(c.z)); }
//////////////////////////////////////////////////////////////////////////

int MultiFitter::Init(double _epsabs, double _epsrel)
{
	epsabs=_epsabs; epsrel=_epsrel;	s = NULL; iter=0; p=GetParamsNum();
	return GSL_SUCCESS;
}

int MultiFitter::MainFDF()
{	
	multifit_fdfsolver_type = gsl_multifit_fdfsolver_lmsder;
	s = gsl_multifit_fdfsolver_alloc (multifit_fdfsolver_type, n, p);
	gsl_multifit_fdfsolver_set (s, &F, &x.vector);
	do
	{
		iter++;
		status = gsl_multifit_fdfsolver_iterate (s);
		status = gsl_multifit_test_delta (s->dx, s->x, epsabs, epsrel);
	}
	while (status == GSL_CONTINUE && iter < max_iter);
    return status;	
}

int MultiFitter::MainF()
{
	multifit_fdfsolver_type = gsl_multifit_fdfsolver_lmsder;
	sf = gsl_multifit_fsolver_alloc (multifit_fsolver_type, n, p);
	gsl_multifit_fsolver_set (sf, &Ff, &x.vector);
	do
	{
		iter++;
		status = gsl_multifit_fsolver_iterate (sf);
		status = gsl_multifit_test_delta (sf->dx, sf->x, epsabs, epsrel);
	}
	while (status == GSL_CONTINUE && iter < max_iter);
	return status;	
}

void MultiFitter::CleanUp()
{
	if(s!=NULL) { gsl_multifit_fdfsolver_free (s); s=NULL; }
	if(sf!=NULL) { gsl_multifit_fsolver_free (sf); sf=NULL; }
}
//////////////////////////////////////////////////////////////////////////
int FFTRealTransform::Main()
{
	switch(params.dir)
	{
	case FORWARD: 
		work = gsl_fft_real_workspace_alloc (n);
		real = gsl_fft_real_wavetable_alloc (n);
		status=gsl_fft_real_transform (y, 1, n, real, work);
		break;
	case BACKWARD: 
		work = gsl_fft_real_workspace_alloc (n);
		hc = gsl_fft_halfcomplex_wavetable_alloc (n);
		status=gsl_fft_halfcomplex_inverse (y, 1, n, hc, work);
		break;
	default: return GSL_FAILURE;
	}
	return status;
}

void FFTRealTransform::CleanUp()
{
	if(work!=NULL)	{ gsl_fft_real_workspace_free (work); work=NULL; }
	if(real!=NULL)	{ gsl_fft_real_wavetable_free (real); real=NULL; }
	if(hc!=NULL)	{ gsl_fft_halfcomplex_wavetable_free (hc); hc=NULL; }
}

int FFTRealTransform::Run( Params& params,Direction dir )
{
	Timer1.Start();
	if( (status=Init(params, dir))==GSL_SUCCESS)
	{
		status=Main();
		CleanUp();
	}
	dt=Timer1.StopStart();
	if(status==GSL_SUCCESS)
	{		
		params.dt=dt;
		params.status=status;
	}
	return status;
}

int FFTRealTransform::Init( Params& _params, Direction _dir )
{
	params=_params; y=params.y->GetData(); n=params.y->GetSize(); params.dir=_dir; 
	return GSL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////

DoubleArray& DoubleArray::operator=( DoubleArray& arr )
{
	RemoveAll();
	SetSize(arr.GetSize());
	for(int i=0; i<arr.GetSize();i++) Add(arr[i]);
	return *this;
}

DoubleArray::DoubleArray(): CArray<double>()
{

}
