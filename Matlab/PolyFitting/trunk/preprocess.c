/*
	File: preprocess.c
	
	C code of preprocess for noise reduction. Click <C_Preprocess.c> to show the source code.
	
	Algorithms:
	- Remove the isolated noise <remove_isolation>
	- Thinning the binary image for further processing <thinningImage>.
	- Creating CC for inside noise removal <create_cc>
	- Removing noise CC inside the building. <remove_ins_cc>	

*/

#include <mex.h>
#include <windows.h>
#include <stdio.h>
#include <math.h>


#define PI 3.1415926
#define X_MIN 105
#define X_MAX 425
#define Y_MIN 130
#define Y_MAX 260

#define BLACK 1
#define WHITE 0

typedef unsigned char BYTE;

typedef struct _vector
{
	size_t size;
	size_t capacity;
	int *data;
} Vector;

typedef struct _cc_array
{
	size_t size;
	size_t capacity;
	Vector *data;
} CC_Head;

void item_seg(int y, int x, BYTE **image, BYTE **item_LUT, Vector *vct, int *seg_pixels, int h, int w);

Vector* create_vector()
{
	Vector *node;

	node = (Vector *) malloc(sizeof(Vector));
	node->size = 0;
	node->capacity = 100;
	node->data = (int *)malloc(sizeof(int)*node->capacity);

	return node;
}

void push_CC(CC_Head *vct, Vector value)
{
	Vector *tmp;

	if (vct->size == vct->capacity)
	{
		vct->capacity += 100;
		tmp = (Vector*) malloc (sizeof(Vector)*vct->capacity);
		memcpy(tmp, vct->data, (vct->capacity - 100)*sizeof(Vector));
		free(vct->data);
		vct->data = tmp;
	}

	vct->data[vct->size++] = value;

}

void push_back(Vector *vct, int value)
{
	int *tmp;

	if (vct->size == vct->capacity)
	{
		vct->capacity += 100;
		tmp = (int*) malloc (sizeof(int)*vct->capacity);
		memcpy(tmp, vct->data, (vct->capacity - 100)*sizeof(int));
		free(vct->data);
		vct->data = tmp;
	}

	vct->data[vct->size++] = value;

}

BYTE** copy_image(BYTE **src, int h, int w)
{
   	//make image from color to gray
	BYTE **dst;
	int y;

	dst = (BYTE**)malloc(sizeof(BYTE*)*h);
	dst[0] = (BYTE*)malloc(sizeof(BYTE)*h*w);
	for (y=0; y<h; y++)
		dst[y] = dst[0] + w*y;

	memcpy(dst[0], src[0], h*w);

	return dst;
}

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

	image = (BYTE**)malloc(sizeof(BYTE*)*h);
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
//	Function: remove_ins_cc
//
//  Remove the noise falling inside of the building
//		
//	Pre-conditions:
//		CC_Head *cc_head - pointing to the head of the CCs.
//		BYTE **image - image to be computed
//		int h - image height
//		int w - image width
//
//	Post-conditions:
//		return the image with noise removed
//
//	Algorithms:
//	- call <psu_thinning> to get the boundary pixels.
//	- Calculate the number of pixels in CC which belongs to the boundary pixels. Using a threshold here.
*/

void remove_ins_cc(CC_Head *cc_head, BYTE **img, int h, int w)
{
	CC_Head *temp = cc_head;
	Vector *vct;
	int x, y;
	size_t i, j, total_val;
	
	// algorithm 1:
	BYTE **tmp_img = copy_image(img, h, w);
	
	// tmp_img is the boundary image of the building.
	psu_thinning(tmp_img, h, w);
	
	for (i = 0; i < temp->size; i++)
	{
		vct = temp->data + i;
		total_val = 0;
		for (j = 0; j < vct->size; j++)
		{
			x = vct->data[j++];
			y = vct->data[j];
			if (tmp_img[y][x] == BLACK)  // redundant condition? (NO - count the boundary points)
				total_val ++;

		}

		// count the number of boundary points.
		if ((total_val < (vct->size)/4 + 1) && (total_val < 10)) // total_val < 10 used for case 39, where the boundary points are connected with inner points heavily.
		{
			for (j = 0; j < vct->size; j=j+2)
				img[vct->data[(j+1)]][vct->data[j]] = WHITE;
		}

	} // end of i;

	free(tmp_img[0]);
	free(tmp_img);
}

