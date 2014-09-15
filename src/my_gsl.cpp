#include "stdafx.h"
#include "my_gsl.h"

DoubleArray& DoubleArray::operator=( DoubleArray& arr )
{
	RemoveAll();
	SetSize(arr.GetSize());
	for(int i=0; i<arr.GetSize();i++) Add(arr[i]);
	return *this;
}

void DoubleArray::operator=( gsl_vector& vector )
{
	double t; RemoveAll();
	for (size_t i = 0; i < vector.size; i++)  
	{
		t = gsl_vector_get (&vector, i);
		(*this).Add(t);
	}
}

DoubleArray::operator gsl_vector*()
{
	gsl_vector* ret=NULL; double* arr = GetData(); int size = GetSize();
	ret = gsl_vector_alloc (size);

	for (int i = 0; i < GetSize(); i++)  
		gsl_vector_set (ret, i, arr[i]);
	return ret;
}

int Solver1d::Run (BoundaryConditions X, SolverErrors Err)
{
	MyTimer Timer1; Timer1.Start();
	CleanUp();	
	s = gsl_root_fsolver_alloc (fsolver_type);
	gsl_root_fsolver_set (s, &F, X.min, X.max);
	do
	{
		cntr.iter++;
		status = gsl_root_fsolver_iterate (s);
		X.min = gsl_root_fsolver_x_lower (s); X.max = gsl_root_fsolver_x_upper (s);
		status = gsl_root_test_interval (X.min, X.max, Err.abs, Err.rel);
	}
	while (status == GSL_CONTINUE && cntr.iter < max_iter);	
	root = gsl_root_fsolver_root(s); 
	dt = Timer1.StopStart();
	return status;
}

void Solver1d::CleanUp()
{
	if (s != NULL) { gsl_root_fsolver_free(s); s = NULL; }
	PerfomanceInfoMk1::CleanUp();
}

template <class FuncParams>
void Solver1dTemplate<FuncParams>::CleanUp()
{
	Roots.RemoveAll(); Solver1d::CleanUp(); 
}

template <class FuncParams>
int Solver1dTemplate<FuncParams>::FindSubRgns( BoundaryConditions X, BoundaryConditionsArray& SubRgns )
{
	if (rgm == SINGLE_ROOT)
	{
		SubRgns.Add(X);
	}
	else
	{
		double x, dx = (X.max - X.min)/(subrgns_max - 1); 
		BoundaryConditions y(func(X.min, F.params), 0);

		for(int i = 1; i < subrgns_max; i++)
		{
			x = X.min + i*dx; y.max = func(x, F.params);
			if ((y.min < 0 && y.max > 0) || (y.min > 0 && y.max < 0))
			{
				SubRgns.Add(BoundaryConditions(x - dx, x));
			}
			y.min = y.max;
		}
	}
}

template <class FuncParams>
int Solver1dTemplate<FuncParams>::Run( BoundaryConditions X, SolverErrors Err )
{
	MyTimer Timer1; double r; BoundaryConditionsArray SubRgns; int total_func_call = 0;
	Timer1.Start(); 
	CleanUp(); 
	p->PrepareBuffers();
	FindSubRgns(X, SubRgns); 
	for(int i = 0; i < SubRgns.GetSize(); i++)
	{
		if( Solver1d::Run(SubRgns[i], Err) == GSL_SUCCESS) 
		{		
			Roots << Solve r;
		}
		total_func_calls += cntr.func_call;
	}
	dt=Timer1.StopStart(); cntr.func_call = total_func_call;
	p->DestroyBuffers(); 
	return status;
}


//////////////////////////////////////////////////////////////////////////
int MultiDimMinimizer::Run(DoubleArray& initX, DoubleArray& initdX, SolverErrors Err)
{
	MyTimer Timer1; Timer1.Start();
	double size; X = initX; dX = initdX;
	CleanUp();
	s = gsl_multimin_fminimizer_alloc (fminimizer_type, initX.GetSize());
	gsl_multimin_fminimizer_set (s, &F, X, dX);
	do
	{
		cntr.iter++;
		status = gsl_multimin_fminimizer_iterate(s);
		if (status) break;
		size = gsl_multimin_fminimizer_size (s);
		status = gsl_multimin_test_size (size, Err.abs);
	}
	while (status == GSL_CONTINUE && cntr.iter < max_iter);
	if (status == GSL_SUCCESS)
	{
		Minimum.x = *(s->x); Minimum.y = s->fval;
	}
	dt = Timer1.StopStart();
	return status;
}

void MultiDimMinimizer::CleanUp()
{
	if (s != NULL)		{gsl_multimin_fminimizer_free (s); s = NULL;}
	if (X != NULL)		{gsl_vector_free(X); X = NULL;}
	if (dX != NULL)	{gsl_vector_free(dX); dX = NULL;}
	Minimum.CleanUp();
	PerfomanceInfoMk1::CleanUp();
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
	MyTimer Timer1; Timer1.Start();
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

