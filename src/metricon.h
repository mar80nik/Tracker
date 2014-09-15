#pragma once

#include "TChart/TChartSeries.h"
#include "my_gsl.h"

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
struct CalibrationParams
{
	class Calculator
	{
	public:
		struct FuncParams: public BaseForFuncParams
		{
		protected:
			double *A, *B, *N, *teta, n_p, n_s, alfa;
			int size;
		public:
			//This is callback function for GSL solver
			static double func (double x, void *params);
			FuncParams(DoubleArray& _N, DoubleArray& _teta, double _n_p, double _n_s, double _alfa)
			{
				A = B = NULL; size = _N.GetSize(); N = _N; teta=_teta; n_p=_n_p; n_s=_n_s; alfa=_alfa; 
			}
			FuncParams(FuncParams& p)
			{
				A=B=NULL; size=p.size; N=p.N; teta=p.teta; n_p=p.n_p; n_s=p.n_s; alfa=p.alfa;
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
			void InitCalibrationParams(CalibrationParams& cal);
		} Params;
	public:
		Solver1dTemplate<FuncParams> Solver;

		Calculator(DoubleArray& _N, DoubleArray& _teta, double n_p, double n_s, double alfa): 
			Params(_N, _teta, n_p, n_s, alfa), Solver(SINGLE_ROOT, &Params) {}
		~Calculator() {}
		int Run(CalibrationParams& cal, Solver1d::BoundaryConditions X, SolverErrors Err);
	};

	class PixelToAngleSolver
	{
		struct FuncParams
		{
		protected:
			double Npix;
			CalibrationParams& cal;
		public:
			//This is callback function for GSL solver
			static double func (double x, void *params);
			FuncParams(double _Npix, CalibrationParams& _cal): cal(_cal) { Npix = _Npix; }
		} Params;	
	public:
		Solver1dTemplate<FuncParams> Solver;

		PixelToAngleSolver(double Npix, CalibrationParams& cal): 
			Params(Npix, cal), Solver(SINGLE_ROOT, &Params) {}
		~PixelToAngleSolver() {}
		int Run(double* teta, Solver1d::BoundaryConditions X, SolverErrors Err);
	};

//	This values are predefined
	double alfa, n_p, n_s;
	DoubleArray Nexp,teta;
//	This values will be calculated
	double N0, L, d0, fi0;
	
	CalibrationParams() { alfa = 0; n_s = n_p = 0; N0 = L = d0 = fi0 = 0;}
	virtual void Serialize(CArchive& ar);
	CalibrationParams& operator=(CalibrationParams& t);
	
	int Init( DoubleArray& Nexp, DoubleArray& teta);
	int ConvertPixelToAngle(double Npix, double* teta);
	double ConertAngleToBeta(double teta) { return n_p*sin(teta); }
};
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
enum Polarization {TE, TM};

class DispEqSolver
{
	struct FuncParams: public BaseForFuncParams
	{
	public:
		double n1, n2, n3, kHf;
	public:
		//This is callback function for GSL solver
		static double (*func) (double x, void *params);
		static double funcTE (double x, void *params);
		static double funcTM (double x, void *params);
		FuncParams(Polarization pol, double _n1, double _n2, double _n3, double _kHf)
		{
			n1 = _n1; n2 = _n2; n3 = _n3; kHf = _kHf;
			switch (pol)
			{
			case TE: func = funcTE; break;
			case TM: func = funcTM; break;
			default: func = NULL;
			}
		}
	} Params;	
public:
	Solver1dTemplate<FuncParams> Solver;
	
	DispEqSolver(Polarization pol, double n1, double n2, double n3, double kHf): 
		Params(pol, n1, n2, n3, kHf), Solver(MULTI_ROOT, &Params) {}
	int Run(Solver1d::BoundaryConditions X, SolverErrors Err);
};

//////////////////////////////////////////////////////////////////////////
struct FilmParams: public BaseForFuncParams
{
	double n, H, m; 
	FilmParams(double _n = 0, double _H = 0, double _m = 0) { n=_n; H=_H; m=_m;}
	//int GetParamsNum() {return 2;}
	FilmParams(const gsl_vector* p) { n=gsl_vector_get (p, 0); H=gsl_vector_get (p, 1); }
	operator gsl_vector*()
	{
		gsl_vector* ret=NULL;
		ret = gsl_vector_alloc (GetParamsNum());
		gsl_vector_set (ret, 0, n);
		gsl_vector_set (ret, 1, H);
		return ret;
	}
	FilmParams& operator=(gsl_vector* p) { n=gsl_vector_get (p, 0); H=gsl_vector_get (p, 1); return *this;}
	
};

struct betta_info 
{
	double val; int n;
	betta_info() {val=0; n=0;}
};

//class FilmFuncParams: public BaseForFuncParams
//{ 
//public:
//	double n1, n3, k; DoubleArray& bettaexp; 
//	CArray<betta_info> betta_teor;
//
//	FilmFuncParams(DoubleArray& _bettaexp, double _n1=0, double _n3=0, double _k=0): bettaexp(_bettaexp)
//	{
//		n1=_n1; n3=_n3; k=_k; 
//	}
//};

//class FilmFuncTEParams: public FilmFuncParams
//{
//public:
//	FilmFuncTEParams(DoubleArray& _bettaexp,double _n1=0, double _n3=0, double _k=0): FilmFuncParams(_bettaexp, _n1,_n3,_k) {}
//};
//class FilmFuncTMParams: public FilmFuncParams
//{
//public:
//	FilmFuncTMParams(DoubleArray& _bettaexp,double _n1=0, double _n3=0, double _k=0): FilmFuncParams(_bettaexp, _n1,_n3,_k) {}
//};

class FilmMinimizer
{
	struct FuncParams: public BaseForFuncParams
	{
	public:
		DoubleArray& bettaexp;
	public:
		//This is callback function for GSL solver
		static double (*func) (const gsl_vector * x, void * params);
		static double funcTE (const gsl_vector * x, void * params);
		static double funcTM (const gsl_vector * x, void * params);
		FuncParams(Polarization pol, DoubleArray& _bettaexp): bettaexp(_bettaexp)
		{
			switch (pol)
			{
			case TE: func = funcTE; break;
			case TM: func = funcTM; break;
			default: func = NULL;
			}
		}
	} Params;	
public:
	MultiDimMinimizerTemplate<FuncParams> Solver;

	FilmMinimizer(Polarization pol, double n1, double n2, double n3, double kHf): 
		Params(pol, n1, n2, n3, kHf), Solver(&Params) {}
	int Run(DoubleArray& intiX, DoubleArray& initdX, SolverErrors Err);
};

//typedef MultiDimMinimizerTemplate<FilmParams, FilmFuncTEParams> FilmMinimizerTE;
//typedef MultiDimMinimizerTemplate<FilmParams, FilmFuncTMParams> FilmMinimizerTM;
//////////////////////////////////////////////////////////////////////////
struct CalcRParams
{
	FilmParams i,f,s;
	double lambda, Np, teta_min, teta_max;
	int num_pnts;
	TSimplePointSeries::DataImportMsg* R, *teta;
	CalcRParams() { num_pnts=7000; R=teta=NULL; }
};

int CalclFilmParamsTE(FilmFuncTEParams& in, FilmParams& out);
int CalclFilmParamsTM(FilmFuncTMParams& in, FilmParams& out);
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