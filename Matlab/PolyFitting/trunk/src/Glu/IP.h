/* ======================================================================
 * IMPROC: Image Processing Software Package
 * Copyright (C) 2002 by George Wolberg
 *
 * IP.h - Header for IP*.c files
 *
 * Written by: George Wolberg, 2002
 * ======================================================================
 */

/* ----------------------------------------------------------------------
 * standard include files
 */

#ifndef		IP_H
#define		IP_H

#ifndef LINUX
#include <windows.h>
#endif

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include <ctype.h>
#include <setjmp.h>
#include <stdlib.h>
#include <sys/stat.h>
//#include <sys/param.h>
#include <stdarg.h>
#include <signal.h>


#define GL
/* ----------------------------------------------------------------------
 * datatype typedefs
 */
typedef	unsigned char	uchar;

typedef	unsigned long	ulong;
typedef	unsigned short	ushort;
typedef	unsigned int	uint;

typedef int   (*PFI) ();	/* ptr to fct returning int   */
typedef void  (*PFV) ();	/* ptr to fct returning void  */
typedef char* (*PFCP)();	/* ptr to fct returning char* */



/* ----------------------------------------------------------------------
 * math definitions
 */
#undef PI2
#undef PI_2
#define PII		3.1415926535897931160E0
#define PI2		6.2831853071795862320E0
#define PI_2		1.5707963267948965580E0
#define DEGtoRAD	0.0174532927777777777E0
#define RADtoDEG	57.295778666661658617E0




/* ----------------------------------------------------------------------
 * image and channel definitions
 */
#define MXIMAGE		1024
#define MXCH		4096
#define MXCHANNEL	16
#define NOTFREE		0
#define FREE		1
#define NEWIMAGE	IP_allocImage(0,0,NULL_TYPE)



/* ----------------------------------------------------------------------
 * image and channel types
 * IMPORTANT:	CHTYPE_IMAGE[] and CHTYPE_CH[] are dependent on numbering
 *		used in image and channel types, respectively.
 * NOTE:	This does not apply to image types that refer to file
 *		formats (SUN*_IMAGE, GIF_IMAGE, SGI_IMAGE, PBM_IMAGE,
 *		JPG_IMAGE, TIF_IMAGE ...)
 */
enum image_types {
	 NULL_IMAGE,
	   BW_IMAGE,
	  BWA_IMAGE,
	  RGB_IMAGE,
	 RGBA_IMAGE,
	  HSV_IMAGE,
	  YIQ_IMAGE,
	  LUT_IMAGE,
	  MAT_IMAGE,
	  FFT_IMAGE,
	   GL_IMAGE,
	 SUN1_IMAGE,
	  SUN_IMAGE,
	  GIF_IMAGE,
	  SGI_IMAGE,
	  PBM_IMAGE,
	  JPG_IMAGE,
	  TIF_IMAGE
};

enum channel_types {
	 UCHAR_TYPE,
	 SHORT_TYPE,
	   INT_TYPE,
	  LONG_TYPE,
	 FLOAT_TYPE,
	DOUBLE_TYPE
};



/* ----------------------------------------------------------------------
 * channel sizes
 */
#define  UCHAR_SZ	sizeof(uchar)
#define  SHORT_SZ	sizeof(short)
#define    INT_SZ	sizeof(int)
#define   LONG_SZ	sizeof(long)
#define  FLOAT_SZ	sizeof(float)
#define DOUBLE_SZ	sizeof(double)



/* ----------------------------------------------------------------------
 * pixel sizes
 */
#define   BW_PXLSZ	   UCHAR_SZ
#define  BWA_PXLSZ	(2*UCHAR_SZ)
#define  RGB_PXLSZ	(3*UCHAR_SZ)
#define RGBA_PXLSZ	(4*UCHAR_SZ)
#define  HSV_PXLSZ	(3*SHORT_SZ)
#define  YIQ_PXLSZ	(3*SHORT_SZ)



/* ----------------------------------------------------------------------
 * filter kernels for image resizing
 */
enum filter_kernels {
	POINT2,
	BOX,
	TRIANGLE,
	CSPLINE,
	CUBIC_CONV,
	LANCZOS,
	HANN
};



/* ----------------------------------------------------------------------
 * dither options for error diffusion
 */
enum dither_options {
	FLOYD_STEINBERG,
	FAN,
	JARVIS_JUDICE_NINKE,
	STUCKI,
	BURKES,
	SIERRA,
	STEVENSON_ARCE,
	USER_SPECIFIED
};



