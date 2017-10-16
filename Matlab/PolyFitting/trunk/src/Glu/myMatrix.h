#ifndef MYMATRIX_H
#define MYMATRIX_H

typedef struct {
	int m;
	int n;
	float *data;
}myMatrixS, *myMatrixP;

myMatrixP newMatrix(int m, int n);
void freeMatrix(myMatrixP M);

myMatrixP inverseMatrix(myMatrixS M);
float detMatrix(myMatrixS M);
float cofactorM(int x, int y, myMatrixS M);
myMatrixP adjugateMatrix(myMatrixS M);
myMatrixP transposeMatrix(myMatrixS M);
myMatrixP addMatrix(myMatrixS M, myMatrixS N);
myMatrixP subMatrix(myMatrixS M, myMatrixS N);
myMatrixP mulMatrixConst(myMatrixS M, float c);
myMatrixP mulMatrix(myMatrixS M, myMatrixS N);
myMatrixP pseudoInverse(myMatrixS M);
void printMatrix(myMatrixS M);

#endif