#pragma once

#include "my_gsl.h"

enum Polarization {TE, TM};

//////////////////////////////////////////////////////////////////////////
struct AngleFromCalibration: public SolverData
{ 
	int status; double teta, Npix; DoubleArray cal; 
	AngleFromCalibration() {teta = Npix = 0; status = GSL_FAILURE;}
	AngleFromCalibration& operator=(const AngleFromCalibration& ref) 
	{
		status = ref.status; teta = ref.teta; cal = ref.cal; Npix = ref.Npix;
		*((SolverData*)this) = *((SolverData*)&ref);
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
			double (FuncParams::*funcCB)(double x);
			double func (double x); //This is callback function for GSL solver

			FuncParams(	DoubleArray& _N, DoubleArray& _teta, 
						double _n_p, double _n_i, double _n_s, double _alfa, double _lambda):
							N(_N), teta(_teta), n_p(_n_p), n_i(_n_i), n_s(_n_s), alfa(_alfa), lambda(_lambda)
			{
				A = B = NULL; size = N.GetSize(); 
				funcCB = &FuncParams::func;
			}
			virtual ~FuncParams()
			{
				DestroyBuffers();			
			}							
			FuncParams(FuncParams& p):
				N(p.N), teta(p.teta), n_p(p.n_p), n_s(p.n_s), alfa(p.alfa), funcCB(p.funcCB)
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
				if(A != NULL) {delete[] A; A = NULL;}
				if(B != NULL) {delete[] B; B = NULL;}
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
			double (FuncParams::*funcCB)(double x);

			double func(double x); //This is callback function for GSL solver
			FuncParams(double _Npix, DoubleArray& _cal): Npix(_Npix), cal(_cal) 
			{ 
				funcCB = &FuncParams::func;
			}
			FuncParams(FuncParams& p): Npix(p.Npix), cal(p.cal), funcCB(p.funcCB) { }
		};	
	};

	enum {ind_alfa, ind_n_p, ind_n_i, ind_n_s, ind_lambda, ind_fi0, ind_L, ind_d0, ind_N0, ind_max};
public:
	DoubleArray val;
	DoubleArray Nexp, teta;
	
	CalibrationParams(): SolverData()  {CleanUp();}
	~CalibrationParams() {CleanUp();}
	virtual void Serialize(CArchive& ar);
	CalibrationParams& operator=(const CalibrationParams& t);
	BOOL IsValidCalibration() {return (val.GetSize() == ind_max);}

	int CalculateFrom(	DoubleArray& _Nexp, DoubleArray& _teta, 
						double _n_p, double _n_i, double _n_s, double _alfa, double _lambda);
	AngleFromCalibration ConvertPixelToAngle(double Npix);
	double ConertAngleToBeta(double teta) { return val[ind_n_p]*sin(teta); }
	static double Get_k(const DoubleArray& cal_val) {return 2*M_PI/cal_val[ind_lambda];}
	virtual void CleanUp()
	{
		val.RemoveAll(); Nexp.RemoveAll(); teta.RemoveAll();	
	}
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
			double (FuncParams::*funcCB) (double x); 
			double funcTE (double x); double funcTM (double x);
			FuncParams(Polarization pol, double _n1, double _n2, double _n3, double _kHf):
				n_i(_n1), n_f(_n2), n_s(_n3), kHf(_kHf)
			{
				switch (pol)
				{
				case TE: funcCB = &FuncParams::funcTE; break;
				case TM: funcCB = &FuncParams::funcTM; break;
				default: funcCB = NULL;
				}
			}
			FuncParams(FuncParams& params):
				n_i(params.n_i), n_f(params.n_f), n_s(params.n_s), kHf(params.kHf), funcCB(params.funcCB)
			{}
		};	
	};
	struct FuncParams: public BaseForFuncParams
	{
	protected:
		CalibrationParams cal;
		double k, n_i, n_s;
	public:
		TypeArray<AngleFromCalibration> betta_exp; Polarization pol;
		TypeArray<betta_info> betta_teor;
	public:
		double func(const gsl_vector * x);
		FuncParams(const Polarization _pol, const TypeArray<AngleFromCalibration> &tetaexp, 
			const CalibrationParams &_cal): 
			pol(_pol)
		{			
			cal = _cal;
			k = 2*M_PI/cal.val[CalibrationParams::ind_lambda];
			n_i = cal.val[CalibrationParams::ind_n_i]; n_s = cal.val[CalibrationParams::ind_n_s];

			for(int i = 0; i < tetaexp.GetSize(); i++)
			{
				AngleFromCalibration angle(tetaexp[i]);				
				angle.teta = cal.ConertAngleToBeta(angle.teta); 
				betta_exp << angle;
			}
		}
		FuncParams(FuncParams& params): 
			pol(params.pol), betta_exp(params.betta_exp)
		{}
		virtual void CleanUp();

	};	
public:
	double n, H, m, minimum_value;
	DoubleArray betta_exp;
	TypeArray<betta_info> betta_teor;	

	FilmParams(double _n = 0, double _H = 0, double _m = 0): n(_n), H(_H), m(_m), SolverData() {minimum_value = 0; m = 0;}
	int Calculator( const Polarization pol, const CalibrationParams &cal, 
		const TypeArray<AngleFromCalibration> &tetaexp, 
		FilmParams initX = FilmParams(), FilmParams initdX = FilmParams(1e-4, 1e-1));
};

//////////////////////////////////////////////////////////////////////////
struct CalcR_Result
{
	double teta, Ra, ST;
	CalcR_Result(double t = 0, double ra = 0, double st = 0): teta(t), Ra(ra), ST(st) {}
};
typedef TypeArray<CalcR_Result> CalcR_ResultArray;

struct CalcRParams
{
	FilmParams i,f,s;
	double lambda, Np, teta_min, teta_max;
	int num_pnts;
	CalcRParams() { num_pnts=7000; }
};

CalcR_ResultArray CalcR(const Polarization pol, const CalcRParams& params);

int FourierFilter(FFTRealTransform::Params& in, double spec_width, FFTRealTransform::Params& out);