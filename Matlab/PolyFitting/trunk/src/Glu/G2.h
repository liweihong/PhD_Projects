/* ======================================================================
 * IMPROC: Image Processing Software Package
 * Copyright (C) 2002 by George Wolberg
 *
 * G2.h - IMPROC 2-D graphics package header file
 *
 * Written by: George Wolberg, 2002
 * ======================================================================
 */

/* Define current drawing window */
//#define WINDOW	(RENDER_FULL ? RENDER_FULL : \
//		 RENDER_PART ? RENDER_PART : \
//		 ImageWin[DrawImage].full.window ? \
//		 ImageWin[DrawImage].full.window : \
//		 ImageWin[DrawImage].part.window)
#define WINDOW	ImageWin[DrawImage].mywin.window

/* Define immediate drawing macros	*/
#define	G2_pointI(x, y)			G2_point(x, y),			G2_flush()
#define	G2_lineI(x1, y1, x2, y2)	G2_line(x1, y1, x2, y2),	G2_flush()
#define	G2_rectangleI(x1, y1, x2, y2)	G2_rectangle(x1, y1, x2, y2),	G2_flush()
#define	G2_circleI(x, y, r)		G2_circle(x, y, r),		G2_flush()
#define	G2_ellipseI(x, y, w, h)		G2_ellipse(x, y, w, h),		G2_flush()
#define	G2_arcI(x, y, w, h, a1, a2)	G2_arc(x, y, w, h, a1, a2),	G2_flush()
#define	G2_textI(x, y, s)		G2_text(x, y, s),		G2_flush()
#define G2_polyPointI(n, x, y)		G2_polyPoint(n, x, y),		G2_flush()
#define G2_polyLineI(n, x, y)		G2_polyLine(n, x, y),		G2_flush()
#define G2_polyCircleI(n, x, y, r)	G2_polyCircle(n, x, y, r),	G2_flush()
#define G2_polyPointI_I(I)		G2_polyPoint_I(I),		G2_flush()
#define G2_polyLineI_I(I)		G2_polyLine_I(I),		G2_flush()
#define G2_polyCircleI_I(I)		G2_polyCircle_I(I),		G2_flush()

/* value of y-coordinate A when y-axis rises upward (flipped) */ 
#define YUP(A)	(ImageWin[DrawImage].height-1 - (A))

/* Define line styles	*/
//#define G2_Solid	LineSolid
//#define G2_Dash		LineOnOffDash
//#define G2_DoubleDash	LineDoubleDash
#define G2_Solid		PS_SOLID
#define G2_Dash			PS_DASH
#define G2_DoubleDash	PS_DASHDOTDOT

/* Define spline types  */
enum spline_types {
	G2_Bezier,
	G2_Bspline,
	G2_CatmullRom,
	G2_Cspline,
	G2_Lspline
};

/* Global variables	*/
extern int	FillMode;
extern int	CloseMode;
extern int	DirectDrawMode;


/*	G2init	*/
extern void G2_init();
/*	G2canvas.c	*/
extern void	G2_makeCanvas	(int, int, int*);
extern void	G2_setCanvasRGB	(int, int, int );
extern void	G2_setCanvasGray(int );
extern void	G2_blankCanvas	(void);
extern void	G2_displayCanvas(void);

/*	G2draw.c	*/
extern void	G2_point	(int, int);
extern void	G2_line		(int, int, int, int);
extern void	G2_rectangle	(int, int, int, int);
extern void	G2_circle	(int, int, int     );
extern void	G2_ellipse	(int, int, int, int);
//extern void	G2_arc		(int, int, int, int, int, int);
extern void	G2_text		(int, int, char*);
extern void	G2_polyPoint	(int, int*, int*);
extern void	G2_polyLine	(int, int*, int*);
extern void	G2_polyCircle	(int, int*, int*, int);
extern void	G2_polyPoint_I	(imageP);
extern void	G2_polyLine_I	(imageP);
extern void	G2_polyCircle_I	(imageP, int);
extern void	G2_flush	(void);

/*	G2rubberband.c	*/
extern int	G2_lineR	(int*, int*, int*, int*);
extern int	G2_rectangleR	(int*, int*, int*, int*);
extern int	G2_rectangleCenterR(int*, int*, int*, int*);
extern int	G2_circleR	(int*, int*, int*      );
extern int	G2_ellipseR	(int*, int*, int*, int*);
extern int	G2_polyLineR	(int*, int*, int*, int );
extern int	G2_splineR	(int*, int*, int*, int, int);

/*	G2set.c		*/
extern void	G2_setGray	(int);
extern void	G2_setBackgroundGray(int);
extern void	G2_setRGB	(int, int, int);
extern void	G2_setBackgroundRGB(int, int, int);
extern void	G2_setXOR	(int);
extern void	G2_setDirectDraw(int);
extern void	G2_setFill	(int);
extern void	G2_setClose	(int);
extern void	G2_setLineWidth	(int);
extern void	G2_setLineStyle	(UINT);
extern void	G2_setDashes	(int, char*, int);
//extern ulong	G2_allocColor	(Colormap, int, int, int);

/*	G2spline.c	*/
extern void	G2_bezier	(imageP, int, imageP);
extern void	G2_bspline	(imageP, int, int, float*, imageP);
extern void	G2_catmullRom	(imageP, int, int, int, imageP);
extern void	G2_cspline	(imageP, int, imageP);
extern void	G2_lspline	(imageP, int, imageP);

/*	G2splineXY.c	*/
extern void	G2_catmullRomXY	(float*,float*,int,float*,float*,int);
extern void	G2_csplineXY	(float*,float*,int,float*,float*,int);
extern void	G2_lsplineXY	(float*,float*,int,float*,float*,int);

/*	G2translate.c	*/
extern int	G2_lineT	(int*, int*, int*, int*);
extern int	G2_rectangleT	(int*, int*, int*, int*);
extern int	G2_rectangleCenterT(int*, int*, int*, int*);
extern int	G2_circleT	(int*, int*, int*      );
extern int	G2_ellipseT	(int*, int*, int*, int*);