/* 
//	Function: create_cc
//
//  creat the CC for further usage.
//		
//	Pre-conditions:
//		BYTE **image - image to be computed
//		int h - image height
//		int w - image width
//
//	Post-conditions:
//		return CC_Head* pointing to the head of the CC.
//
//	Algorithms:
//	Call the <item_seg> to create CC.
*/

CC_Head* create_cc(BYTE **mimio_image,  int h, int w)
{
	int x, y;
	CC_Head *cc_head_ptr;
	int *segment_pixels;
	Vector *cc_vct;
	BYTE **item_LUT;
	
	cc_head_ptr = (CC_Head*) malloc (sizeof(CC_Head));
	cc_head_ptr->size = 0;
	cc_head_ptr->capacity = 100;
	cc_head_ptr->data = (Vector *) malloc (sizeof(Vector)*cc_head_ptr->capacity);

	// Set up the LUT for the connective component
	item_LUT = (BYTE **)malloc(sizeof(BYTE*) *h);
	item_LUT[0] = (BYTE *)malloc(sizeof(BYTE)*h*w);
	for (y=0; y<h; y++)
		item_LUT[y] = item_LUT[0] + y * w; 
	for (y=0; y < h; y++)
		for (x=0; x<w; x++)
			item_LUT[y][x] = 0;


	//Segment the items.
	segment_pixels = (int *)malloc(sizeof(int));
	*segment_pixels = 0;

	for (y=1; y<h-1; y++)
	{
		for(x=1; x<w-1; x++)
		{
			if (mimio_image[y][x] == BLACK && item_LUT[y][x] == 0)
			{
				cc_vct = create_vector();
				item_seg(y,x,mimio_image,item_LUT,cc_vct,segment_pixels,h,w);
				push_CC(cc_head_ptr, *cc_vct);
			}
		}

	}

	free(item_LUT[0]);
	free(item_LUT);

	return cc_head_ptr;

}

/* 
//	Function: item_seg
//
//  Set up the distance look up table for symmetry line finding.
//		
//	Pre-conditions:
//		int x, y - current pixel
//		Vector *vct - the vector recording all pixel in current CC.
//		int *seg_pixels - number of pixels in this CC, not used so far.
//		BYTE **image - image to be computed
//		BYTE **lut - LUT for the pixels
//		int h - image height
//		int w - image width
//
//	Post-conditions:
//		return the LUT and the vector.
//
//	Algorithms:
//	Using the vector to construct the CC.
*/