/* ----------------------------------------------------------------------
 * correlation options
 */
enum correlation_options {
	CROSS_CORR,
	SSD,
	CORR_COEFF,
	PHASE_CORR
};



/* ----------------------------------------------------------------------
 * pad options
 */
enum pad_options {
	ZERO,
	REPLICATE,
	REFLECT,
	EXTRAPOLATE,
	WRAPAROUND
};



/* ----------------------------------------------------------------------
 * type casting macro:
 * cast total objects in p1 (of type t1) into p2 (with type t2)
 * with strides s1 and s2, respectively
 */
#define MEM_CAST(p1, t1, s1, p2, t2, s2, total)\
	{\
	t1 *src;\
	t2 *dst;\
	int i;\
	src = (t1 *) p1;\
	dst = (t2 *) p2;\
	for(i=0; i<total; i++,src+=s1,dst+=s2) *dst = (t2) *src;\
	}



/* ----------------------------------------------------------------------
 * useful macros
 * Note: ROUND   rounds arg to nearest integer
 * Note: FLOOR   rounds arg to nearest integer towards -infinity
 * Note: CEILING rounds arg to nearest integer towards +infinity
 */
#undef	MAX
#undef	MIN

#define ABS(A)		((A) >= 0 ? (A) : -(A))
#define SGN(A)		((A) >  0 ? 1 : ((A) < 0 ? -1 : 0 ))
#define ROUND(A)	((A) >= 0 ? (int)((A)+.5) : -(int)(.5-(A)))
#define FLOOR(A)	((A)==(int)(A) ? (int)(A) : (A)>0 ? (int)(A)   : (int)(A)-1)
#define CEILING(A)	((A)==(int)(A) ? (int)(A) : (A)>0 ? (int)(A)+1 : (int)(A))

#define MAX(A,B)	((A) > (B) ? (A) : (B))
#define MIN(A,B)	((A) < (B) ? (A) : (B))
#define SWAP(A,B)	{ double temp=(A); (A)  = (B);	(B)  = temp; }
#define SWAP_INT(A,B)	{ (A) ^= (B);	   (B) ^= (A);	(A) ^= (B);  }

#define CLIP(A,L,H)	((A)<=(L) ? (L) : (A)<=(H) ? (A) : (H))

#define IP_copyImageHeader(I1, I2)		IP_copyHeader(I1, 1, I2)
#define IP_copyImageHeaderOnly(I1, I2)		IP_copyHeader(I1, 0, I2)
#define IP_copyImageHeader2(I1, I2, I3)		IP_copyHeader2(I1, I2, 1, I3)
#define IP_copyImageHeaderOnly2(I1, I2, I3)	IP_copyHeader2(I1, I2, 0, I3)



/* ----------------------------------------------------------------------
 * miscellaneous definitions
 */
#define MXRES		4096
#define XMXRES		1280
#define YMXRES		1024
#define MXGRAY		256
#define MXBLUR		256
#define MXHALFTONE	10
#define MXSTRLEN	80
#define MXHISTORY	512
#define MXREAD		65536
#define MXBUFLEN	32768
#define LSB		0x0001



/* ----------------------------------------------------------------------
 * image data structure
 */
typedef struct {			/* image structure	*/
	void	*buf   [MXCHANNEL];	/* channel memory ptrs	*/
	int	 chtype[MXCHANNEL];	/* channel data  types	*/
	int	 width, height;		/* image dimensions	*/
	int	 xoffset, yoffset;	/* offsets from origin	*/
	int	 imagetype;		/* label of image type	*/
	int	 status;		/* FREE or NOTFREE	*/
} imageS, *imageP;



/* ----------------------------------------------------------------------
 * channel data structure
 */
typedef struct {			/* channel structure	*/
	void	*buf;			/* channel memory ptr	*/
	int	 links;			/* # of links to images	*/
	int	 chsize;		/* channel size (bytes)	*/
	int	 status;		/* FREE or NOTFREE	*/
} channelS, *channelP;



/* ----------------------------------------------------------------------
 * system variables (set/fixed at start of session)
 */
extern int	 MaxGray;
extern int	 Verbose;
extern int	 VisibleImages;
extern int	 EnableGL;
extern int	 SIZEOF[];

extern int	 NULL_TYPE[];
extern int	   BW_TYPE[];
extern int	  BWA_TYPE[];
extern int	  RGB_TYPE[];
extern int	 RGBA_TYPE[];
extern int	  HSV_TYPE[];
extern int	  YIQ_TYPE[];
extern int	  LUT_TYPE[];
extern int	  MAT_TYPE[];
extern int	  FFT_TYPE[];

