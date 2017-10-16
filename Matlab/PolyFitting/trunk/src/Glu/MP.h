/* ======================================================================
 * IMPROC: Image Processing Software Package
 * Copyright (C) 2002 by George Wolberg
 *
 * MP.h - IMPROC math package header file
 *
 * Written by: George Wolberg, 2002
 * ======================================================================
 */


typedef imageP matrixP;
typedef int    vector2i[2];	/* int   version  */
typedef int    vector3i[3];
typedef int    vector4i[4];
typedef float  vector2f[2];	/* float version  */
typedef float  vector3f[3];
typedef float  vector4f[4];
typedef double vector2d[2];	/* double version */
typedef double vector3d[3];
typedef double vector4d[4];
typedef float  matrix4f[16];

#define EPSILON		1.0e-10
#define NEWMATRIX	NEWIMAGE
#define ROWS(M)		((M)->height)
#define COLS(M)		((M)->width )
#define BUF(M)		((double*) (M)->buf[0])

static matrix4f Mi = {
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
};

/* MP function macros */
#define MP_allocMatrix(R, C)		IP_allocImage(C, R, DOUBLECH_TYPE)
#define MP_freeMatrix(M)		IP_freeImage(M)
#define MP_readMatrix(F)		IP_readImage(F)
#define MP_saveMatrix(M, F)		IP_saveImage(M, F)
#define MP_clearMatrix(M)		IP_clearImage(M)
#define MP_matrixType(M)		(M->chtype[0])
#define MP_castMatrix(M, T)		IP_castChannel(M, 0, M, 0, T)
#define MP_copyMatrix(S, D)		IP_copyImage(S, D);
#define MP_identityMatrix4f(M)	memcpy((char*) M, (char*) Mi, sizeof(matrix4f))
#define MP_makeMatrix(R, C, M)	\
	{			\
	M->width  = C;		\
	M->height = R;		\
	M->imagetype= 1;	\
	IP_initChannels(M, DOUBLECH_TYPE);\
	}

/*	MParith.c	*/
extern void	MP_matrixMultiply	(matrixP, matrixP, matrixP);
extern void	MP_identityMatrix	(matrixP);
extern double	MP_determinant		(matrixP);
extern void	MP_transpose		(matrixP, matrixP);

/*	MPband.c	*/
extern void	MP_tridiagonal		(matrixP, matrixP);

/*	MPgauss.c	*/
extern void	MP_GaussJordan		(matrixP, matrixP, matrixP);
extern void	MP_GaussElimination	(matrixP, matrixP, matrixP);

/*	MPlu.c		*/
extern void	MP_LUdcmp		(matrixP, int*, double*);
extern void	MP_LUbksb		(matrixP, int*, matrixP);
extern void	MP_improveX		(matrixP, matrixP, int*, matrixP, matrixP);

/*	MPquaternion.c	*/
extern void	MP_quaternionMul	(vector4f, vector4f, vector4f);
extern void	MP_quaternionToRot	(vector4f, vector4f);
extern void	MP_initRot		(float, int, float*, float*);


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Useful 2D vector macros:
 *	MP_vectorCopy2	   (A, B)	B = A
 *	MP_vectorAdd2	   (A, B, C)	C = A + B
 *	MP_vectorSub2	   (A, B, C)	C = A - B
 *	MP_vectorMul2	   (A, B, C)	C = A * B
 *	MP_vectorMulScalar2(A, B, C)	C = A * B where B is a scalar
 *	MP_vectorDot2	   (A)		A dot B (dot product scalar)
 *	MP_vectorNorm2	   (A)		norm   of A: sqrt(A dot A)
 *	MP_vectorLength2   (A)		length of A: sqrt(A dot A)
 *	MP_vectorNormalize2(A, B)	normalize vector A into unit vector B
 */

#define MP_vectorCopy2(A, B)	\
	{			\
	B[0] = A[0];		\
	B[1] = A[1];		\
	}

#define MP_vectorAdd2(A, B, C)	\
	{			\
	C[0] = A[0] + B[0];	\
	C[1] = A[1] + B[1];	\
	}

#define MP_vectorSub2(A, B, C)	\
	{			\
	C[0] = A[0] - B[0];	\
	C[1] = A[1] - B[1];	\
	}

