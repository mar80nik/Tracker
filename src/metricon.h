#pragma once

#include "TChart\TChartSeries.h"
#include "my_gsl.h"

enum Polarization {TE, TM};

//////////////////////////////////////////////////////////////////////////
//struct FittingPerfomanceInfo: public PerfomanceInfoMk1
//{	
//protected:
//	int p;
//public:
//	double *a,*da;
//	double chisq_dof, leftmostX, rightmostX, dx;
//	size_t n;
//
//	FittingPerfomanceInfo(int p);
//	virtual ~FittingPerfomanceInfo();
//	void GetSolverResults(gsl_multifit_fdfsolver *s);	
//	void operator= (const FittingPerfomanceInfo& t);
//};
//////////////////////////////////////////////////////////////////////////
struct AngleFromCalibration: public SolverData
{ 
	int status; double teta, Npix; DoubleArray cal; 
	AngleFromCalibration() {teta = Npix = 0; status = GSL_FAILURE;}
	AngleFromCalibration& operator=(const AngleFromCalibration& ref) 
	{
		status = ref.status; teta = ref.teta; cal = ref.cal; Npix = ref.Npix;
		return (*this);
	}
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
			double *A, *B; DoubleArray &N, &teta; double n_p, n_i, n_s, alfa, lambda;
			int size;
		public:			
			double func (double x); //This is callback function for GSL solver
			FuncParams(	DoubleArray& _N, DoubleArray& _teta, 
						double _n_p, double _n_i, double _n_s, double _alfa, double _lambda):
							N(_N), teta(_teta), n_p(_n_p), n_i(_n_i), n_s(_n_s), alfa(_alfa), lambda(_lambda)
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

	enum {ind_alfa, ind_n_p, ind_n_i, ind_n_s, ind_lambda, ind_fi0, ind_L, ind_d0, ind_N0, ind_max};
public:
	DoubleArray val;
	DoubleArray Nexp, teta;
	
	CalibrationParams(): SolverData()  {val.SetSize(ind_max);}
	virtual void Serialize(CArchive& ar);
	CalibrationParams& operator=(CalibrationParams& t);

	int CalculateFrom(	DoubleArray& _Nexp, DoubleArray& _teta, 
						double _n_p, double _n_i, double _n_s, double _alfa, double _lambda);
	AngleFromCalibration ConvertPixelToAngle(double Npix);
	double ConertAngleToBeta(double teta) { return val[ind_n_p]*sin(teta); }
	static double Get_k(const DoubleArray& cal_val) {return 2*M_PI/cal_val[ind_lambda];}
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
			double n_i, n_f, n_s, kHf;
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
			n_i(_n1), n_f(_n2), n_s(_n3), kHf(_kHf)
			{
				SetFunc(pol);
			}
			FuncParams(FuncParams& params):
				n_i(params.n_i), n_f(params.n_f), n_s(params.n_s), kHf(params.kHf), func(params.func)
			{}
		};	
	};
	struct FuncParams: public BaseForFuncParams
	{
	public:
		TypeArray<AngleFromCalibration> &bettaexp; Polarization pol;
		TypeArray<betta_info> betta_teor;
	public:
		double func(const gsl_vector * x);
		FuncParams(Polarization _pol, TypeArray<AngleFromCalibration> &_bettaexp): 
			pol(_pol), bettaexp(_bettaexp)
		{}
		FuncParams(FuncParams& params): 
			pol(params.pol), bettaexp(params.bettaexp)
		{}
		virtual void CleanUp();

	};	
public:
	double n, H, m, minimum_value;
	TypeArray<betta_info> betta_teor;

	FilmParams(double _n = 0, double _H = 0, double _m = 0): n(_n), H(_H), m(_m), SolverData() {minimum_value = 0; m = 0;}
	int Calculator(	Polarization pol, TypeArray<AngleFromCalibration> &bettaexp, 
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
//////////////////////////////////////////////////////////////////////////
class ParabolaFitFunc: public BaseForFitFunc
{
//f = a*x*x + b*x + c
public:
	enum {ind_c, ind_b, ind_a, ind_max};
protected:
	struct FuncParams: public BaseForMultiFitterFuncParams
	{
		static double func(const double &x, const double *a, const size_t &p);	

		static double df_da(const double &x, const double *a, const size_t &p, double *c);	
		static double df_db(const double &x, const double *a, const size_t &p, double *c);	
		static double df_dc(const double &x, const double *a, const size_t &p, double *c);	

		FuncParams( const DoubleArray& x, const DoubleArray& y, const DoubleArray& sigma ) : 
			BaseForMultiFitterFuncParams(ind_max, x, y, sigma)
		{
			pFunction = FuncParams::func;
			pDerivatives[ind_a] = df_da; pDerivatives[ind_b] = df_db; pDerivatives[ind_c] = df_dc;
		}
	};
public:
	double GetTop(double &x);	
	int CalculateFrom(const DoubleArray& x, const DoubleArray& y, const DoubleArray& sigma, DoubleArray& init_a);
};
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
class KneeFitFunc: public BaseForFitFunc
{
//f = C + (A/(1 + exp(-2*k*(x - B))));
public:
	enum {ind_A, ind_B, ind_C, ind_k, ind_max};
protected:
	double buf;
	struct FuncParams: public BaseForMultiFitterFuncParams
	{
		static double func(const double &x, const double *a, const size_t &p);	

		static double df_dA(const double &x, const double *a, const size_t &p, double *c);	
		static double df_dB(const double &x, const double *a, const size_t &p, double *c);	
		static double df_dC(const double &x, const double *a, const size_t &p, double *c);	
		static double df_dk(const double &x, const double *a, const size_t &p, double *c);	

		FuncParams( const DoubleArray& x, const DoubleArray& y, const DoubleArray& sigma ) : 
		BaseForMultiFitterFuncParams(ind_max, x, y, sigma)
		{
			pFunction = FuncParams::func;
			pDerivatives[ind_A] = df_dA; pDerivatives[ind_B] = df_dB; 
			pDerivatives[ind_C] = df_dC; pDerivatives[ind_k] = df_dk;
		}
	};
	virtual double * PrepareDerivBuf(const double &x, const double *a, const size_t &p);
public:
	double GetInflection(double &x, const double &level);	
	int CalculateFrom(const DoubleArray& x, const DoubleArray& y, const DoubleArray& sigma, DoubleArray& init_a);
};
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
int FourierFilter(FFTRealTransform::Params& in, double spec_width, FFTRealTransform::Params& out);