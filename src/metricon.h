#pragma once

#include "TChart/TChartSeries.h"
#include "my_gsl.h"

enum Polarization {TE, TM};

//////////////////////////////////////////////////////////////////////////
struct FittingPerfomanceInfo: public PerfomanceInfoMk1
{	
protected:
	int p;
public:
	double *a,*da;
	double chisq_dof, leftmostX, rightmostX, dx;
	size_t n;

	FittingPerfomanceInfo(int p);
	virtual ~FittingPerfomanceInfo();
	void GetSolverResults(gsl_multifit_fdfsolver *s);	
	void operator= (const FittingPerfomanceInfo& t);
};
//////////////////////////////////////////////////////////////////////////
struct CalibrationParams: public SolverData
{
	class Calculator
	{
	public:
		struct FuncParams: public BaseForFuncParams
		{
			friend struct CalibrationParams;
		protected:
			double *A, *B; DoubleArray &N, &teta; double n_p, n_s, alfa, lambda;
			int size;
		public:			
			double func (double x); //This is callback function for GSL solver
			FuncParams(DoubleArray& _N, DoubleArray& _teta, double _n_p, double _n_s, double _alfa, double _lambda):
				N(_N), teta(_teta), n_p(_n_p), n_s(_n_s), alfa(_alfa), lambda(_lambda)
			{
				A = B = NULL; size = N.GetSize();
			}
			FuncParams(FuncParams& p):
				N(p.N), teta(p.teta), n_p(p.n_p), n_s(p.n_s), alfa(p.alfa) 
			{
				A = B = NULL; size = p.size;
			}
			virtual void PrepareBuffers() 
			{ 
				DestroyBuffers(); 
				A = new double[size]; B = new double[size];
			}
			virtual void DestroyBuffers()
			{
				if(A!=NULL) {delete[] A; A=NULL;}
				if(B!=NULL) {delete[] B; B=NULL;}
			}	
			virtual void CleanUp()
			{
				BaseForFuncParams::CleanUp();
				DestroyBuffers();
			}
		};
	};

	class PixelToAngleSolver
	{
	public:
		struct FuncParams: public BaseForFuncParams
		{		
			double Npix; DoubleArray& cal;

			double func (double x); //This is callback function for GSL solver
			FuncParams(double _Npix, DoubleArray& _cal): Npix(_Npix), cal(_cal) {}
			FuncParams(FuncParams& _p): Npix(_p.Npix), cal(_p.cal) {}
		};	
	};
	struct AngleFromCalibration 
	{ 
		int status; double teta; DoubleArray cal; 
		AngleFromCalibration() {teta = 0; status = GSL_FAILURE;}
	};

	enum {ind_alfa, ind_n_p, ind_n_s, ind_lambda, ind_fi0, ind_L, ind_d0, ind_N0, ind_max};
public:
	DoubleArray val;
	DoubleArray Nexp, teta;
	
	CalibrationParams(): SolverData()  {val.SetSize(ind_max);}
	virtual void Serialize(CArchive& ar);
	CalibrationParams& operator=(CalibrationParams& t);

	int CalculateFrom(DoubleArray& _Nexp, DoubleArray& _teta, double _n_p, double _n_s, double _alfa, double _lambda);
	AngleFromCalibration ConvertPixelToAngle(double Npix);
	double ConertAngleToBeta(double teta) { return val[ind_n_p]*sin(teta); }
};
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
struct betta_info 
{
	double val; int n;
	betta_info(double _val = 0, int _n = 0): val(_val), n(_n) {}
};

