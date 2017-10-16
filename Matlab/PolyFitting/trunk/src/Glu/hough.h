#ifndef HOUGH_H
#define HOUGH_H


#include "IP.h"
#include "myMatrix.h"
#include "dl.h"
#include "points.h"

typedef		float	point2f[2];
typedef		float	point3f[3];
typedef		float	vector2f[2];
typedef		float	vector3f[3];

/*typedef struct {
	point3f		corners[4];
	point3f		point;
	vector3f	normal;
}BPlaneS, *BPlaneP;			//bounded plane
*/

void hough_plane_cb();
void hough_plane(imageP I, pointCloudP ptCloud, int topNum,vector3f *topNorms, int bDisplay);
int getVertNorm(int itmp, vector3f norm1,int ptxWidth,int ptxHeight,
				float *ptxX, float *ptxY,float *ptxZ, vector3f avgXYDir, int r);
float getDistFromPlane(myPlaneS plane, float x, float y, float z);
void findAvgXYDir(int ptxWidth,int ptxHeight,int ptxPtCount,
				  float *ptxX, float *ptxY, float *ptxZ, float *ptxR, vector3f avgXYDir);
void normalize(vector3f vec, vector3f normVec);
void assignIdsToPoints(pointCloudP ptc, int topNum,vector3f *topNorms,int *planeIds);
void assignIdsToPointsByDist(pointCloudP ptc,int topCount,vector3f *topNorms, int *planeIds,
							 int *topCount2, myPlaneP *mPlanes, int *planeIds2, int topPerNorm);

#endif
