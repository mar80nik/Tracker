#include "stdafx.h"
#include "my_gsl.h"

DoubleArray& DoubleArray::operator=(const DoubleArray& arr )
{
	RemoveAll();
	SetSize(arr.GetSize());
	for(int i=0; i<arr.GetSize();i++) Add(arr[i]);
	return *this;
}

void DoubleArray::operator=( const gsl_vector& vector )
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
	ret = gsl_vector_alloc (size); ASSERT(0);

	for (int i = 0; i < size; i++)  
		gsl_vector_set (ret, i, arr[i]);
	return ret;
}

void DoubleArray::Serialize( CArchive& ar )
{
	int i, n; double t;
	if(ar.IsStoring())
	{
		n = GetSize(); ar << n; 
		for(i = 0; i < n; i++) { t = operator[](i); ar << t; }
	}
	else
	{		
		RemoveAll(); ar >> n; 
		for(i = 0; i < n; i++) { ar >> t; Add(t); }
	}
}

BOOL DoubleArray::operator==(const CArray<double> &ref)
{
	if (GetSize() != ref.GetSize()) return FALSE;
	for (int i = 0; i < GetSize(); i++)
	{
		if (operator[](i) != ref[i]) return FALSE;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
template <class FuncParams>
Solver1dTemplate<FuncParams>::Solver1dTemplate(SolverRegime _rgm, int _max_iter): 
	SolverData(_max_iter), rgm(_rgm)
{ 
	fsolver_type=gsl_root_fsolver_brent; s = NULL; 		
	F.function = func; F.params = this; 
	subrgns_max = 50;
}

template <class FuncParams>
int Solver1dTemplate<FuncParams>::Run (FuncParams* _params, BoundaryConditions _X, SolverErrors Err)
{
	MyTimer Timer1; double r; params = _params; ASSERT(params);
	Timer1.Start(); CleanUp(); 
	params->PrepareBuffers(); FindSubRgns(_X, SubRgns); 
	s = gsl_root_fsolver_alloc (fsolver_type);

	for(int i = 0; i < SubRgns.GetSize(); i++)
	{
		BoundaryConditions& X = SubRgns[i]; callback.CleanUp(); iter = 0;
		gsl_root_fsolver_set (s, &F, X.min, X.max);
		do
		{
			iter++;
			status = gsl_root_fsolver_iterate (s);
			X.min = gsl_root_fsolver_x_lower (s); X.max = gsl_root_fsolver_x_upper (s);
			status = gsl_root_test_interval (X.min, X.max, Err.abs, Err.rel);
		}
		while (status == GSL_CONTINUE && iter < max_iter);	

		if( status == GSL_SUCCESS) 
		{		
			Roots << gsl_root_fsolver_root(s);
		}
		cntr.iter += iter; cntr.func_call += params->func_call_cntr; 
	}
	dt=Timer1.StopStart(); params->DestroyBuffers();
	return status;
}
template <class FuncParams>
void Solver1dTemplate<FuncParams>::CleanUp()
{
	SolverData::CleanUp();
	if (s != NULL) { gsl_root_fsolver_free(s); s = NULL; }
	Roots.RemoveAll(); params->CleanUp(); SubRgns.RemoveAll();	
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
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
template <class FuncParams>
MultiDimMinimizerTemplate<FuncParams>::MultiDimMinimizerTemplate( int _max_iter/*=100*/ ) : 
	SolverData(_max_iter)
{
	fminimizer_type = gsl_multimin_fminimizer_nmsimplex; 
	s = NULL; X = dX = NULL; 
	F.f = func; F.params = this;
}
template <class FuncParams>
int MultiDimMinimizerTemplate<FuncParams>::Run(FuncParams* _params, DoubleArray& initX, DoubleArray& initdX, SolverErrors Err)
{
	MyTimer Timer1; double size; X = initX; dX = initdX; F.n = initX.GetSize(); params = _params; ASSERT(params);
	Timer1.Start();
	CleanUp(); params->PrepareBuffers();
	s = gsl_multimin_fminimizer_alloc (fminimizer_type, F.n);
	gsl_multimin_fminimizer_set (s, &F, X, dX); iter = 0;
	do
	{
		cntr.iter++;
		status = gsl_multimin_fminimizr_iterate(s);
		if (status) break;
		size = gsl_multimin_fminimizer_size (s);
		status = gsl_multimin_test_size (size, Err.abs);
	}
	while (status == GSL_CONTINUE && iter < max_iter);
	if (status == GSL_SUCCESS)
	{
		Roots.x = *(s->x); minimum_value = s->fval;
	}		
	cntr.func_call = params->func_call_cntr; params->DestroyBuffers();
	dt = Timer1.StopStart();
	return status;
}
template <class FuncParams>
void MultiDimMinimizerTemplate<FuncParams>::CleanUp()
{
	SolverData::CleanUp();
	if (s != NULL)		{gsl_multimin_fminimizer_free (s); s = NULL;}
	if (X != NULL)		{gsl_vector_free(X); X = NULL;}
	if (dX != NULL)	{gsl_vector_free(dX); dX = NULL;}
	Roots.CleanUp(); params->CleanUp();	
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
ComplexGSL sqrt( ComplexGSL& c )	{ return ComplexGSL(gsl_complex_sqrt(c.z)); }
ComplexGSL pow2( ComplexGSL& c )	{ return ComplexGSL(gsl_complex_mul(c.z,c.z)); }
ComplexGSL exp( ComplexGSL& c )		{ return ComplexGSL(gsl_complex_exp(c.z)); }
//////////////////////////////////////////////////////////////////////////
//int MultiFitter::MainFDF()
//{	
//
//	x = gsl_vector_view_array (start, p);	
//
//	multifit_fdfsolver_type = gsl_multifit_fdfsolver_lmsder;
//	s = gsl_multifit_fdfsolver_alloc (multifit_fdfsolver_type, n, p);
//	gsl_multifit_fdfsolver_set (s, &F, &x.vector);
//	do
//	{
//		iter++;
//		status = gsl_multifit_fdfsolver_iterate (s);
//		status = gsl_multifit_test_delta (s->dx, s->x, epsabs, epsrel);
//	}
//	while (status == GSL_CONTINUE && iter < max_iter);
//    return status;	
//}

template <class FuncParams>
void MultiFitterTemplate<FuncParams>::CleanUp()
{
	SolverData::CleanUp();
	if (s != NULL) { gsl_multifit_fdfsolver_free (s); s = NULL; }
	if (initX != NULL) { gsl_vector_free(initX); initX = NULL; }
	a.RemoveAll(); da.RemoveAll();
}

template <class FuncParams>
int MultiFitterTemplate<FuncParams>::Run( FuncParams* params, DoubleArray& init_a, SolverErrors Err )
{
	Timer1.Start();
	CleanUp(); params->PrepareBuffers();

	F.p = p = init_a.GetSize(); F.n = n = params->GetPointsNum(); initX = init_a;
	s = gsl_multifit_fdfsolver_alloc (multifit_fdfsolver_type, n, p);
	gsl_multifit_fdfsolver_set (s, &F, initX);
	do
	{
		cntr.iter++;
		status = gsl_multifit_fdfsolver_iterate (s);
		status = gsl_multifit_test_delta (s->dx, s->x, Err.abs, Err.rel);
	}
	while (status == GSL_CONTINUE && iter < max_iter);

	if (status == GSL_SUCCESS)
	{
		gsl_matrix *covar = gsl_matrix_alloc(p, p);
		if (covar != NULL)
		{
			gsl_multifit_covar(s->J, 0.0, covar);
			double c = GSL_MAX_DBL(1, gsl_blas_dnrm2(s->f) / sqrt(n - p)); 
			a = *(s->x); da = (gsl_matrix_diagonal (covar)).vector;
			for (size_t i = 0; i < da.GetSize(); i++)
			{
				da[i] = fabs(c*sqrt(da[i]));
			}
			gsl_matrix_free(covar);
		}
//		roots.leftmostX=fparams.leftmostX; roots.rightmostX=fparams.rightmostX; 
	//	roots.status=status; roots.dx=fparams.dx;					
	}	
	cntr.func_call = params->func_call_cntr;
	dt=Timer1.StopStart(); params->DestroyBuffers();
	return status;
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

