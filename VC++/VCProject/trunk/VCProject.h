#ifndef __H_VCPROJECT_
#define __H_VCPROJECT_

#include <windows.h>
#include <gdiplus.h>
#include <stdio.h>
#include <iostream.h>
#include <fstream.h>
#include <math.h>


#include "libmatlab.h"

/*

#include "matlab.hpp"

#include "matrix_value.hpp"
#include "presp_transform.hpp"
#include "get6points.hpp"
*/


//#pragma comment(lib, "libmatpm.lib")
//#pragma comment(lib, "libmx.lib")
//#pragma comment(lib, "libmatlb.lib")
//#pragma comment(lib, "libmat.lib")
//#pragma comment(lib, "libmmfile.lib") 

#pragma comment(lib, "libmatlab.lib")


#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

#define MIN(A,B)	((A) < (B) ? (A) : (B))
#define MAX(A,B)	((A) > (B) ? (A) : (B))
#define cnt 100
#define BLACK 1
#define WHITE 0
extern ofstream fout;

void camera_calibrate(BYTE **&image,Bitmap *&bitmap);
void transformation(Bitmap *&result, Bitmap *A_bitmap, Bitmap *C_bitmap, double *params);
void overlap_coarse(BYTE **ppt_image, BYTE **mimio_image, Bitmap *&result);
void overlap(BYTE **ppt_image, BYTE **mimio_image, double *params, Bitmap *&result);
int  item_seg(int y, int x, BYTE **image, BYTE **item_LUT, int& yMin, int& xMin, int& yMax, int& xMax,int& seg_pixels);
void matching(BYTE **ppt_image, BYTE **mimio_image, Bitmap *ppt_bitmap, Bitmap *mimio_bitmap, Bitmap *&matching, Bitmap *&matched,double *params);
void scale_translate(BYTE **ppt_image, BYTE **mimio_image, Bitmap *ppt_bitmap, Bitmap *mimio_bitmap, Bitmap *&newbitmap,double *params);
int  GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
void grayimage(Bitmap *bitmap, BYTE **image);
void Save2File(char* fname, BYTE **image, Bitmap *bitmap, CLSID *pClsid, int image_W, int image_H);
void set_image(BYTE **&image, Bitmap *bitmap);
void get_image(BYTE **image, Bitmap *bitmap);
void thinningImage(BYTE **image, Bitmap *bitmap, CLSID *pClsid);
void convolve (BYTE **in, BYTE **out, int flag, int **mask, int mask_h, int mask_w);
void global_translate(BYTE ** ppt_image, BYTE ** mimio_scaled_image, int *tranx, int *trany, Bitmap *&result);


#endif