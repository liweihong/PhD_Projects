/*
	File: myHT.c
	
	C code of Hough Transform to accelerate computation. Click <C_MyHT.c> to show the source code.

*/

#include "mex.h"
#include "math.h"

#define PI 3.1415926

// 
//	Function: myprintf
//
//	Print only when some conditions are satisfied.
//
//	Parameters:
//		int i - the 1st parameter
//		int iv - the 1st parameter value
//		int j - the 2nd parameter
//		int jv - the 2nd parameter value
//		double val - the value to be monitored
//		
//	Notes:
//		Can use printf instead of mexPrintf;
//
void myprintf(int i, int iv, int j, int jv, double val)
{
	if (i == iv && j == jv)
		mexPrintf("The 5th value is: %f\n", val);
}

/* 
//	Function: mexFunction
//
//  Interface function for C and Matlab.
//
//	Notes: 
//		the parameters for double *im; instead of unsigned char *im;
//		
//	Parameters:
//		int nlhs
//		
*/
void mexFunction(int nlhs, mxArray *plhs[], int nrhs,
                 const mxArray *prhs[])
{
  double *z, *acc, *p, *np, *t, *nt;
  unsigned char *im;
  int mrows, ncols;
  int i, j;
  int ifi, ifj, ir, ic;
  double Indteta, roi, temp1,temp2;
  
  /* The input must be a noncomplex scalar double.*/
  mrows = mxGetM(prhs[0]);
  ncols = mxGetN(prhs[0]);

  /* Create matrix for the return argument. */
  plhs[0] = mxCreateDoubleMatrix(mrows,ncols, mxREAL);
  //mexPrintf("size of input %f", sin(30.0/180.0*PI));
  //return; 
  
  /* Assign pointers to each input and output. */
  z = mxGetPr(plhs[0]);

  acc = mxGetPr(prhs[0]);
  im  = mxGetPr(prhs[1]);
  p   = mxGetPr(prhs[2]);
  np  = mxGetPr(prhs[3]);
  t   = mxGetPr(prhs[4]);
  nt  = mxGetPr(prhs[5]);
  ir  = (int)(*mxGetPr(prhs[6]));
  ic  = (int)(*mxGetPr(prhs[7]));

  //mexPrintf("size of input %d, %d -- length of p and t is : %f , %f, and image size: %d, %d;;;%d, %d\n", mrows, ncols, *np, *nt, ir, ic,i,j);

  /* Call the timestwo subroutine. */
  /* myht(z, acc, im, p, np, t, nt, row, col); */
  for (i = 0; i<mrows*ncols; i++) z[i] = 0;

  for (i = 0; i < ic; i ++) /* i is xIndex, j is yIndex */
	  for (j = 0; j < ir; j++)
	  {
		  if (im[i*ir + j] == 0) /* find(Imbinary == 0)*/
		  {
			  Indteta = 0.0;

			  // myprintf (i, 357, j, 262, im[i*ir + j]
			  for ( ifi = 0; ifi < ncols; ifi ++)
			  {
				  temp1 = t[ifi]*PI/180;
				  roi = i*cos(temp1) + j*sin(temp1);
				  if ( roi > 1 && roi < p[mrows-1])
				  {
					  temp2 = floor(roi+0.5);
					  z[(int)(Indteta*mrows + temp2)] ++;
				  }

				  Indteta ++;
			  }


		  }

	  }
  /* end of big for */
}