#define MP_vectorMul2(A, B, C)	\
	{			\
	C[0] = A[0] * B[0];	\
	C[1] = A[1] * B[1];	\
	}

#define MP_vectorMulScalar2(A, B, C)	\
	{			\
	C[0] = A[0] * B;	\
	C[1] = A[1] * B;	\
	}

#define MP_vectorDot2(A, B)	((A[0]*B[0]) + (A[1]*B[1]))
#define MP_vectorNorm2(A)	(sqrt( MP_vectorDot2(A,A) ))
#define MP_vectorLength2(A)	(sqrt( MP_vectorDot2(A,A) ))
#define MP_vectorNormalize2(A, B)	\
	{				\
	double _len;			\
	_len = MP_vectorNorm2(A);	\
	if(_len == 0) _len = 1;		\
	B[0] = A[0] / _len;		\
	B[1] = A[1] / _len;		\
	}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Useful 3D vector macros:
 *	MP_vectorCopy3	   (A, B)	B = A
 *	MP_vectorAdd3	   (A, B, C)	C = A + B
 *	MP_vectorSub3	   (A, B, C)	C = A - B
 *	MP_vectorMul3	   (A, B, C)	C = A * B
 *	MP_vectorMulScalar3(A, B, C)	C = A * B where B is a scalar
 *	MP_vectorDot3	   (A)		A dot B (dot product scalar)
 *	MP_vectorNorm3	   (A)		norm   of A: sqrt(A dot A)
 *	MP_vectorLength3   (A)		length of A: sqrt(A dot A)
 *	MP_vectorNormalize3(A, B)	normalize vector A into unit vector B
 *	MP_vectorCross3	   (A, B, C)	C = A x B (cross product); float version
 *	MP_vectorCrossD3   (A, B, C)	C = A x B (cross product);double version
 *	MP_vectorTransform3(A, B, C)	C = A * B where B is a 3x3 matrix
 */

#define MP_vectorCopy3(A, B)	\
	{			\
	B[0] = A[0];		\
	B[1] = A[1];		\
	B[2] = A[2];		\
	}

#define MP_vectorAdd3(A, B, C)	\
	{			\
	C[0] = A[0] + B[0];	\
	C[1] = A[1] + B[1];	\
	C[2] = A[2] + B[2];	\
	}

#define MP_vectorSub3(A, B, C)	\
	{			\
	C[0] = A[0] - B[0];	\
	C[1] = A[1] - B[1];	\
	C[2] = A[2] - B[2];	\
	}

#define MP_vectorMul3(A, B, C)	\
	{			\
	C[0] = A[0] * B[0];	\
	C[1] = A[1] * B[1];	\
	C[2] = A[2] * B[2];	\
	}

#define MP_vectorMulScalar3(A, B, C)	\
	{			\
	C[0] = A[0] * B;	\
	C[1] = A[1] * B;	\
	C[2] = A[2] * B;	\
	}

#define MP_vectorDot3(A, B)	((A[0]*B[0]) + (A[1]*B[1]) + (A[2]*B[2]))
#define MP_vectorNorm3(A)	(sqrt( MP_vectorDot3(A,A) ))
#define MP_vectorLength3(A)	(sqrt( MP_vectorDot3(A,A) ))
#define MP_vectorNormalize3(A, B)	\
	{				\
	double _len;			\
	_len = MP_vectorNorm3(A);	\
	if(_len == 0) _len = 1;		\
	B[0] = A[0] / _len;		\
	B[1] = A[1] / _len;		\
	B[2] = A[2] / _len;		\
	}

/* float version (default) */
#define MP_vectorCross3(A, B, C)	\
	{				\
	vector3f _T;			\
	_T[0] = A[1]*B[2] - A[2]*B[1];	\
	_T[1] = A[2]*B[0] - A[0]*B[2];	\
	_T[2] = A[0]*B[1] - A[1]*B[0];	\
	memcpy((char*) C, (char*) _T, sizeof(vector3f)); \
	}