extern int	  UCHARCH_TYPE[];
extern int	  SHORTCH_TYPE[];
extern int	    INTCH_TYPE[];
extern int	   LONGCH_TYPE[];
extern int	  FLOATCH_TYPE[];
extern int	 DOUBLECH_TYPE[];

extern int	  INT2_TYPE[];
extern int	FLOAT2_TYPE[];
extern int	  INT3_TYPE[];
extern int	FLOAT3_TYPE[];
extern int	  INT4_TYPE[];
extern int	FLOAT4_TYPE[];

extern int	*CHTYPE_CH[];
extern int	*CHTYPE_IMAGE[];
extern char	*CHTYPE_NAME[];
extern char	 CHTYPE_LTR[]; 

extern uchar	 BITMASK[];



/* ----------------------------------------------------------------------
 * remaining global variables (may change during session)
 */
extern int	InImage;
extern int	NextImage;
extern int	DrawImage;
extern imageP	InImageP;
extern imageP	NextImageP;
extern imageP	ImagePtr[];
extern imageP	ImageIOBuffer;

extern int	PlotAlg;
extern double	PlotScl;
extern double	CubicConvA;
extern int	AbsVal;
extern int	HistoFit;
extern int	ImagePadMtd;
extern int	ImagePadLen;
extern int	SavePixmap;
extern int	Serpentine;
extern channelP	ChannelPtr[];



/* ----------------------------------------------------------------------
 * Color quantization externs
 */
extern int	 CmapSize;
extern int	 CQMethod;
extern int	 CQDither;
extern PFI	 CQMethods[];
extern imageP	 QntzIndex;
extern imageP	 QntzCmap;


/* ----------------------------------------------------------------------
 * function externs
 */
/*	IParith.c	*/
extern int	IP_subtractImage(imageP, imageP , imageP);
extern int	IP_subtractConst(imageP, double*, imageP);
extern int	IP_addImage	(imageP, imageP , imageP);
extern int	IP_addConst	(imageP, double*, imageP);
extern int	IP_multiplyImage(imageP, imageP , imageP);
extern int	IP_multiplyCmplx(imageP, imageP , imageP);
extern int	IP_multiplyConst(imageP, double*, imageP);
extern int	IP_divideImage	(imageP, imageP , imageP);
extern int	IP_divideConst	(imageP, double*, imageP);
extern void	IP_overlayImage	(imageP, imageP , imageP);
extern int	IP_absoluteValue(imageP, imageP);
extern int	IP_magnitude	(imageP, imageP , imageP);
extern int	IP_magnitudeAbsVal(imageP,imageP, imageP);

/*	IPbit.c		*/
extern void	IP_andImage	(imageP, imageP, imageP);
extern void	IP_andConst	(imageP, uchar*, imageP);
extern void	IP_orImage	(imageP, imageP, imageP);
extern void	IP_orConst	(imageP, uchar*, imageP);
extern void	IP_xorImage	(imageP, imageP, imageP);
extern void	IP_xorConst	(imageP, uchar*, imageP);
extern void	IP_bicImage	(imageP, imageP, imageP);
extern void	IP_notImage	(imageP, imageP);

/*	IPblur.c	*/
extern void	IP_blur		(imageP, double, double, imageP);
extern void	IP_blur1D	(uchar*, int,int,double, uchar*);
extern void	IP_fblur1D	(float*, int,int,double, float*);

/*	IPcanny.c	*/
extern void	IP_canny	(imageP, double, int, int, imageP);
extern void	IP_cannyGradient(imageP, double, int, int, imageP,imageP,imageP);

/*	IPcastimg.c	*/
extern void	IP_castImage	   (imageP, int, imageP);
extern void	IP_initAlphaChannel(imageP, int);

/*	IPcastmem.c	*/
extern void	IP_castMemory	(void*, int, int, void*, int, int, int);

/*	IPcolorconv.c	*/
extern void	IP_RGBtoYIQ1	(int, int, int, int*, int*, int*);
extern void	IP_YIQtoRGB1	(int, int, int, int*, int*, int*);
extern void	IP_RGBtoHSV1	(int, int, int, int*, int*, int*);
extern void	IP_HSVtoRGB1	(int, int, int, int*, int*, int*);
extern void	IP_clipRGB	(int*, int*, int*);

