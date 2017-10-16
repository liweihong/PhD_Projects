/*
	File: preprocess_2.c
	
	C code of preprocess for noise reduction. Click <C_Preprocess_2.c> to show the source code. This processing will be deprecated because of the enhancement of preprocess 1.

	Algorithms:
	- Remove the isolated noise <remove_isolation>
	- Using pudo-thinning to get the boundary of the image <psu_thinning>.
*/

#include <mex.h>
#include <windows.h>
#include <stdio.h>
#include <math.h>


#define PI 3.1415926

#define BLACK 1
#define WHITE 0


typedef unsigned char BYTE;

/* 
//	Function: psu_thinning
//
//  Look for the boundary pixels for the building.
//		
//	Pre-conditions:
//		BYTE **image - image to be computed
//		int h - image height
//		int w - image width
//
//	Post-conditions:
//		return the image with new value copied.
//
//	Algorithms:
//	- create a brand new white image.
//	- search for boundary pixel: top=>bottom; bottom=>top; left=>right; right=>left;
//	- copy back to the original image.
*/
void psu_thinning(BYTE **mimio_image, int h, int w)
{
	int i, j, y;
	BYTE ** image;

	image = (BYTE*)malloc(sizeof(BYTE*)*h);
	image[0] = (BYTE*)malloc(sizeof(BYTE)*h*w);
	for (y=0; y<h; y++)
		image[y] = image[0] + w*y;

	for (j = 0; j < h; j++)
		for (i=0; i < w; i++)
			image[j][i] = WHITE;

	for (i = 0; i < w; i++)
	{
		// from top to botton
		for (j = 0; j < h; j++)
			if (mimio_image[j][i] == BLACK)
			{
				image[j][i] = BLACK;
				break;
			}

		// from bottom to top 
		for (j = h-1; j >= 0; j--)
			if (mimio_image[j][i] == BLACK)
			{
				image[j][i] = BLACK;
				break;
			}
	}

	for (j = 0; j < h; j++)
	{
		// from left to right
		for (i = 0; i < w; i++)
			if (mimio_image[j][i] == BLACK)
			{
				image[j][i] = BLACK;
				break;
			}

		// from right to left 
		for (i = w-1; i >= 0; i--)
			if (mimio_image[j][i] == BLACK)
			{
				image[j][i] = BLACK;
				break;
			}
	}

	for (j = 0; j < h; j++)
		for (i = 0; i < w; i ++)
			mimio_image[j][i] = image[j][i];

	free(image[0]);
	free(image);


}


/* 
//	Function: prior_remove
//
//  Remove the noise based on prior knowledge (cheating for simplify purpose).
//		
//	Pre-conditions:
//		BYTE **image - image to be computed
//		int h - image height
//		int w - image width
//
//	Post-conditions:
//		return the orignal image; the value has been changed.
//
//	Algorithms:
//	remove the noise based on building prior knowledge
*/
void prior_remove(BYTE **mimio_image, int h, int w)
{
	int x_min = 105, x_max= 425;
	int y_min = 130, y_max = 260;
	int i, j;

	for (i = 0; i < x_min; i++)
	{
		for (j=0; j<h; j++)
			mimio_image[j][i] = WHITE;
	}

	for (i = x_max; i < w; i++)
	{
		for (j=0; j<h; j++)
			mimio_image[j][i] = WHITE;
	}

	for (i = 0; i < y_min; i++)
	{
		for (j=0; j<w; j++)
			mimio_image[i][j] = WHITE;
	}

	for (i = y_max; i < h; i++)
	{
		for (j=0; j<w; j++)
			mimio_image[i][j] = WHITE;
	}

}

/* 
//	Function: remove_isolation
//
//  Remove the isolated noise.
//		
//	Pre-conditions:
//		BYTE **image - image to be computed
//		int h - image height
//		int w - image width
//
//	Post-conditions:
//		return the orignal image; the value has been changed.
//
//	Algorithms:
//	remove the noise outside of 40 pixel; crop the image
*/
void remove_isolation(BYTE **mimio_image, int h, int w)
{
	int THRESH = 40;
	int i, j;

	for (i = 0; i < THRESH; i++)
	{
		for (j=0; j<h; j++)
			mimio_image[j][i] = WHITE;
		for (j=0; j<w; j++)
			mimio_image[i][j] = WHITE;
	}

	for (i = w - THRESH; i < w; i++)
	{
		for (j=0; j<h; j++)
			mimio_image[j][i] = WHITE;
	}

	for (i = h - THRESH; i < h; i++)
	{
		for (j=0; j<w; j++)
			mimio_image[i][j] = WHITE;
	}

}


/* 
//	Function: mexFunction
//
//  Interface function for C and Matlab for preprocessing.
//
//	Notes: 
//		the parameters for double *im; instead of unsigned char *im;
//		
//	Algorithms:
//		- Removing the isolated noise
//		- Doing psudo thinning processing to obtain the contour of the building, and remove all other noise.
//		
*/
void mexFunction(int nlhs, mxArray *plhs[], int nrhs,
                 const mxArray *prhs[])
{
  BYTE *z;
  unsigned char *im;
  int mrows, ncols;
  int h, w, x, y;
  BYTE **image;
  
  /* The input must be a noncomplex scalar double.*/
  mrows = mxGetM(prhs[0]);
  ncols = mxGetN(prhs[0]);

  /* Create matrix for the return argument. */
  plhs[0] = mxCreateNumericMatrix(mrows,ncols, mxUINT8_CLASS, mxREAL);
  //mexPrintf("size of input %f", sin(30.0/180.0*PI));
  //return; 
  
  /* Assign pointers to each input and output. */
  z = mxGetPr(plhs[0]);
  im = (unsigned char*)mxGetPr(prhs[0]);

 
	// convert 1D array to 2D array
	h = mrows, w = ncols;
	image = (BYTE*)malloc(sizeof(BYTE*)*h);
	image[0] = (BYTE*)malloc(sizeof(BYTE)*h*w);
	for (y=0; y<h; y++)
		image[y] = image[0] + w*y;

 
	for (y=0; y<h; y++)
		for (x=0; x<w; x++)
			image[y][x] = 1-im[x*h+y]; // 1D => 2D
 
	// Algorithm
	// remove noise around boundary
	//remove_isolation(image, h, w);
	prior_remove(image, h, w);

	// do not psudo thinning to get boundary
	psu_thinning(image, h, w);

	// Convert 2D to 1D 
	for (y=0; y<h; y++)
	{
		for (x=0; x<w; x++)
		{
			z[x*h+y] = 1- image[y][x]; // 2D => 1D
		}
	}

	
}