/* double version */
#define MP_vectorCrossD3(A, B, C)	\
	{				\
	vector3d _T;			\
	_T[0] = A[1]*B[2] - A[2]*B[1];	\
	_T[1] = A[2]*B[0] - A[0]*B[2];	\
	_T[2] = A[0]*B[1] - A[1]*B[0];	\
	memcpy((char*) C, (char*) _T, sizeof(vector3d)); \
	}

#define MP_vectorTransform3(A, B, C)	\
	{				\
	vector3f _T;			\
	_T[0] = A[0]*B[0] + A[1]*B[3] + A[2]*B[6]; \
	_T[1] = A[0]*B[1] + A[1]*B[4] + A[2]*B[7]; \
	_T[2] = A[0]*B[2] + A[1]*B[5] + A[2]*B[8]; \
	memcpy((char*) C, (char*) _T, sizeof(vector3f)); \
	}



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Useful 4D vector macros:
 *	MP_vectorCopy4	   (A, B)	B = A
 *	MP_vectorAdd4	   (A, B, C)	C = A + B
 *	MP_vectorSub4	   (A, B, C)	C = A - B
 *	MP_vectorMul4	   (A, B, C)	C = A * B
 *	MP_vectorMulScalar4(A, B, C)	C = A * B where B is a scalar
 *	MP_vectorDot4	   (A)		A dot B (dot product scalar)
 *	MP_vectorNorm4	   (A)		norm   of A: sqrt(A dot A)
 *	MP_vectorLength4   (A)		length of A: sqrt(A dot A)
 *	MP_vectorNormalize4(A, B)	normalize vector A into unit vector B
 *	MP_vectorTransform4(A, B, C)	C = A * B where B is a 4x4 matrix
 */

#define MP_vectorCopy4(A, B)	\
	{			\
	B[0] = A[0];		\
	B[1] = A[1];		\
	B[2] = A[2];		\
	B[3] = A[3];		\
	}

#define MP_vectorAdd4(A, B, C)	\
	{			\
	C[0] = A[0] + B[0];	\
	C[1] = A[1] + B[1];	\
	C[2] = A[2] + B[2];	\
	C[3] = A[3] + B[3];	\
	}

#define MP_vectorSub4(A, B, C)	\
	{			\
	C[0] = A[0] - B[0];	\
	C[1] = A[1] - B[1];	\
	C[2] = A[2] - B[2];	\
	C[3] = A[3] - B[3];	\
	}

#define MP_vectorMul4(A, B, C)	\
	{			\
	C[0] = A[0] * B[0];	\
	C[1] = A[1] * B[1];	\
	C[2] = A[2] * B[2];	\
	C[3] = A[3] * B[3];	\
	}

#define MP_vectorMulScalar4(A, B, C)	\
	{			\
	C[0] = A[0] * B;	\
	C[1] = A[1] * B;	\
	C[2] = A[2] * B;	\
	C[3] = A[3] * B;	\
	}

#define MP_vectorDot4(A, B)	((A[0]*B[0])+(A[1]*B[1])+(A[2]*B[2])+(A[3]*B[3]))
#define MP_vectorNorm4(A)	(sqrt( MP_vectorDot4(A,A) ))
#define MP_vectorLength4(A)	(sqrt( MP_vectorDot4(A,A) ))
#define MP_vectorNormalize4(A, B)	\
	{				\
	double _len;			\
	_len = MP_vectorNorm4(A);	\
	if(_len == 0) _len = 1;		\
	B[0] = A[0] / _len;		\
	B[1] = A[1] / _len;		\
	B[2] = A[2] / _len;		\
	B[3] = A[3] / _len;		\
	}

#define MP_vectorTransform4(A, B, C)	\
	{				\
	vector4f _T;			\
	_T[0] = A[0]*B[0] + A[1]*B[4] + A[2]*B[ 8] + A[3]*B[12]; \
	_T[1] = A[0]*B[1] + A[1]*B[5] + A[2]*B[ 9] + A[3]*B[13]; \
	_T[2] = A[0]*B[2] + A[1]*B[6] + A[2]*B[10] + A[3]*B[14]; \
	_T[3] = A[0]*B[3] + A[1]*B[7] + A[2]*B[11] + A[3]*B[15]; \
	memcpy((char*) C, (char*) _T, sizeof(vector4f));	 \
	}
