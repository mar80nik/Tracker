#include "stdafx.h"
#include "metricon.h"

//////////////////////////////////////////////////////////////////////////
AngleFromCalibrationArray& AngleFromCalibrationArray::operator=( AngleFromCalibrationArray& arr )
{
	RemoveAll();
	SetSize(arr.GetSize());
	for(int i=0; i<arr.GetSize();i++) Add(arr[i]);
	return *this;
}
//////////////////////////////////////////////////////////////////////////
void CalibrationParams::Serialize( CArchive& ar )
{
	if(ar.IsStoring())
	{
		val.Serialize(ar);
		Nexp.Serialize(ar);  teta.Serialize(ar);
	}
	else
	{
		val.Serialize(ar);
		Nexp.Serialize(ar);  teta.Serialize(ar);	
	}
}

CalibrationParams& CalibrationParams::operator=( CalibrationParams& t )
{
	val = t.val; Nexp = t.Nexp; teta = t.teta;
	return *this;
}

double CalibrationParams::Calculator::FuncParams::func( double fi)
{
	double ret;
	for(int i = 0; i < size; i++)
	{
		double t=(n_p/n_s)*sin(alfa - teta[i]);
		A[i] = t / (cos(fi)*sqrt(1.-t*t)-sin(fi)*t);
		B[i] = sin(alfa)*tan(alfa - teta[i])/cos(fi)*(sin(fi)*A[i] + 1.);
	}
	ret =	((N[1]-N[3])*(A[0]-A[3]) - (N[0]-N[3])*(A[1]-A[3])) * 
		((B[0]-B[3])*(A[2]-A[3]) - (B[2]-B[3])*(A[0]-A[3])) - 
		((N[2]-N[3])*(A[0]-A[3]) - (N[0]-N[3])*(A[2]-A[3])) * 
		((B[0]-B[3])*(A[1]-A[3]) - (B[1]-B[3])*(A[0]-A[3]));
	return ret;
}

int CalibrationParams::CalculateFrom(	DoubleArray& Nexp, DoubleArray& teta, 
										double n_p, double n_i, double n_s, double alfa, double lambda)
{
	Calculator::FuncParams params(Nexp, teta, n_p, n_i, n_s, alfa, lambda);
	Solver1dTemplate<Calculator::FuncParams> FindFI(SINGLE_ROOT);

	if( FindFI.Run(&params, BoundaryConditions(-45*DEGREE, 45*DEGREE), SolverErrors(1e-12)) == GSL_SUCCESS) 
	{
		double *N = params.N, *A = params.A, *B = params.B;
		val.RemoveAll();
		val << params.alfa <<  params.n_p << params.n_i << params.n_s << params.lambda; 
		Nexp = params.N; teta = params.teta;
		val << FindFI.Roots[0]; 	
		val << ((N[1] - N[3])*(A[0] - A[3]) - (N[0] - N[3])*(A[1] - A[3])) / 
			((B[0] - B[3])*(A[1] - A[3]) - (B[1] - B[3])*(A[0] - A[3])); 
		val << ((N[3] - N[0]) - val[ind_L]*(B[0] - B[3]))/(A[0] - A[3]);
		val << N[0] + val[ind_d0]*A[0] + val[ind_L]*B[0];
		*((SolverData*)(this)) = *((SolverData*)&FindFI);
	}
	return FindFI.status;
}
//////////////////////////////////////////////////////////////////////////
double CalibrationParams::PixelToAngleSolver::FuncParams::func( double teta )
{
	double par1, par2, ret;	double t = (cal[ind_n_p]/cal[ind_n_s])*sin(cal[ind_alfa] - teta);	
	par1 = t / (cos(cal[ind_fi0])*sqrt(1-t*t) - sin(cal[ind_fi0])*t);
	par2 = sin(cal[ind_alfa])*tan(cal[ind_alfa] - teta)/cos(cal[ind_fi0])*(sin(cal[ind_fi0])*par1 + 1);
	ret = Npix - (cal[ind_N0] - cal[ind_d0]*par1 - cal[ind_L]*par2);
	return ret;
}

