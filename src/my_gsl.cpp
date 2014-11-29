#include "stdafx.h"
#include "my_gsl.h"

void DoubleArray::operator=( const gsl_vector& vector )
{
	double t; RemoveAll();
	for (size_t i = 0; i < vector.size; i++)  
	{
		t = gsl_vector_get (&vector, i);
		(*this).Add(t);
	}
}
gsl_vector* DoubleArray::CreateGSLReplica()
{
	gsl_vector* ret=NULL; double* arr = GetData(); int size = GetSize();
	ret = gsl_vector_alloc (size); ASSERT(ret);

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
//////////////////////////////////////////////////////////////////////////
BaseForMultiFitterFuncParams::
	BaseForMultiFitterFuncParams(const size_t _p, 
	const DoubleArray &_x, const DoubleArray &_y, const DoubleArray &_sigma): BaseForFuncParams(), p(_p)
{		
	ASSERT(_y.GetSize() ==_sigma.GetSize()); sigma = NULL;
	y = _y.GetData(); FillSigma(_sigma); n =_y.GetSize(); 
	ASSERT(n >= p);
	leftmostX = _x[0]; rightmostX = _x[n - 1];
	dx = (rightmostX - leftmostX)/(n - 1);
	pDerivatives = NULL; pFunction = NULL; pDerivatives = new pDerivFunc[p];
}
int BaseForMultiFitterFuncParams::f( const gsl_vector * x, gsl_vector * f )
{
	for (size_t i = 0; i < n; i++)
	{
		gsl_vector_set (f, i, (pFunction(i, x->data, p) - y[i])/sigma[i]);
	}
	return GSL_SUCCESS;
}
int BaseForMultiFitterFuncParams::df( const gsl_vector * x, gsl_matrix * J )
{
	double *c;
	for (size_t i = 0; i < n; i++)
	{
		c = PrepareDerivBuf(i, x->data, p);
		for (size_t j = 0; j < p; j++)
		{
			gsl_matrix_set (J, i, j, pDerivatives[j](i, x->data, p, c)/sigma[i]);		
		}
	}
	return GSL_SUCCESS;
}
BaseForMultiFitterFuncParams::~BaseForMultiFitterFuncParams()
{
	delete[] pDerivatives;
	if (sigma != NULL)
	{
		delete[] sigma; sigma = NULL;
	}
}

int BaseForMultiFitterFuncParams::FillSigma(const DoubleArray &_sigma)
{
	int ret = -1;
	if (sigma != NULL)
	{
		delete[] sigma; sigma = NULL;
	}
	if (_sigma.HasValues())
	{
		ret = 0;
		sigma = new double[_sigma.GetSize()];
		for (int i = 0; i < _sigma.GetSize(); i++)
		{
			if (_sigma[i] < 1e-10)
			{
				sigma[i] = 1e-10; ret++;
			}
			else
			{
				sigma[i] = _sigma[i];
			}
		}
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////
void BaseForFitFunc::InitFrom( const BaseForMultiFitterFuncParams &params )
{
	leftmostX = params.leftmostX; rightmostX = params.rightmostX; dx = params.dx; pFunction = params.pFunction;
}
void BaseForFitFunc::InitFrom( const SolverData &data ) { *((SolverData*)this) = *((SolverData*)&data); }
double BaseForFitFunc::GetXabsY( const double &x ) { return pFunction((x - leftmostX)/dx, a, a.GetSize()); }
double BaseForFitFunc::GetXrelY( double &x ) { double ret = pFunction(x, a, a.GetSize()); x += leftmostX; return ret; }
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
ComplexGSL sqrt( ComplexGSL& c )	{ return ComplexGSL(gsl_complex_sqrt(c.z)); }
ComplexGSL pow2( ComplexGSL& c )	{ return ComplexGSL(gsl_complex_mul(c.z,c.z)); }
ComplexGSL exp( ComplexGSL& c )		{ return ComplexGSL(gsl_complex_exp(c.z)); }
//////////////////////////////////////////////////////////////////////////
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