/*	IPcolorqntz*.c	*/
extern int	IP_qntzMedianCut(imageP*,int,int,imageP*,imageP*,int);
extern int	IP_qntzOctree	(imageP*,int,int,imageP*,imageP*,int);

/*	IPconvolve.c	*/
extern void	IP_convolve	(imageP, imageP, imageP);
extern float	IP_correlation	(imageP, imageP, int, int, int*, int*);

/*	IPdisplay.c	*/
extern void	IP_displayImage	      (void);
extern void	IP_displayImageInPlace(void);

/*	IPdither.c	*/
extern void	IP_ditherUnordered(imageP, int, double, imageP);
extern void	IP_ditherOrdered  (imageP, int, double, imageP);
extern void	IP_halftone	  (imageP, int,int,double,imageP);
extern void	IP_ditherDiffuse  (imageP, int, double, imageP);

/*	IPfft.c		*/
extern void	IP_fft2D	(imageP, int, imageP);
extern void	IP_fft1D	(imageP, int, imageP);
extern int	IP_displayFFT	(imageP, imageP);
extern void	IP_fftRecenter	(imageP, imageP);
extern void	IP_complexConjugate(imageP, imageP);

/*	IPfiltnbr.c	*/
extern void	IP_blurGaussian	(imageP, double, imageP);
extern void	IP_blurEdgePreserve(imageP, double, double, double, imageP);
extern void	IP_blurMask	(imageP, imageP, imageP, double, imageP);
extern void	IP_blurMatte	(imageP, double, double, imageP);
extern void	IP_sharpen	(imageP, double, double, double, imageP);
extern void	IP_median	(imageP, int,    imageP);
extern void	IP_gradient	(imageP, imageP, imageP);
extern void	IP_sobel	(imageP, imageP);
extern void	IP_laplacian	(imageP, imageP);
extern void	IP_laplacianThresh(imageP, double*, imageP);
extern void	IP_houghLines	(imageP, int, imageP, imageP);
extern void	IP_houghCircles	(imageP, imageP, imageP, int, int, int, imageP);

/*	IPfiltpt.c	*/
extern void	IP_threshold	(imageP, double, double, double, double, double, imageP);
extern void	IP_qntzUniform	(imageP, int, imageP);
extern void	IP_clip		(imageP, double, double, imageP);
extern void	IP_scaleRange	(imageP, double, double, imageP);
extern void	IP_embedRange	(imageP, double, double, imageP);
extern void	IP_gammaCorrect	(imageP, double, imageP);
extern void	IP_medianOfChannels(imageP, imageP);

/*	IPgeo.c		*/
extern void	IP_resize	(imageP, int, int, int, imageP);
extern void	IP_resize1D	(uchar*, int, int, int, int, uchar*);
extern void	IP_fresize1D	(float*, int, int, int, int, float*);
extern void	IP_rotate	(imageP, double, imageP);
extern void	IP_translate	(imageP, double, double, imageP);
extern void	IP_reflect	(imageP, int, imageP);
extern void	IP_tile		(imageP, int, int, imageP);

/*	IPhisto.c	*/
extern void	IP_histogram(imageP, int, int*, int, double*, double*);
extern void	IP_histogramEqualize	(imageP, imageP	  );
extern void	IP_histogramMatch	(imageP, imageP, imageP);
extern void	IP_displayHistogram	(imageP, int, int, int );

/*	IPio.c		*/
extern imageP	IP_readImage		(char*);
extern int	IP_readImageIntoI	(char*, imageP);
extern int	IP_readImageDimensions	(char*, int*, int*);
extern int	IP_saveImage		(imageP, char*);

/*	IPlut.c		*/
extern void	IP_makeLut	(imageP);
extern void	IP_editLut	(imageP, imageP);
extern void	IP_applyLut	(imageP, imageP, imageP);
extern void	IP_applyLutIntrp(imageP, imageP, imageP);

/*	IPmatte.c	*/
extern void	IP_matteExtract	(imageP, imageP, imageP);
extern void	IP_matteOverlay	(imageP, imageP, imageP);
extern void	IP_matteOverlayAvg(imageP, imageP, imageP);
extern void	IP_matteCut	(imageP, int   , imageP);
extern void	IP_matteOver	(imageP, imageP, int, imageP);
extern void	IP_matteIn	(imageP, imageP, imageP);
extern void	IP_matteOut	(imageP, imageP, imageP);
extern void	IP_matteAtop	(imageP, imageP, int, imageP);
extern void	IP_matteXor	(imageP, imageP, int, imageP);
extern void	IP_matteHicon	(imageP, imageP);
extern void	IP_matteDarken	(imageP, double, imageP);
extern void	IP_matteOpaque	(imageP, double, imageP);
extern void	IP_matteDissolve(imageP, double, imageP);
extern void	IP_matteMultiply(imageP, imageP, int, imageP);
extern void	IP_matteMaxAlpha(imageP, imageP, imageP);