AngleFromCalibration CalibrationParams::ConvertPixelToAngle(double Npix)
{
	AngleFromCalibration ret;
	PixelToAngleSolver::FuncParams params(Npix, this->val);
	Solver1dTemplate<PixelToAngleSolver::FuncParams> FindTETA(SINGLE_ROOT);	

	if ((ret.status = FindTETA.Run(&params, BoundaryConditions(35*DEGREE, 68.*DEGREE), SolverErrors(1e-12))) == GSL_SUCCESS) 
	{
		ret.teta = FindTETA.Roots[0]/DEGREE;
		ret.cal = params.cal;
	}
	return ret;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
double FilmParams::DispEqSolver::FuncParams::funcTE( double betta )
{
	double gam2, gam3, gam4, ret;
	gam2 = sqrt(betta*betta - n_i*n_i); gam3 = sqrt(n_f*n_f - betta*betta); gam4 = sqrt(betta*betta - n_s*n_s);
	ret = (gam3*gam3 - gam2*gam4)*sin(gam3*kHf) - gam3*(gam2 + gam4)*cos(gam3*kHf);
	return ret;
}
double FilmParams::DispEqSolver::FuncParams::funcTM( double betta )
{
	double gam2, gam3, gam4, t1, t2, t3, ret; 
	t1 = n_i*n_i; t2 = n_f*n_f; t3=n_s*n_s;
	gam2 = sqrt(betta*betta - t1); gam3 = sqrt(t2 - betta*betta); gam4 = sqrt(betta*betta - t3);    
	ret = ( gam3*gam3*t1*t3 - gam2*gam4*t2*t2 )*sin(gam3*kHf) - gam3*t2*(gam2*t3 + gam4*t1)*cos(gam3*kHf);
	return ret;
}
//////////////////////////////////////////////////////////////////////////
double FilmParams::FuncParams::func(const gsl_vector * x)
{
    double ret=100; int status;	DoubleArray film; film = *x;

	DoubleArray cal = bettaexp[0].cal; 
	double k = 2*M_PI/cal[CalibrationParams::ind_lambda];
	double n_i = cal[CalibrationParams::ind_n_i], n_s = cal[CalibrationParams::ind_n_s];
	Solver1dTemplate<DispEqSolver::FuncParams> FindBettas(MULTI_ROOT);
	FilmParams::DispEqSolver::FuncParams params(pol, n_i, film[index_n], n_s, k*film[index_H]);

	if ((status = FindBettas.Run(&params, BoundaryConditions(n_s, film[index_n]), SolverErrors(1e-6))) == GSL_SUCCESS) 
	{
		double cur_ret; 
		int i, j, roots_n = FindBettas.Roots.GetSize(), betta_n = bettaexp.GetSize(); 
		for (i = 0; i <= roots_n - betta_n; i++)
		{
			cur_ret = 0;			
			for (j = 0; j < betta_n; j++)
			{
				cur_ret += abs(FindBettas.Roots[j + i] - bettaexp[j].teta);
			}
			if (cur_ret < ret) 
			{
				ret = cur_ret;
				betta_teor.RemoveAll();
				for (j = 0; j < betta_n; j++)
				{
					betta_teor.Add(betta_info(FindBettas.Roots[j + i], j + i));
				}
			}
		}
	}
	func_call_cntr += params.func_call_cntr;
	return ret;
}

void FilmParams::FuncParams::CleanUp()
{
	BaseForFuncParams::CleanUp(); bettaexp.RemoveAll(); betta_teor.RemoveAll(); 
}

//////////////////////////////////////////////////////////////////////////

int FilmParams::Calculator(	Polarization pol, AngleFromCalibrationArray& bettaexp,FilmParams initX, FilmParams initdX)
{
	MultiDimMinimizerTemplate<FuncParams> FindFilmParams(200);
	FuncParams params(pol,  bettaexp);
	DoubleArray X0, dX0; 
	switch (pol)
	{
	case TE: 
//		initX.n = bettaexp[0]; initX.H = 1430; 
		initX.n = 1.8; initX.H = 1150; 
		break;
	case TM:
//		initX.n = bettaexp[0]; initX.H = 1020; 
		initX.n = 1.8; initX.H = 1250; 
		break;
	}
	
	if (pol == TM) { initX.n = 1.8; initX.H = 1250; }
	X0 << initX.n << initX.H; dX0 << initdX.n << initdX.H;

	if (FindFilmParams.Run(&params, X0, dX0, SolverErrors(1e-6)) == GSL_SUCCESS) 
	{		
		n = FindFilmParams.Roots[index_n]; H = FindFilmParams.Roots[index_H]; 
		minimum_value = FindFilmParams.minimum_value;
		*((SolverData*)(this)) = *((SolverData*)&FindFilmParams);
	}
	return FindFilmParams.status;
}

void CalcR_TE(CalcRParams& params)
{
	SimplePoint pnt; pnt.type.Set(GenericPnt);
	double k, teta, ST, CT, dteta;
	ComplexGSL gam2, gam3, A, B, C, t, gam4, As, Ra;
	double &Np = params.Np; FilmParams &i=params.i, &f=params.f, &s=params.s;

	k = 2*M_PI/params.lambda; dteta=(params.teta_max-params.teta_min)/(params.num_pnts-1);

	ComplexGSL epsi(i.n, i.m), epsf(f.n, f.m), epss(s.n, s.m);
	epsi=pow2(epsi); epsf=pow2(epsf); epss=pow2(epss);
	
	for(int j=0; j<params.num_pnts; j++)
	{
		teta=params.teta_min+j*dteta;
		ST = Np*sin(teta*DEGREE); ST*=ST; CT = Np*cos(teta*DEGREE);

		gam2=sqrt((epsi-ST)*-1.); 
		gam3=sqrt((epsf-ST)*-1.); 
		gam4=sqrt((epss-ST)*-1.); 

		A=(gam2 - gam3)/(gam2 + gam3); 
		B=(gam3 - gam4)/(gam3 + gam4);
		t=gam2*cJ; C=(t - CT)/(t + CT); C*=-1;

		t=B*exp(gam3*(-2*k*f.H)); As=( A + t )/( A*t + 1. );

		t=As*exp(gam2*(-2*k*i.H)); Ra=( C + t )/( C*t + 1 );
		
		if(params.R!=NULL) { pnt.x=teta; pnt.y=Ra.abs2(); params.R->Points.Add(pnt); }
		if(params.teta!=NULL) { pnt.x=teta; pnt.y=sqrt(ST); params.teta->Points.Add(pnt); }
	}	
	
}

void CalcR_TM(CalcRParams& params)
{
	SimplePoint pnt; pnt.type.Set(GenericPnt);
	double k, teta, ST, CT, dteta;
	ComplexGSL gam2, gam3, A, B, C, t, t1, gam4, Ap, Ra;
	double &Np = params.Np; FilmParams &i=params.i, &f=params.f, &s=params.s;

	k = 2*M_PI/params.lambda; dteta=(params.teta_max-params.teta_min)/(params.num_pnts-1);

	ComplexGSL epsi(i.n, i.m), epsf(f.n, f.m), epss(s.n, s.m);
	epsi=pow2(epsi); epsf=pow2(epsf); epss=pow2(epss);

	for(int j=0; j<params.num_pnts; j++)
	{
		teta=params.teta_min+j*dteta;
		ST = Np*sin(teta*DEGREE); ST*=ST; CT = Np*cos(teta*DEGREE);

		gam2=sqrt((epsi-ST)*-1.); 
		gam3=sqrt((epsf-ST)*-1.); 
		gam4=sqrt((epss-ST)*-1.); 

		t=epsf*gam2; t1=epsi*gam3;				A = (t - t1)/(t + t1);
		t=epss*gam3; t1=epsf*gam4;				B = (t - t1)/(t + t1);
		t=epsi*CT; t1=gam2*ComplexImGSL(Np*Np); C = (t - t1)/(t + t1);

		t=B*exp(gam3*(-2*k*f.H));
		Ap=( A + t )/( A*t + 1. );		

		t=Ap*exp(gam2*(-2*k*i.H));
		Ra=( C + t )/( C*t + 1 );

		if(params.R!=NULL) { pnt.x=teta; pnt.y=Ra.abs2(); params.R->Points.Add(pnt); }
		if(params.teta!=NULL) { pnt.x=teta; pnt.y=sqrt(ST); params.teta->Points.Add(pnt); }
	}		
}

//////////////////////////////////////////////////////////////////////////
/*
int Fit_Parabola( MultiFitterFuncParams& in, Fit_Ax2BxCParams& init,Fit_Ax2BxCParams& out )
{
  
	Fit_Ax2BxC fit(in,10);	
	fit.Run(init, 1e-15, 1e-9); 
	out=fit.roots;
	out.dt=fit.dt;
	out.func_call_cntr=Fit_Ax2BxC::func_call_cntr;
	out.epsabs=fit.epsabs;  out.epsabs=fit.epsabs; 		
	out.iter_num=fit.iter;
	
	return out.status;
}
*/


void FittingPerfomanceInfo::GetSolverResults( gsl_multifit_fdfsolver *s )
{
	double chi, dof, c;	size_t p=s->x->size, n=s->f->size; 
	gsl_matrix *covar = gsl_matrix_alloc (p, p);
	if(covar!=NULL)
	{
		gsl_multifit_covar (s->J, 0.0, covar);
		chi = gsl_blas_dnrm2(s->f); dof = n - p; c = GSL_MAX_DBL(1, chi / sqrt(dof)); chisq_dof=chi*chi / dof;
		for(size_t i=0;i<p;i++)
		{
			a[i]=gsl_vector_get(s->x, i); 
			da[i]=fabs(c*sqrt(gsl_matrix_get(covar,i,i)));
		}
		gsl_matrix_free(covar);
	}
}
/*
FittingPerfomanceInfo::FittingPerfomanceInfo( int _p )
{
	a=da=NULL; p=_p;
	a=new double[p];
	da=new double[p];
	for(int i=0;i<p;i++) {a[i]=0; da[i]=0;}
	chisq_dof=leftmostX=rightmostX=0; n=0; dx=0;
}

FittingPerfomanceInfo::~FittingPerfomanceInfo()
{
	if(a!=NULL) delete[] a;	
	if(da!=NULL) delete[] da;
}

void FittingPerfomanceInfo::operator=( const FittingPerfomanceInfo& t )
{
	PerfomanceInfoMk1::operator=(t);
	chisq_dof=t.chisq_dof; leftmostX=t.leftmostX; rightmostX=t.rightmostX; dx=t.dx; n=t.n;
	ASSERT(p==t.p);
	for(int i=0;i<p;i++) {a[i]=t.a[i]; da[i]=t.da[i];}
}
*/



//Fit_Ax2BxC_FuncParams::Fit_Ax2BxC_FuncParams( double _a/*=0*/,double _b/*=0*/,double _c/*=0*/ ) : FittingPerfomanceInfo(GetParamsNum())
//{
//	a[2]=_a; a[1]=_b; a[0]=_c;
//}

SimplePoint Fit_Ax2BxC_FuncParams::GetTop()
{
//	double t=(-a[1]/(2*a[2]));
//	return SimplePoint(leftmostX+t*dx, a[0]-a[1]*a[1]/(a[2]*4));
	return 0;
}

SimplePoint Fit_Ax2BxC_FuncParams::GetXabsY( double x )
{
	//return SimplePoint(x, f((x - leftmostX)/dx,a));
		return 0;
}

SimplePoint Fit_Ax2BxC_FuncParams::GetXrelY( double x )
{
	//return SimplePoint(x + leftmostX,f(x,a));
		return 0;
}

//////////////////////////////////////////////////////////////////////////
double Fit_Ax2BxC_FuncParams::func( double x, double* a )
{
	double ret = a[ind_c], tx = x;
	for(size_t j = 1; j < p; j++) { ret += (a[j]*tx); tx *= x;}
	return ret;
}

int Fit_Ax2BxC_FuncParams::f (const gsl_vector * x, gsl_vector * f)
{
	for (size_t i = 0; i < n; i++)
	{
		gsl_vector_set (f, i, (func(i, x->data) - y[i])/sigma[i]);
	}
	return GSL_SUCCESS;
}
int Fit_Ax2BxC_FuncParams::df (const gsl_vector * x, gsl_matrix * J)
{
	for (size_t i = 0; i < n; i++)
	{
		double tx = i; double s = sigma[i];
		gsl_matrix_set (J, i, ind_c, 1/s);		
		gsl_matrix_set (J, i, ind_b, tx/s); tx *= i;
		gsl_matrix_set (J, i, ind_a, tx/s);
	}
	return GSL_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
int FourierFilter( FFTRealTransform::Params& in, double spec_width, FFTRealTransform::Params& out) // spec_width=0...100.
{
	int ret, i, j,  n; ms dt;
	FFTRealTransform Trasformer;

	ret=Trasformer.Run(in,FFTRealTransform::FORWARD);
    if(ret!=GSL_SUCCESS) return ret;
	
	j=(int)((n=in.y->GetSize())*spec_width/100.);
	for(i=j;i<n;i++) (*in.y)[i]=0.;

	dt+=in.dt;
	ret=Trasformer.Run(in,FFTRealTransform::BACKWARD);
	
	out=in; out.dt+=dt;
	return ret;

}

//////////////////////////////////////////////////////////////////////////
/*
int Fit_StepFunc( MultiFitterFuncParams& in, Fit_KneeParams& init,Fit_KneeParams& out )
{
	Fit_Knee fit(in,100);	
	fit.Run(init, 1e-15, 1e-9); 
	out=fit.roots;
	out.dt=fit.dt;
	out.func_call_cntr=Fit_Knee::func_call_cntr;
	out.epsabs=fit.epsabs;  out.epsabs=fit.epsabs; 		
	out.iter_num=fit.iter;

	return out.status;
}

void Fit_KneeParams::operator=( const Fit_KneeParams& t )
{
	FittingPerfomanceInfo::operator =(t);
}

double Fit_KneeParams::f( double x, double* a )
{
	double &A=a[0], &B=a[1], &C=a[2], &k=a[3];
	double ret=C+(A/( 1+exp(-2*k*(x-B)) ));	
	return ret;
}

//Fit_KneeParams::Fit_KneeParams( double _a,double _b,double _c,double _k) : 
	FittingPerfomanceInfo(GetParamsNum()), A(a[0]), B(a[1]), C(a[2]), k(a[3])
{
	A=_a; B=_b; C=_c; k=_k;
}

SimplePoint Fit_KneeParams::GetXabsY( double x )
{
	SimplePoint ret(x,f((x-leftmostX)/dx,a));
	return ret;
}

SimplePoint Fit_KneeParams::GetXrelY( double x )
{
	SimplePoint ret(x+leftmostX,f(x,a));
	return ret;
}

SimplePoint Fit_KneeParams::GetTop(double T)
{
	double &A=a[0], &B=a[1], &C=a[2], &k=a[3];
	double t=B+log(T/(1-T))/(2*k);
	return SimplePoint(leftmostX+t*dx, f(t,a));	
}

int Fit_Knee::f (const gsl_vector * x, void *data, gsl_vector * f)
{
	MultiFitterFuncParams *params=(MultiFitterFuncParams *)data;
	size_t& n = params->n, p = x->size, i; double Yi;
	double *y = params->y, *sigma = params->sigma, *a=x->data;

	for (i = 0; i < n; i++)
	{
		Yi=Fit_KneeParams::f(i,a);
		gsl_vector_set (f, i, (Yi - y[i])/sigma[i]);
	}
	func_call_cntr++;
	return GSL_SUCCESS;
}
int Fit_Knee::df (const gsl_vector * x, void *data, gsl_matrix * J)
{
	MultiFitterFuncParams *params=(MultiFitterFuncParams *)data;
	size_t& n = params->n, p = x->size, i;
	double *sigma = params->sigma;
	double &A=x->data[0], &B=x->data[1], &C=x->data[2], &k=x->data[3];

	for (i = 0; i < n; i++)
	{
		double s = sigma[i]; 
		double t0=exp(-2*k*(i-B)), t=1/(1+t0), t1=-A*t*t/s; 

		gsl_matrix_set (J, i, 0, t/s);		
		gsl_matrix_set (J, i, 1, 2*k*t1*t0);		
		gsl_matrix_set (J, i, 2, 1/s);		
		gsl_matrix_set (J, i, 3, (-2*(i-B))*t1*t0);		
	}
	return GSL_SUCCESS;
}
*/