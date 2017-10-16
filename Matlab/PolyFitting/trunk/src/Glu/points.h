#ifndef POINTS_H
#define POINTS_H

#include "IP.h"
#include "math.h"

#ifdef LINUX
#define FALSE false
#define TRUE  true
#define BOOL  bool
#endif

typedef		int		point2i[2];
typedef		float	point2f[2];
typedef		float	point3f[3];
typedef		float	vector2f[2];
typedef		float	vector3f[3];

typedef struct {
	point3f  point;
	vector3f normal;
}myPlaneS,*myPlaneP;

typedef struct {
	int width;
	int height;
	int ptCount;
	float *ptxX, *ptxY, *ptxZ, *ptxR;
}pointCloudS, *pointCloudP;

typedef struct {
	point3f		corners[4];
	point3f		point;
	vector3f	normal;
}BPlaneS, *BPlaneP;			//bounded plane

int loadPtxFile(char *filename, pointCloudP ptCloud);
int savePtxFile(char *filename,pointCloudP ptCloud);
BOOL IsNullPoint(float x, float y, float z);
int convToSphere(float *vec, int *theta, int *phi);
int convToCartesian(int theta, int phi, float *x, float *y, float *z);
void cross(float *v1, float *v2, float *norm);
float dot(vector3f v1, vector3f v2);
int Angle(vector3f v1, vector3f v2);
void projVu(vector3f u, vector3f v, vector3f proj);
float pointPointDist(point3f P1, point3f P2);
float pointPlaneDist(myPlaneS mPlane, point3f P);
void pointPlaneProj(myPlaneS mPlane, point3f P, point3f projP);
float pointLineDist(point3f linePt, vector3f dirVec, point3f Pt);

void linePlaneIntersect(BPlaneS bPlane, point3f pt1, point3f pt2, point3f projPt);
void pointPlaneDirProj(myPlaneS mPlane,point3f pt0, point3f pt1, point3f projPt);

#endif
