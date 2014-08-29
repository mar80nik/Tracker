//---------------------------------------------------------------------------
#ifndef my_colorH
#define my_colorH
#include <math.h>
#include <WinGDI.h>
//---------------------------------------------------------------------------
#define SWAP(a,b,t) { t=a; a=b; b=t; }
#define BYTE unsigned char

__inline int getL_HSL(RGBTRIPLE& col)
{
	BYTE t; 
	if(col.rgbtGreen<col.rgbtBlue)	SWAP(col.rgbtGreen,col.rgbtBlue,t);
	if(col.rgbtRed<col.rgbtGreen)	
	{
		SWAP(col.rgbtRed,col.rgbtGreen,t);
	}
	else
	{
		return (col.rgbtBlue+col.rgbtRed)>>1;	
	}
	if(col.rgbtGreen<col.rgbtBlue)	SWAP(col.rgbtGreen,col.rgbtBlue,t);
	return (col.rgbtBlue+col.rgbtRed)>>1;
};


inline int getL_HSV(RGBTRIPLE& col)
{
	BYTE t; 
	if(col.rgbtGreen<col.rgbtBlue)	SWAP(col.rgbtGreen,col.rgbtBlue,t);
	if(col.rgbtRed<col.rgbtGreen)	
	{
		SWAP(col.rgbtRed,col.rgbtGreen,t);
	}
	else
	{
		return col.rgbtRed;	
	}
	if(col.rgbtGreen<col.rgbtBlue)	SWAP(col.rgbtGreen,col.rgbtBlue,t);
	return col.rgbtRed;
};

struct HSLColor
{
protected:
	double M, wmax,wmid,wmin;
	BYTE l; int h; double s;
	struct rgbColors {BYTE r,g,b;};
	
	rgbColors Convert();
public:
	HSLColor(int _h=0, BYTE _l=127, double _s=1.) {l=_l; h=_h; s=_s;M=255; wmin=1/3.;wmid=1/3.;wmax=1/3.;};
	void Set(int _h, BYTE _l=127, double _s=1.) {l=_l; h=_h; s=_s;};
	operator COLORREF();
	operator RGBQUAD();
};
#endif