void item_seg(int y, int x, BYTE **image, BYTE **item_LUT, Vector *vct, int* seg_pixels, int h, int w)
{
	if (item_LUT[y][x] == 1) 
		return ;

	push_back(vct, x);
	push_back(vct, y);

	item_LUT[y][x] = 1;
	(*seg_pixels) ++;
	
	if (x<1 || x>w-2 || y<1 || y>h-2)
	{
		printf("boundary problem in the item_seg\n");
		return;
	}
	if (image[y-1][x] == BLACK) 
	{
		item_seg(y-1,x,image,item_LUT,vct,seg_pixels, h, w);
	}
	if (image[y-1][x+1] == BLACK) 
	{
		item_seg(y-1,x+1,image,item_LUT,vct,seg_pixels, h, w);
	}
	if (image[y][x+1] == BLACK) 
	{
		item_seg(y,x+1,image,item_LUT,vct,seg_pixels, h, w);
	}
	if (image[y+1][x+1] == BLACK) 
	{
		item_seg(y+1,x+1,image,item_LUT,vct,seg_pixels, h, w);
	}
	if (image[y+1][x] == BLACK) 
	{
		item_seg(y+1,x,image,item_LUT,vct,seg_pixels, h, w);
	}
	if (image[y+1][x-1] == BLACK) 
	{
		item_seg(y+1,x-1,image,item_LUT,vct,seg_pixels, h, w);
	}
	if (image[y][x-1] == BLACK) 
	{
		item_seg(y,x-1,image,item_LUT,vct,seg_pixels, h, w);
	}
	if (image[y-1][x-1] == BLACK) 
	{
		item_seg(y-1,x-1,image,item_LUT,vct,seg_pixels, h, w);
	}

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
	int x_min = X_MIN, x_max = X_MAX;
	int y_min = Y_MIN, y_max = Y_MAX;
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
//	Function: posterior_remove
//
//  Remove the noise based on some trickies in this projects.
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
//	remove the noise based on building posterior_remove knowledge
*/

void posterior_remove(BYTE **image, int h, int w)
{
	int x_min = X_MIN + 15, x_max = X_MAX - 15;
	int y_min = Y_MIN, y_max = Y_MAX;
	int thresh = 40;
	int i, j;
	int up;

	psu_thinning(image, h, w);
	
	// psudo code for interior noise remove:
	// 1. get the left and right majority x value by choose 3 random points of y, say 170, 195, 230
	// 2. get the top and bottom majority y value, by similiar algorithm.
	// 3. remove the area between majority - 5 (threshold)
	
	// Even simple method:
	// crop the interior area (only works for case < 39)
	for (i = 125; i < 410; i++)
		for (j = 165; j < 235; j++)
			image[j][i] = WHITE;
		
	for (i = x_min; i < x_max; i++)
	{
		// from bottom to top 
		for (j = y_max; j >= y_min; j--)
		{
			if (image[j][i] == BLACK)
			{ 
				for (up = 1; up < 10 ; up++)
					image[j-up][i] = WHITE;
				
				break;
			}
					
		}
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
//	Function: thinningImage
//
//  Thinning algorithm for image noise removal.
//		
//	Pre-conditions:
//		BYTE **image - image to be computed
//		int h - image height
//		int w - image width
//
//	Post-conditions:
//		The thinned image is pointed by **image.
//
//	Algorithms:
//	- Thinning LUT.
//	- Convolute the LUT N.E.W.S.
*/
void thinningImage(BYTE **image , int h, int w)
{
	BYTE THINNING_LUT[256] = {  1,1,1,0,1,1,0,0,1,1,0,0,0,1,0,0,
								1,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0, /*32*/
								1,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0, 
								0,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0, /*64*/
								1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
								1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
								0,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0,
								0,1,1,1,1,1,1,1,0,1,0,0,0,1,0,0, /*128*/
								1,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,
								1,1,1,1,1,1,1,1,1,1,0,0,1,1,0,0,
								0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,
								1,1,1,1,1,1,1,1,1,1,0,0,1,1,0,0,
								0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,
								0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0};


	int x, y;
	BYTE Bit_shift = 0;
	int is_done = 0;

	while (is_done == 0)
	{
		is_done = 1;  // To indicate no things to be changed.

		// First try to do north and south.
		// Starting from 1, we don't need to worry about the bounder.
		// Need to finish another part, i.e. the bouder situation.
		for (x=1; x<w-1; x++)   // thinning from north.
			for (y=1; y<h-1; y++ )
			{
				if (image[y][x] == BLACK)   
				{
					Bit_shift = (image[y-1][x] << 7) | (image[y-1][x+1] << 6) |
								(image[y][x+1] << 5) | (image[y+1][x+1] << 4) |
								(image[y+1][x] << 3) | (image[y+1][x-1] << 2) |
								(image[y][x-1] << 1) | (image[y-1][x-1] );
					image[y][x] = THINNING_LUT[Bit_shift];
					if (image[y][x] != BLACK)
						is_done = 0; // To indicate something changed.
					Bit_shift = 0;
					

					// Now test the next one to try to thin from north.
					for (y = y + 1; y < h-1 && image[y][x] == BLACK; ++y);
				}// end of if image[y][x] == 1;

			}// end of for.
		
		for (x=1; x<w-2; x++)   // thinning from south.
			for (y=h-2; y>0; y-- )
			{
				if (image[y][x] == BLACK)   
				{
					Bit_shift = (image[y-1][x] << 7) | (image[y-1][x+1] << 6) |
								(image[y][x+1] << 5) | (image[y+1][x+1] << 4) |
								(image[y+1][x] << 3) | (image[y+1][x-1] << 2) |
								(image[y][x-1] << 1) | (image[y-1][x-1] );
					image[y][x] = THINNING_LUT[Bit_shift];
					if (image[y][x] != BLACK)
						is_done = 0; // To indicate something changed.
					Bit_shift = 0;
					

					// Now test the next one to try to thin from south.
					for (y = y - 1; y > 1 && image[y][x] == BLACK; --y);
				}// end of if image[y][x] == 1;

			}// end of for.

		// Now try to do west and east.
		// Starting from 1, we don't need to worry about the bounder.
		// Need to finish another part, i.e. the bouder situation.
		for (y=1; y<h-1; y++ )   // thinning from west.
			for (x=1; x<w-1; x++)
			{
				if (image[y][x] == BLACK)   
				{
					Bit_shift = (image[y-1][x] << 7) | (image[y-1][x+1] << 6) |
								(image[y][x+1] << 5) | (image[y+1][x+1] << 4) |
								(image[y+1][x] << 3) | (image[y+1][x-1] << 2) |
								(image[y][x-1] << 1) | (image[y-1][x-1] );
					image[y][x] = THINNING_LUT[Bit_shift];
					if (image[y][x] != BLACK)
						is_done = 0; // To indicate something changed.
					Bit_shift = 0;
					

					// Now test the next one to try to thin from west.
					for (x = x + 1; x < w-1 && image[y][x] == BLACK; ++x);
				}// end of if image[y][x] == 1;

			}// end of for.

		for (y=1; y<h-1; y++ )    // thinning from east.
			for (x=w-2; x>1; x--)
			{
				if (image[y][x] == BLACK)   
				{
					Bit_shift = (image[y-1][x] << 7) | (image[y-1][x+1] << 6) |
								(image[y][x+1] << 5) | (image[y+1][x+1] << 4) |
								(image[y+1][x] << 3) | (image[y+1][x-1] << 2) |
								(image[y][x-1] << 1) | (image[y-1][x-1] );
					image[y][x] = THINNING_LUT[Bit_shift];
					if (image[y][x] != BLACK)
						is_done = 0; // To indicate something changed.
					Bit_shift = 0;
					

					// Now test the next one to try to thin from east.
					for (x = x - 1; x > 1 && image[y][x] == BLACK; --x);
				}// end of if image[y][x] == 1;

			}// end of for.

	} // end of while;
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
//		- Remove the isolated noise
//		- Thinning the binary image for further processing.
//		- Creating CC for inside noise removal
//		- Removing noise CC inside the building.
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
  CC_Head *cc_head_ptr;
  
  /* The input must be a noncomplex scalar double.*/
  mrows = mxGetM(prhs[0]);
  ncols = mxGetN(prhs[0]);

  /* Create matrix for the return argument. */
  plhs[0] = mxCreateNumericMatrix(mrows,ncols, mxUINT8_CLASS, mxREAL);
  //mexPrintf("size of input %f", sin(30.0/180.0*PI));
  //return; 
  
  /* Assign pointers to each input and output. */
  z = (BYTE*) mxGetPr(plhs[0]);
  im = (unsigned char*)mxGetPr(prhs[0]);

 
	// convert 1D array to 2D array
	h = mrows, w = ncols;
	image = (BYTE**)malloc(sizeof(BYTE*)*h);
	image[0] = (BYTE*)malloc(sizeof(BYTE)*h*w);
	for (y=0; y<h; y++)
		image[y] = image[0] + w*y;

 
	for (y=0; y<h; y++)
		for (x=0; x<w; x++)
			image[y][x] = 1-im[x*h+y]; // 1D => 2D
 
	// Algorithm
	// remove noise around boundary
	prior_remove(image, h, w); // some prior knowledge about the building
	//remove_isolation(image, h, w);
	thinningImage(image, h, w);

	cc_head_ptr = create_cc(image, h, w);
	remove_ins_cc(cc_head_ptr, image, h, w);
	
	// apply some tricky to remove noise for fitting
	// then, try to use chain code representation and apply reconstruction rules.
	posterior_remove(image, h, w);

	// Convert 2D to 1D 
	for (y=0; y<h; y++)
	{
		for (x=0; x<w; x++)
		{
			z[x*h+y] = 1- image[y][x]; // 2D => 1D
		}
	}

	
}