class FilmParams: public SolverData
{
public:
	enum {index_n, index_H};
public:
	class DispEqSolver
	{
	public:
		struct FuncParams: public BaseForFuncParams
		{
		public:
			double n1, n2, n3, kHf;
		public:
			//This is callback function for GSL solver
			double (DispEqSolver::FuncParams::*func) (double x); double funcTE (double x); double funcTM (double x);
			void SetFunc(Polarization pol)
			{
				switch (pol)
				{
				case TE: func = &DispEqSolver::FuncParams::funcTE; break;
				case TM: func = &DispEqSolver::FuncParams::funcTM; break;
				default: func = NULL;
				}
			}
			FuncParams(Polarization pol, double _n1, double _n2, double _n3, double _kHf):
			n1(_n1), n2(_n2), n3(_n3), kHf(_kHf)
			{
				SetFunc(pol);
			}
			FuncParams(FuncParams& params):
				n1(params.n1), n2(params.n2), n3(params.n3), kHf(params.kHf), func(params.func)
			{}
		};	
	};
	struct FuncParams: public BaseForFuncParams
	{
	public:
		double n1, n3, k; DoubleArray& bettaexp; Polarization pol;
		CArray<betta_info> betta_teor;
	public:
		double func(const gsl_vector * x);
		FuncParams(Polarization _pol, double _n1, double _n3, double _k, DoubleArray& _bettaexp): 
			pol(_pol), bettaexp(_bettaexp), n1(_n1), n3(_n3), k(_k)
		{}
		FuncParams(FuncParams& params): 
			pol(params.pol), bettaexp(params.bettaexp), n1(params.n1), n3(params.n3), k(params.k)
		{}
		virtual void CleanUp();

	};	
public:
	double n, H, m, minimum_value;

	FilmParams(double _n = 0, double _H = 0): n(_n), H(_H), SolverData() {minimum_value = 0; m = 0;}
	int Calculator(	Polarization pol, double n1, double n3, double k, 
					DoubleArray& bettaexp, 
					FilmParams initX = FilmParams(), FilmParams initdX = FilmParams(1e-4, 1e-1));
};

//////////////////////////////////////////////////////////////////////////
struct CalcRParams
{
	FilmParams i,f,s;
	double lambda, Np, teta_min, teta_max;
	int num_pnts;
	TSimplePointSeries::DataImportMsg* R, *teta;
	CalcRParams() { num_pnts=7000; R=teta=NULL; }
};

void CalcR_TE(CalcRParams& params);
void CalcR_TM(CalcRParams& params);
//////////////////////////////////////////////////////////////////////////

struct Fit_Ax2BxCParams: public FittingPerfomanceInfo
{
public:
	Fit_Ax2BxCParams(double _a=0,double _b=0,double _c=0);
	operator double*() {return a;}
	virtual size_t GetParamsNum() {return 3;}
	SimplePoint GetTop();
	static double f(double x, double* a);
	SimplePoint GetXabsY(double x);
	SimplePoint GetXrelY(double x);
};

struct MultiFitterFuncParams 
{
	size_t n;
	double *y, *sigma, leftmostX, rightmostX,dx;
	MultiFitterFuncParams(DoubleArray& _x, DoubleArray& _y, DoubleArray& _sigma) 
	{
		n=0; y=sigma=NULL;
		if(_y.GetSize()!=_sigma.GetSize()) return;
        y=_y.GetData(); sigma=_sigma.GetData(); n=_y.GetSize();
		leftmostX=_x[0]; 
		rightmostX=_x[(int)n-1];
		dx=(rightmostX-leftmostX)/n;
	}
	size_t GetPointsNum() {return n;}
};
typedef MultiFitterTemplate<Fit_Ax2BxCParams,MultiFitterFuncParams> Fit_Ax2BxC;
int Fit_Parabola(MultiFitterFuncParams& in, Fit_Ax2BxCParams& init,Fit_Ax2BxCParams& out);
//////////////////////////////////////////////////////////////////////////
//************************************************************************
//////////////////////////////////////////////////////////////////////////

struct Fit_KneeParams: public FittingPerfomanceInfo
{
public:
	double &A, &B, &C, &k;

	Fit_KneeParams(double _a=0,double _b=0,double _c=0,double _k=0);
	void operator= (const Fit_KneeParams& t);
	operator double*() {return a;}
	virtual size_t GetParamsNum() {return 4;}
	SimplePoint GetTop(double l);
	static double f(double x, double* a);
	SimplePoint GetXabsY(double x);
	SimplePoint GetXrelY(double x);
};

typedef MultiFitterTemplate<Fit_KneeParams,MultiFitterFuncParams> Fit_Knee;
int Fit_StepFunc(MultiFitterFuncParams& in, Fit_KneeParams& init,Fit_KneeParams& out);
//////////////////////////////////////////////////////////////////////////
int FourierFilter(FFTRealTransform::Params& in, double spec_width, FFTRealTransform::Params& out);