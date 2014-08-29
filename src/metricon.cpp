#include "stdafx.h"
#include "metricon.h"

int CalibrationSolver::func_call_cntr=0;
int CalibratorSolver::func_call_cntr=0;
int DispEqTESolver::func_call_cntr=0;
int DispEqTMSolver::func_call_cntr=0;
int FilmMinimizerTE::func_call_cntr=0;
int FilmMinimizerTM::func_call_cntr=0;
int Solver1dTemplate<DispEqTEFuncParams>::func_call_cntr=0;
int Solver1dTemplate<DispEqTMFuncParams>::func_call_cntr=0;
int Fit_Ax2BxC::func_call_cntr=0;
int Fit_Knee::func_call_cntr=0;
//////////////////////////////////////////////////////////////////////////
double CalibrationSolver::func( double x, void *params )
{
	struct CalibrationFuncParams *p = (CalibrationFuncParams *) params;
	double *A = p->GetA(), *B = p->GetB(), *N = p->N, *teta=p->teta;
	double &n_p=p->n_p, &n_s=p->n_s, &alfa=p->alfa, &fi=x;
	double ret;

	for(int i=0;i<p->size; i++)
	{
		double t=(n_p/n_s)*sin(alfa - teta[i]);
		A[i] = t / (cos(fi)*sqrt(1.-t*t)-sin(fi)*t);
		B[i] = sin(alfa)*tan(alfa - teta[i])/cos(fi)*(sin(fi)*A[i] + 1.);
	}
	ret =	((N[1]-N[3])*(A[0]-A[3]) - (N[0]-N[3])*(A[1]-A[3])) * 
		((B[0]-B[3])*(A[2]-A[3]) - (B[2]-B[3])*(A[0]-A[3])) - 
		((N[2]-N[3])*(A[0]-A[3]) - (N[0]-N[3])*(A[2]-A[3])) * 
		((B[0]-B[3])*(A[1]-A[3]) - (B[1]-B[3])*(A[0]-A[3]));
	func_call_cntr++;
	return ret;
}
//////////////////////////////////////////////////////////////////////////
int CreateCalibration( DoubleArray& Nexp, DoubleArray& teta, CalibrationParams& cal )
{
	CalibrationFuncParams in_params(Nexp.GetSize(), Nexp, teta, cal.n_p, cal.n_s, cal.alfa );
	CalibrationSolver FindFI( in_params );
	if( (cal.status=FindFI.Run(-45*DEGREE, 45*DEGREE, 1e-12))==GSL_SUCCESS ) 
	{
		double *A = FindFI.fparams.GetA(), *B = FindFI.fparams.GetB(), *N = FindFI.fparams.N;

		cal.fi0=FindFI.root;
		cal.L =	((N[1] - N[3])*(A[0] - A[3]) - (N[0] - N[3])*(A[1] - A[3])) / 
			((B[0] - B[3])*(A[1] - A[3]) - (B[1] - B[3])*(A[0] - A[3])); 
		cal.d0 = ((N[3] - N[0]) - cal.L*(B[0] - B[3]))/(A[0] - A[3]);
		cal.N0 = N[0] + cal.d0*A[0] + cal.L*B[0];

		cal.dt=FindFI.dt;
		cal.func_call_cntr=CalibrationSolver::func_call_cntr;
		cal.epsabs=FindFI.epsabs; cal.epsrel=FindFI.epsrel;
		cal.Nexp.RemoveAll(); cal.teta.RemoveAll();
		for(int i=0;i<Nexp.GetSize();i++) cal.Nexp.Add(N[i]);
		for(int i=0;i<teta.GetSize();i++) cal.teta.Add(teta[i]);
	}	
	return cal.status;
}
//////////////////////////////////////////////////////////////////////////
double CalibratorSolver::func( double x, void *params )
{
	struct CalibratorFuncParams *p = (CalibratorFuncParams *) params;
	double &n_p=p->cal.n_p, &n_s=p->cal.n_s, &alfa=p->cal.alfa, &tetax=x, &Npix=p->Npix;
	double &fi0=p->cal.fi0, &N0=p->cal.N0, &d0=p->cal.d0, &L=p->cal.L;
	double par1, par2, ret;

	double t=(n_p/n_s)*sin(alfa - tetax);
	par1 = t / (cos(fi0)*sqrt(1-t*t)-sin(fi0)*t);
	par2 = sin(alfa)*tan(alfa - tetax)/cos(fi0)*(sin(fi0)*par1 + 1);
	ret = Npix - (N0 - d0*par1 - L*par2);
	func_call_cntr++;
	return ret;
}
//////////////////////////////////////////////////////////////////////////
int Calibrator( CalibratorParams& params, CalibrationParams& cal )
{
	CalibratorSolver FindTETA(CalibratorFuncParams(params.Npix,cal));
	if( (params.status=FindTETA.Run(35*DEGREE, 68.*DEGREE, 1e-12)) ==GSL_SUCCESS) 
	{
		params.teta=FindTETA.root/DEGREE;
		params.betta=cal.n_p*sin(FindTETA.root);
		params.dt=FindTETA.dt;
		params.func_call_cntr=CalibratorSolver::func_call_cntr;
		params.epsabs=FindTETA.epsabs; params.epsrel=FindTETA.epsrel;
	}
	return params.status;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
double Solver1dTemplate<DispEqTEFuncParams>::func( double x, void *params ) {return 0;}
double DispEqTESolver::func( double x, void *params )
{
	struct DispEqFuncParams *p = (DispEqFuncParams *) params;
	double gam2, gam3, gam4; double ret; double &betta=x;
	double &n1=p->n1, &n2=p->n2, &n3=p->n3, &kHc=p->kHf;

	gam2 = sqrt(betta*betta - n1*n1); gam3 = sqrt(n2*n2 - betta*betta); gam4 = sqrt(betta*betta - n3*n3);
	ret = (gam3*gam3 - gam2*gam4)*sin(gam3*kHc) - gam3*(gam2 + gam4)*cos(gam3*kHc);
	func_call_cntr++;
	return ret;
}
double Solver1dTemplate<DispEqTMFuncParams>::func( double x, void *params ) {return 0;}
double DispEqTMSolver::func( double x, void *params )
{
	struct DispEqFuncParams *p = (DispEqFuncParams *) params;
	double gam2, gam3, gam4; double ret; double &betta=x;
	double &n1=p->n1, &n2=p->n2, &n3=p->n3, &kHc=p->kHf; double t1,t2,t3;

	gam2 = sqrt(betta*betta - n1*n1); gam3 = sqrt(n2*n2 - betta*betta); gam4 = sqrt(betta*betta - n3*n3);
    t1=n1*n1; t2=n2*n2; t3=n3*n3;
	ret = ( gam3*gam3*t1*t3 - gam2*gam4*t2*t2 )*sin(gam3*kHc) - gam3*t2*(gam2*t3 + gam4*t1)*cos(gam3*kHc);
	func_call_cntr++;
	return ret;
}
//////////////////////////////////////////////////////////////////////////
double FilmMinimizerTE::func(const gsl_vector * x, void * params)
{
    double ret=100; int status;
	FilmFuncParams* p=(FilmFuncParams*)params;
	double &n1=p->n1, &n3=p->n3, &k=p->k, *bettaexp=p->bettaexp;
	FilmParams film(x);

	DispEqTESolver Solver(DispEqTEFuncParams(n1, film.n, n3, k*film.H));
	Solver.min_roots=p->bettaexp.GetSize(); Solver.max_roots=Solver.min_roots+2;
	if( (status=Solver.Run(n3,film.n, 1e-6)) == GSL_SUCCESS) 
	{
		int i,j,roots_n=Solver.roots.GetSize(),betta_n=Solver.min_roots; double cur_ret;
		for(i=0;i<=roots_n-betta_n;i++)
		{
			cur_ret=0;			
			for(j=0;j<betta_n;j++)
			{
				cur_ret+=abs(Solver.roots[j+i]-bettaexp[j]);
			}
			if(cur_ret<ret) 
			{
				ret=cur_ret;
				p->betta_teor.RemoveAll(); betta_info t;
				for(j=0;j<betta_n;j++)
				{
					t.val=Solver.roots[j+i]; t.n=j+i; p->betta_teor.Add(t);
				}
			}
		}
	}
	func_call_cntr+=DispEqTESolver::func_call_cntr;
	return ret;
}

double FilmMinimizerTM::func(const gsl_vector * x, void * params)
{
	double ret=100; int status;
	FilmFuncParams* p=(FilmFuncParams*)params;
	double &n1=p->n1, &n3=p->n3, &k=p->k, *bettaexp=p->bettaexp;
	FilmParams film(x);

	DispEqTMSolver Solver(DispEqTMFuncParams(n1, film.n, n3, k*film.H));
	if( (status=Solver.Run(n3,film.n, 1e-6)) ==GSL_SUCCESS) 
	{
		int i,j,roots_n=Solver.roots.GetSize(),betta_n=Solver.min_roots; double cur_ret;
		for(i=0;i<=roots_n-betta_n;i++)
		{
			cur_ret=0;			
			for(j=0;j<betta_n;j++)
			{
				cur_ret+=abs(Solver.roots[j+i]-bettaexp[j]);
			}
			if(cur_ret<ret) 
			{
				ret=cur_ret;
				p->betta_teor.RemoveAll(); betta_info t;
				for(j=0;j<betta_n;j++)
				{
					t.val=Solver.roots[j+i]; t.n=j+i; p->betta_teor.Add(t);
				}
			}
		}
	}
	func_call_cntr+=DispEqTMSolver::func_call_cntr;
	return ret;
}
//////////////////////////////////////////////////////////////////////////

int CalclFilmParamsTE(FilmFuncTEParams& in, FilmParams& out)
{
	FilmMinimizerTE Minimizer(in,200);	
//	out.status=Minimizer.Run(FilmParams(in.bettaexp[0],1430), FilmParams(1e-4,1e-1), 1e-6);
	out.status=Minimizer.Run(FilmParams(1.8,1150), FilmParams(1e-4,1e-1), 1e-6);
	out=Minimizer.roots;
	out.dt=Minimizer.dt;
	out.func_call_cntr=FilmMinimizerTE::func_call_cntr;
	out.epsabs=Minimizer.epsabs; 
	out.fval=Minimizer.fval; out.size=Minimizer.size;
	return out.status;
}

int CalclFilmParamsTM(FilmFuncTMParams& in, FilmParams& out)
{
	FilmMinimizerTM Minimizer(in,200);		
//	out.status=Minimizer.Run(FilmParams(in.bettaexp[0],1020.), FilmParams(1e-4,1e-1), 1e-6);
	out.status=Minimizer.Run(FilmParams(1.8,1250.), FilmParams(1e-4,1e-1), 1e-6);
	out=Minimizer.roots;
	out.dt=Minimizer.dt;
	out.func_call_cntr=FilmMinimizerTM::func_call_cntr;
	out.epsabs=Minimizer.epsabs; 
	out.fval=Minimizer.fval; out.size=Minimizer.size;
	return out.status;
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

		t=B*exp(gam3*(-2*k*f.H));
		As=( A + t )/( A*t + 1. );

		t=As*exp(gam2*(-2*k*i.H));
		Ra=( C + t )/( C*t + 1 );
		
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
int Fit_Parabola( MultiFitterFuncParams& in, Fit_Ax2BxCParams init,Fit_Ax2BxCParams& out )
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


double Fit_Ax2BxCParams::f( double x, double* a )
{
	double ret=a[0],tx=x;
	for(int j=1;j<3;j++) {ret+=(a[j]*tx); tx*=x;}
	return ret;
}

Fit_Ax2BxCParams::Fit_Ax2BxCParams( double _a/*=0*/,double _b/*=0*/,double _c/*=0*/ ) : FittingPerfomanceInfo(GetParamsNum())
{
	a[2]=_a; a[1]=_b; a[0]=_c;
}

SimplePoint Fit_Ax2BxCParams::GetTop()
{
	double t=(-a[1]/(2*a[2]));
	return SimplePoint(leftmostX+t*dx, a[0]-a[1]*a[1]/(a[2]*4));
}

SimplePoint Fit_Ax2BxCParams::GetXabsY( double x )
{
	return SimplePoint(x,f((x-leftmostX)/dx,a));
}

SimplePoint Fit_Ax2BxCParams::GetXrelY( double x )
{
	return SimplePoint(x+leftmostX,f(x,a));
}

//////////////////////////////////////////////////////////////////////////
int Fit_Ax2BxC::f (const gsl_vector * x, void *data, gsl_vector * f)
{
	MultiFitterFuncParams *params=(MultiFitterFuncParams *)data;
	size_t& n = params->n, p = x->size, i; double Yi;
	double *y = params->y, *sigma = params->sigma, *a=x->data;

	for (i = 0; i < n; i++)
	{
		Yi=Fit_Ax2BxCParams::f(i,a);
		gsl_vector_set (f, i, (Yi - y[i])/sigma[i]);
	}
	func_call_cntr++;
	return GSL_SUCCESS;
}
int Fit_Ax2BxC::df (const gsl_vector * x, void *data, gsl_matrix * J)
{
	MultiFitterFuncParams *params=(MultiFitterFuncParams *)data;
	size_t& n = params->n, p = x->size, i,j;
	double *sigma = params->sigma, *a=x->data;

	for (i = 0; i < n; i++)
	{
		double tx=i; double s = sigma[i];
		gsl_matrix_set (J, i, 0, 1/s);		
		for(j=1;j<p;j++)
		{
			gsl_matrix_set (J, i, j, tx/s); tx*=i;
		}
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

void CalibrationParams::Serialize( CArchive& ar )
{
	if(ar.IsStoring())
	{
		ar << alfa << n_p << n_s << N0 << L << d0 << fi0;
		ar << Nexp.GetSize(); for(int i=0;i<Nexp.GetSize();i++) ar << Nexp[i];
		ar << teta.GetSize(); for(int i=0;i<teta.GetSize();i++) ar << teta[i];
	}
	else
	{
		int n; double t;
		ar >> alfa >> n_p >> n_s >> N0 >> L >> d0 >> fi0;
		Nexp.RemoveAll(); ar >> n; for(int i=0;i<n;i++) { ar >> t; Nexp.Add(t); }
		teta.RemoveAll(); ar >> n; for(int i=0;i<n;i++) { ar >> t; teta.Add(t); }
	}
}

CalibrationParams& CalibrationParams::operator=( CalibrationParams& t )
{
	alfa=t.alfa; n_p=t.n_p; n_s=t.n_s;
	N0=t.N0; L=t.L; d0=t.d0; fi0=t.fi0;	//	variables
	Nexp.RemoveAll(); for(int i=0;i<t.Nexp.GetSize();i++) { Nexp.Add(t.Nexp[i]); }
	teta.RemoveAll(); for(int i=0;i<t.teta.GetSize();i++) { teta.Add(t.teta[i]); }
	return *this;
}
//////////////////////////////////////////////////////////////////////////
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

Fit_KneeParams::Fit_KneeParams( double _a/*=0*/,double _b/*=0*/,double _c/*=0*/,double _k/*=0*/ ) : 
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