/*	IPmipmap.c	*/
extern int	IP_mipmap	(imageP, imageP*);

/*	IPmmch.c	*/
extern void*	IP_allocChannel		(int, int, int);
extern void	IP_freeChannel		(void*);
extern void	IP_freeChannels		(imageP, int  );
extern void	IP_initChannels		(imageP, int* );
extern int	IP_getChannel		(imageP, int, uchar**, int*);
extern void	IP_copyChannel		(imageP, int, imageP , int );
extern void	IP_copyChannelEnd	(imageP, int, imageP	   );
extern void	IP_linkChannel		(imageP, int, imageP, int  );
extern void	IP_linkChannelEnd	(imageP, int, imageP	   );
extern void	IP_castChannel		(imageP, int, imageP, int, int);
extern void	IP_castChannels		(imageP, int*,imageP);
extern void	IP_castChannelsEq	(imageP, int, imageP);
extern void	IP_castChannelsMin	(imageP, int, imageP);
extern void	IP_castChannelsMax	(imageP, int, imageP);
extern void	IP_clearChannel		(imageP, int, imageP	   );
extern void	IP_transferChannel	(imageP, int, int,int,int  );
extern void	IP_transferChannelMem	(imageP, int, int,int,int,void*);
extern int	IP_maxChannel		(imageP);
extern int	IP_maxType		(imageP);
extern int	IP_checkDimensions	(imageP, imageP);
extern void	IP_findImagePtr		(void*, imageP*, int*, int*);

/*	IPmmimg.c	*/
extern imageP	IP_allocImage		(int, int, int);
extern imageP	IP_allocImage_I		(imageP);
extern void	IP_allocImageInI	(imageP, int, int, int*);
extern void	IP_freeImage		(imageP);
extern void	IP_copyImage		(imageP, imageP);
extern void	IP_copyHeader		(imageP, int, imageP);
extern void	IP_copyHeader2		(imageP, imageP, int, imageP);
extern void	IP_copyImageBuffer	(imageP, imageP);

/*	IPmorph.c	*/
extern void	IP_shrink	(imageP, int, imageP);
extern void	IP_dilate	(imageP, int, imageP);

/*	IPplot.c	*/
extern void	IP_plot		(imageP, int, int, int, int, double, double);

/*	IPpolar.c	*/
extern void	IP_polarToRect	 (imageP, imageP);
extern void	IP_rectToPolar	 (imageP, imageP);
extern void	IP_rectToLogPolar(imageP, imageP);

/*	IPthin.c	*/
extern void	IP_thin		(imageP, int, int*, int*);
extern void	IP_thinBridge	(imageP, int, int, int, int, int*, int*);

/*	IPutil.c	*/
extern void	IP_interleave	(imageP, imageP);
extern void	IP_uninterleave	(imageP, imageP);
extern void	IP_normalizeDimensions(imageP, imageP);
extern void	IP_crop		(imageP, int, int, int, int, imageP);
extern void	IP_shift	(imageP, int,  int, imageP);
extern void	IP_pad		(imageP, int*, int, imageP);
extern void	IP_pad1D	(uchar*, int, int, int, int, uchar*);
extern void	IP_fpad1D	(float*, int, int, int, int, float*);
extern void	IP_clearImage	(imageP);
extern void	IP_setImage	(imageP, double*);
extern void	IP_minmaxImage	(imageP, double*, double*);
extern void	IP_minmaxChannel(imageP, int, double*, double*);
extern void	IP_RGBdecouple	(imageP, imageP*);
extern void	IP_RGBcouple	(imageP, imageP, imageP);
extern void	IP_bailout	(char*, ...);

/*	IPwarp*.c	*/
extern void	IP_warpCoons	  (void);
extern void	IP_warpSpatialLuts(imageP,imageP,imageP,imageP,imageP);
extern void	IP_warpMesh(imageP,imageP,imageP,imageP,imageP,imageP);

/* include header files */
#ifndef LINUX
#include "UIMS.h"
#include "MP.h"
#include "G2.h"
#ifdef GL
#include "G3.h"
#endif
#endif

#endif
