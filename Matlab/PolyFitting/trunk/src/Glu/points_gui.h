#ifndef	POINTS_GUI_H
#define POINTS_GUI_H

#include "points.h"

#define MXPOINTS	1500000
#define RADIUS_FCTR	0.8
#define R1		0.7	/* inner radius of torus */
#define R2		0.3	/* outer radius of torus */
enum	{TX, TY, TZ, ROTX, ROTY, ROTZ, SPEED, RESOLUTION};
enum	{SPHERE, TORUS};

/* global variable data structure */
typedef struct {
	/* interface vars */
	int	WW, HH;
	int	XX, YY;
	int	UpdateMode;
	int	WorldCoord;
	int	DrawGrid;
	int	DrawAxes;
	int	Cull;
	int	Constraint;
	int	ButtonPressed;
	float	Speed;

	/* quaternion vars */
	float	 dRot,  dT;
	vector3f TT;
	vector4f VV,  Axis;
	matrix4f Rot, RotInc;
	GLuint	 Interface;

	/* point cloud */
	GLuint	 PointCloud;
	float *X, *Y, *Z, *R;
	int	 NPoints;
	vector3f Center;
	vector3f Min, Max;
} globalS, *globalP;

/* macros to access global vars */
#define GLOBAL		((globalP) RENDER_VARS)
#define WW		GLOBAL->WW
#define HH		GLOBAL->HH
#define XX		GLOBAL->XX
#define YY		GLOBAL->YY
#define UpdateMode	GLOBAL->UpdateMode
#define WorldCoord	GLOBAL->WorldCoord
#define DrawGrid	GLOBAL->DrawGrid
#define DrawAxes	GLOBAL->DrawAxes
#define Cull		GLOBAL->Cull
#define Constraint	GLOBAL->Constraint
#define ButtonPressed	GLOBAL->ButtonPressed
#define Speed		GLOBAL->Speed
#define dRot		GLOBAL->dRot
#define dT		GLOBAL->dT
#define TT		GLOBAL->TT
#define VV		GLOBAL->VV
#define Axis		GLOBAL->Axis
#define Rot		GLOBAL->Rot
#define RotInc		GLOBAL->RotInc
#define PointCloud	GLOBAL->PointCloud
#define Points		GLOBAL->Points
#define X		GLOBAL->X
#define Y		GLOBAL->Y
#define Z		GLOBAL->Z
#define R		GLOBAL->R
#define G		GLOBAL->G
#define B		GLOBAL->B
#define NPoints		GLOBAL->NPoints
#define Center		GLOBAL->Center
#define Interface	GLOBAL->Interface
#define Min		GLOBAL->Min
#define Max		GLOBAL->Max

static void render		(void);
static void animate		(void);
static void display		(void);
static void reshape		(int, int);
static  int parseEvent		(void);
static void menu		(void);
static void drawPoints		(void);
static void parse		(char *);
static void init		(void);
static void initDisplayLists	(void);

/* light positions and colors */
static vector4f LightPosition[2] = {{2.0,-2.0,2.0,1.0}, {-1.5,2.5,-1.5,1.0}};
static vector4f LightColor   [2] = {{0.6, 0.6,1.0,1.0}, { 0.9,0.6, 0.0,1.0}};

static void MY_drawQInterface(float z, float r);

void display3D(pointCloudP ptCloud);

#endif
