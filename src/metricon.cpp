#include "stdafx.h"
#include "metricon.h"

//////////////////////////////////////////////////////////////////////////
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
CalibrationParams& CalibrationParams::operator=(const CalibrationParams& t )
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
int CalibrationParams::CalculateFrom(	DoubleArray& _Nexp, DoubleArray& _teta, 
										double n_p, double n_i, double n_s, double alfa, double lambda)
{
	Calculator::FuncParams params(_Nexp, _teta, n_p, n_i, n_s, alfa, lambda);
	Solver1dTemplate<CalibrationParams::Calculator::FuncParams> FindFI(SINGLE_ROOT);
	CleanUp();
	if( FindFI.Run(&params, BoundaryConditions(-45*DEGREE, 45*DEGREE), SolverErrors(1e-12)) == GSL_SUCCESS) 
	{
		double *N = params.N, *A = params.A, *B = params.B;
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
	if (this->IsValidCalibration() == FALSE)
	{
		ret.status = E_FAIL;
		return ret;
	}
	PixelToAngleSolver::FuncParams params(Npix, this->val);
	Solver1dTemplate<PixelToAngleSolver::FuncParams> FindTETA(SINGLE_ROOT);	

	if ((ret.status = FindTETA.Run(&params, BoundaryConditions(35*DEGREE, 68.*DEGREE), 
		SolverErrors(1e-12))) == GSL_SUCCESS) 
	{
		ret.teta = FindTETA.Roots[0];
		ret.cal = params.cal; ret.Npix = Npix;
		*((SolverData*)(&ret)) = *((SolverData*)&FindTETA);
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
double FilmParams::FuncParams::func(const gsl_vector * x)
{
    double ret=100; int status;	DoubleArray film; film = *x;

	Solver1dTemplate<FilmParams::DispEqSolver::FuncParams> FindBettas(MULTI_ROOT);
	FilmParams::DispEqSolver::FuncParams params(pol, n_i, film[index_n], n_s, k*film[index_H]);

	if ((status = FindBettas.Run(&params, BoundaryConditions(n_s, film[index_n]), SolverErrors(1e-6))) == GSL_SUCCESS) 
	{
		ret = 0;
		int i, j, roots_n = FindBettas.Roots.GetSize(), betta_n = betta_exp.GetSize(); 
		for (i = 0; i <= roots_n - betta_n; i++)
		{
			for (j = 0; j < betta_n; j++)
			{
				double a = FindBettas.Roots[j + i];
				double b = betta_exp[j].teta;
				ret += abs(FindBettas.Roots[j + i] - betta_exp[j].teta);
			}
			betta_teor.RemoveAll();
			for (j = 0; j < betta_n; j++)
			{
				betta_teor << betta_info(FindBettas.Roots[j + i], j + i);
			}
		}
	}
	return ret;
}
void FilmParams::FuncParams::CleanUp()
{	
	betta_exp.RemoveAll(); betta_teor.RemoveAll(); 
}
int FilmParams::Calculator(	const Polarization pol, const CalibrationParams &cal, 
						   const TypeArray<AngleFromCalibration> &tetaexp,
						   FilmParams initX, FilmParams initdX)
{
	CleanUp();
	MultiDimMinimizerTemplate<FuncParams> FindFilmParams(200);
	FuncParams params(pol, tetaexp, cal);
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
	
	X0 << initX.n << initX.H; dX0 << initdX.n << initdX.H;

	if (FindFilmParams.Run(&params, X0, dX0, SolverErrors(1e-6)) == GSL_SUCCESS) 
	{		
		n = FindFilmParams.Roots[index_n]; H = FindFilmParams.Roots[index_H]; 
		minimum_value = FindFilmParams.minimum_value;
		betta_teor = params.betta_teor;
		for (int i = 0; i < params.betta_exp.GetSize(); i++)
		{
			betta_exp << params.betta_exp[i].teta;
		}		
		*((SolverData*)(this)) = *((SolverData*)&FindFilmParams);
	}
	return FindFilmParams.status;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
typedef void (*CalcRHelper)(const ComplexGSL &gam2, const ComplexGSL &gam3, const ComplexGSL &gam4,
							const ComplexGSL &epsi, const ComplexGSL &epsf, const ComplexGSL &epss,
							const double &CT, const double &Np,
							ComplexGSL &A, ComplexGSL &B, ComplexGSL &C);

void CalcRTEhelper(const ComplexGSL &gam2, const ComplexGSL &gam3, const ComplexGSL &gam4,
				   const ComplexGSL &epsi, const ComplexGSL &epsf, const ComplexGSL &epss,
				   const double &CT, const double &Np,
				   ComplexGSL &A, ComplexGSL &B, ComplexGSL &C)
{
	ComplexGSL t, t1;
	t = gam2;			t1 = gam3;	A = (t - t1)/(t + t1); 
	t = gam3;			t1 = gam4;	B = (t - t1)/(t + t1); 			
	t = gam2; t *= cJ;	t1 = CT;	C = (t - t1)/(t + t1); 
	C*=-1;
}

void CalcRTMhelper(const ComplexGSL &gam2, const ComplexGSL &gam3, const ComplexGSL &gam4,
				   const ComplexGSL &epsi, const ComplexGSL &epsf, const ComplexGSL &epss,
				   const double &CT, const double &Np,
				   ComplexGSL &A, ComplexGSL &B, ComplexGSL &C)
{
	ComplexGSL t, t1;
	t = epsf;	t *= gam2;	t1 = epsi;	t1 *= gam3;	
		A = (t - t1)/(t + t1);
	t = epss;	t *= gam3;	t1 = epsf;	t1 *= gam4;	
		B = (t - t1)/(t + t1);
	t = epsi;	t *= CT;	t1 = gam2; t1 *= ComplexImGSL(Np*Np);	
		C = (t - t1)/(t + t1);	
}

CalcR_ResultArray CalcR(const Polarization pol, const CalcRParams& params)
{
	CalcR_ResultArray ret; CalcRHelper func;
	double k, teta, ST, ST2, CT, dteta;
	ComplexGSL gam2, gam3, gam4, A, B, C, t, t1, Ax, Ra;
	const double &Np = params.Np; const FilmParams &i=params.i, &f=params.f, &s=params.s;

	switch (pol)
	{
	case TE: func = CalcRTEhelper; break;
	case TM: func = CalcRTMhelper; break;
	}

	k = 2*M_PI/params.lambda; dteta=(params.teta_max-params.teta_min)/(params.num_pnts-1);

	ComplexGSL epsi(i.n, i.m), epsf(f.n, f.m), epss(s.n, s.m);
	epsi=pow2(epsi); epsf=pow2(epsf); epss=pow2(epss);
	
	for(int j=0; j<params.num_pnts; j++)
	{
		teta = params.teta_min + j*dteta;
		ST = Np*sin(teta*DEGREE); ST2 = ST*ST; CT = Np*cos(teta*DEGREE);

		gam2 = sqrt((epsi-ST2)*-1.);	gam3 = sqrt((epsf-ST2)*-1.);	gam4 = sqrt((epss-ST2)*-1.); 
		func(gam2, gam3, gam4, epsi, epsf, epss, CT, Np, A, B, C);
		t = B*exp(gam3*(-2*k*f.H));		Ax = ( A + t )/( A*t + 1. );
		t = Ax*exp(gam2*(-2*k*i.H));	Ra = ( C + t )/( C*t + 1 );
		
		ret << CalcR_Result(teta, Ra.abs2(), ST);
	}
	return ret;	
}
/////////////////////////////////////////////////////////////////
////////////    f = a*x*x + b*x + c    //////////////////////////
/////////////////////////////////////////////////////////////////
double ParabolaFuncParams::func( const double &x, const double *a, const size_t &p )
{
	return a[ind_a]*x*x + a[ind_b]*x + a[ind_c];
}
double ParabolaFuncParams::df_da( const double &x, const double *a, const size_t &p, double *c) { return x*x; }
double ParabolaFuncParams::df_db( const double &x, const double *a, const size_t &p, double *c) { return x; }
double ParabolaFuncParams::df_dc( const double &x, const double *a, const size_t &p, double *c) { return 1; }
double ParabolaFitFunc::GetTop(double &x)
{
	x = (-a[ParabolaFuncParams::ind_b]/(2*a[ParabolaFuncParams::ind_a])); 
	return GetXrelY(x);
}
/////////////////////////////////////////////////////////////////
////////////   f = C + (A/(1 + exp(-2*k*(x - B))))   ////////////
/////////////////////////////////////////////////////////////////
double KneeFuncParams::func( const double &x, const double *a, const size_t &p )
{
	return a[ind_C] + (a[ind_A]/( 1 + exp(-2*a[ind_k]*(x - a[ind_B])) ));	
}
double KneeFuncParams::df_dA(const double &x, const double *a, const size_t &p, double *c)
{
	double t0 = *c;	return 1/(1 + t0);	
}
double KneeFuncParams::df_dB(const double &x, const double *a, const size_t &p, double *c)
{
	double t0 = *c;	return -2*a[ind_k]*a[ind_A]*t0/((1 + t0)*(1 + t0));
}
double KneeFuncParams::df_dC(const double &x, const double *a, const size_t &p, double *c) { return 1; }
double KneeFuncParams::df_dk(const double &x, const double *a, const size_t &p, double *c)
{
	double t0 = *c; return 2*(x - a[ind_B])*a[ind_A]*t0/((1 + t0)*(1 + t0));
}
double * KneeFuncParams::PrepareDerivBuf( const double &x, const double *a, const size_t &p )
{
	buf = exp(-2*a[ind_k]*(x - a[ind_B]));
	return &buf;
}

double KneeFitFunc::GetInflection( double &x, const double &level )
{
	x = a[KneeFuncParams::ind_B] + log(level/(1-level))/(2*a[KneeFuncParams::ind_k]);
	return GetXrelY(x);
}
//////////////////////////////////////////////////////////////////////////
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
