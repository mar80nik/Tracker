#include <stdafx.h>
#include ".\my_color.h"


HSLColor::rgbColors HSLColor::Convert()
{
	double f1,lq, min,max,mid,r=0,g=0,b=0; rgbColors ret;

	if(s==0) {r=l;g=l;b=l;}
	else
	{
		int k=h/60;double f=(h/60.)-(double)k;
		f1=(k%2 ? 1-f:f); 
		lq=(wmid*f1+wmax)*M;
		if(l<=lq)
		{
			min=(1-s)*l;
			mid=(f1*l+min*((1-f1)*wmax-f1*wmin))/(wmax+f1*wmid);
			max=(l-wmid*mid-wmin*min)/wmax;
		}
		else
		{
			max=s*M+(1-s)*l; 
			mid=((1-f1)*l-max*((1-f1)*wmax-f1*wmin))/((1-f1)*wmid+wmin);
			if(wmin>0) min=(l-wmax*max-wmid*mid)/wmin;
			else min=(mid-f1*max)/(1-f1);
		}
		switch(k)
		{
		case 0: r=max;g=mid;b=min;break;
		case 1: r=mid;g=max;b=min;break;
		case 2: r=min;g=max;b=mid;break;
		case 3: r=min;g=mid;b=max;break;
		case 4: r=mid;g=min;b=max;break;
		case 5: r=max;g=min;b=mid;break;
		}
	}
	ret.b=(BYTE)b; ret.g=(BYTE)g; ret.r=(BYTE)r;
	return ret;
}

HSLColor::operator COLORREF()
{
	rgbColors ret=Convert();
	return RGB(ret.r,ret.g,ret.b);
}

HSLColor::operator RGBQUAD()
{ 
	RGBQUAD ret;
	rgbColors t=Convert();
	ret.rgbBlue=t.b; ret.rgbGreen=t.g; ret.rgbRed=t.r;
	return ret;
}

//#include <gsl/gsl_heapsort.h>

int SortByte(const void *_a,const void *_b)
{
	BYTE *a=(BYTE*)_a, *b=(BYTE*)_b; 
	if (*a > *b) return 1;
	else if (*a < *b) return -1;
	else return 0;
}
/*
int getL(RGBTRIPLE col)
{
	BYTE *buf=&col.rgbtBlue; 
	gsl_heapsort(buf,3,1,SortByte);
	return (buf[2]+buf[0])>>1;
}
*/
/*
int getL(RGBTRIPLE col)
{
	BYTE *buf=&col.rgbtBlue,t; 
	if(buf[1]<buf[0]) {t=buf[1]; buf[1]=buf[0]; buf[0]=t;}
	if(buf[2]<buf[1]) {t=buf[2]; buf[2]=buf[1]; buf[1]=t;}
	if(buf[1]<buf[0]) {t=buf[1]; buf[1]=buf[0]; buf[0]=t;}
	return (buf[2]+buf[0])>>1;
}
*/
