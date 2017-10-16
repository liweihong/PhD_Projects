
/* ======================================================================
 * IMPROC: Image Processing Software Package
 * Copyright (C) 1997 by George Wolberg
 *
 * reg.h - Header file for registration code.
 *
 * Written by: George Wolberg, 1997
 * ======================================================================
 */

#ifndef DL_H
#define	DL_H

#include "IP.h"

#ifdef LINUX
#define BOOL bool
#endif

typedef struct {
	int xmin;
	int xmax;
	int ymin;
	int ymax;
}boundaryS, *boundaryP;

//extern void	label(imageP, imageP);
void label(imageP I, imageP Iout, int regColor, int *regNum, boundaryP *bn, BOOL bSort,
		   int scaleOutImage);

#endif
