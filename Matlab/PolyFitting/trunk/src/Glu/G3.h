/* ======================================================================
 * IMPROC: Image Processing Software Package
 * Copyright (C) 2002 by George Wolberg
 *
 * G3.h - IMPROC 3-D graphics package header file
 *
 * Written by: George Wolberg, 2002
 * ======================================================================
 */


/* ----------------------------------------------------------------------
 * Standard GL include files
 */
#include <GL/gl.h>
#include <GL/glu.h>
//#include <GL/glx.h>
#include <gl\glaux.h>


 
/* ----------------------------------------------------------------------
 * OpenGL global variables
 */
extern	int		DoubleBuffer;
//extern	GLXContext	RenderingContext[MXWINDOW];
extern	HGLRC		RenderingContext[MXWINDOW];
enum   {G3_IN, G3_OUT, G3_NS, G3_EW, G3_RING};


/* ----------------------------------------------------------------------
 * Function externs
 */
/*	G3canvas.c	*/
extern void	G3_makeCanvas		(int, int, PFV, PFV, PFV, PFI, PFV);
extern void	G3_displayCanvas	(void);
extern void	G3_displayCanvasInPlace	(void);

/*	G3render.c	*/
extern int	G3_initRenderVars	(void*);
extern void	G3_makeRenderWindow	(void);
extern void	G3_flush		(void);

/*	G3quaternion.c	*/
extern void	G3_drawQInterface	(float, float);
extern void	G3_circle		(float, float, float, float);
extern int	G3_pointInQCircle	(int, int, int, int);
extern void	G3_pointOnQSphere	(int,int,int,int,int,vector4f,vector4f);
