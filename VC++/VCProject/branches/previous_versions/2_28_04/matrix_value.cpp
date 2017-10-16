//
// MATLAB Compiler: 2.2
// Date: Sat Feb 14 22:22:52 2004
// Arguments: "-B" "macro_default" "-O" "all" "-O" "fold_scalar_mxarrays:on"
// "-O" "fold_non_scalar_mxarrays:on" "-O" "optimize_integer_for_loops:on" "-O"
// "array_indexing:on" "-O" "optimize_conditionals:on" "-t" "-L" "Cpp"
// "matrix_value" 
//
#include "matrix_value.hpp"
#include "libmatlbm.hpp"

static mxChar _array1_[142] = { 'R', 'u', 'n', '-', 't', 'i', 'm', 'e', ' ',
                                'E', 'r', 'r', 'o', 'r', ':', ' ', 'F', 'i',
                                'l', 'e', ':', ' ', 'm', 'a', 't', 'r', 'i',
                                'x', '_', 'v', 'a', 'l', 'u', 'e', ' ', 'L',
                                'i', 'n', 'e', ':', ' ', '1', ' ', 'C', 'o',
                                'l', 'u', 'm', 'n', ':', ' ', '1', ' ', 'T',
                                'h', 'e', ' ', 'f', 'u', 'n', 'c', 't', 'i',
                                'o', 'n', ' ', '"', 'm', 'a', 't', 'r', 'i',
                                'x', '_', 'v', 'a', 'l', 'u', 'e', '"', ' ',
                                'w', 'a', 's', ' ', 'c', 'a', 'l', 'l', 'e',
                                'd', ' ', 'w', 'i', 't', 'h', ' ', 'm', 'o',
                                'r', 'e', ' ', 't', 'h', 'a', 'n', ' ', 't',
                                'h', 'e', ' ', 'd', 'e', 'c', 'l', 'a', 'r',
                                'e', 'd', ' ', 'n', 'u', 'm', 'b', 'e', 'r',
                                ' ', 'o', 'f', ' ', 'o', 'u', 't', 'p', 'u',
                                't', 's', ' ', '(', '2', ')', '.' };
static mwArray _mxarray0_ = mclInitializeString(142, _array1_);

static mxChar _array3_[141] = { 'R', 'u', 'n', '-', 't', 'i', 'm', 'e', ' ',
                                'E', 'r', 'r', 'o', 'r', ':', ' ', 'F', 'i',
                                'l', 'e', ':', ' ', 'm', 'a', 't', 'r', 'i',
                                'x', '_', 'v', 'a', 'l', 'u', 'e', ' ', 'L',
                                'i', 'n', 'e', ':', ' ', '1', ' ', 'C', 'o',
                                'l', 'u', 'm', 'n', ':', ' ', '1', ' ', 'T',
                                'h', 'e', ' ', 'f', 'u', 'n', 'c', 't', 'i',
                                'o', 'n', ' ', '"', 'm', 'a', 't', 'r', 'i',
                                'x', '_', 'v', 'a', 'l', 'u', 'e', '"', ' ',
                                'w', 'a', 's', ' ', 'c', 'a', 'l', 'l', 'e',
                                'd', ' ', 'w', 'i', 't', 'h', ' ', 'm', 'o',
                                'r', 'e', ' ', 't', 'h', 'a', 'n', ' ', 't',
                                'h', 'e', ' ', 'd', 'e', 'c', 'l', 'a', 'r',
                                'e', 'd', ' ', 'n', 'u', 'm', 'b', 'e', 'r',
                                ' ', 'o', 'f', ' ', 'i', 'n', 'p', 'u', 't',
                                's', ' ', '(', '3', ')', '.' };
static mwArray _mxarray2_ = mclInitializeString(141, _array3_);
static mwArray _mxarray4_ = mclInitializeDouble(1.0);
static mwArray _mxarray5_ = mclInitializeDouble(2.0);
static mwArray _mxarray6_ = mclInitializeDoubleVector(0, 0, (double *)NULL);
static mwArray _mxarray7_ = mclInitializeDouble(0.0);
static mwArray _mxarray8_ = mclInitializeDouble(3.0);
static mwArray _mxarray9_ = mclInitializeDouble(4.0);
static mwArray _mxarray10_ = mclInitializeDouble(5.0);
static mwArray _mxarray11_ = mclInitializeDouble(6.0);
static mwArray _mxarray12_ = mclInitializeDouble(7.0);
static mwArray _mxarray13_ = mclInitializeDouble(8.0);
static mwArray _mxarray14_ = mclInitializeDouble(9.0);

void InitializeModule_matrix_value() {
}

void TerminateModule_matrix_value() {
}

static mwArray Mmatrix_value(mwArray * R_inv,
                             int nargout_,
                             mwArray dest_value,
                             mwArray mimio_value,
                             mwArray num);

_mexLocalFunctionTable _local_function_table_matrix_value
  = { 0, (mexFunctionTableEntry *)NULL };

//
// The function "matrix_value" contains the normal interface for the
// "matrix_value" M-function from file
// "C:\Liwh\CUNY\VC++\VCProject\matrix_value.m" (lines 1-46). This function
// processes any input arguments and passes them to the implementation version
// of the function, appearing above.
//
mwArray matrix_value(mwArray * R_inv,
                     mwArray dest_value,
                     mwArray mimio_value,
                     mwArray num) {
    int nargout(1);
    mwArray R(mclGetUninitializedArray());
    mwArray R_inv__(mclGetUninitializedArray());
    if (R_inv != NULL) {
        ++nargout;
    }
    R = Mmatrix_value(&R_inv__, nargout, dest_value, mimio_value, num);
    if (R_inv != NULL) {
        *R_inv = R_inv__;
    }
    return R;
}

//
// The function "mlxMatrix_value" contains the feval interface for the
// "matrix_value" M-function from file
// "C:\Liwh\CUNY\VC++\VCProject\matrix_value.m" (lines 1-46). The feval
// function calls the implementation version of matrix_value through this
// function. This function processes any input arguments and passes them to the
// implementation version of the function, appearing above.
//
void mlxMatrix_value(int nlhs, mxArray * plhs[], int nrhs, mxArray * prhs[]) {
    MW_BEGIN_MLX();
    {
        mwArray mprhs[3];
        mwArray mplhs[2];
        int i;
        mclCppUninitializeArrays(2, mplhs);
        if (nlhs > 2) {
            error(_mxarray0_);
        }
        if (nrhs > 3) {
            error(_mxarray2_);
        }
        for (i = 0; i < 3 && i < nrhs; ++i) {
            mprhs[i] = mwArray(prhs[i], 0);
        }
        for (; i < 3; ++i) {
            mprhs[i].MakeDIN();
        }
        mplhs[0] = Mmatrix_value(&mplhs[1], nlhs, mprhs[0], mprhs[1], mprhs[2]);
        plhs[0] = mplhs[0].FreezeData();
        for (i = 1; i < 2 && i < nlhs; ++i) {
            plhs[i] = mplhs[i].FreezeData();
        }
    }
    MW_END_MLX();
}

//
// The function "Mmatrix_value" is the implementation version of the
// "matrix_value" M-function from file
// "C:\Liwh\CUNY\VC++\VCProject\matrix_value.m" (lines 1-46). It contains the
// actual compiled code for that M-function. It is a static function and must
// only be called from one of the interface functions, appearing below.
//
//
// function [R , R_inv] = matrix_value(dest_value, mimio_value, num)
//
static mwArray Mmatrix_value(mwArray * R_inv,
                             int nargout_,
                             mwArray dest_value,
                             mwArray mimio_value,
                             mwArray num) {
    mwLocalFunctionTable save_local_function_table_
      (&_local_function_table_matrix_value);
    mwArray R(mclGetUninitializedArray());
    mwArray T(mclGetUninitializedArray());
    mwArray A(mclGetUninitializedArray());
    mwArray B(mclGetUninitializedArray());
    mwArray i(mclGetUninitializedArray());
    mwArray Y(mclGetUninitializedArray());
    mwArray X(mclGetUninitializedArray());
    mwArray Y0(mclGetUninitializedArray());
    mwArray X0(mclGetUninitializedArray());
    //
    // 
    // dest_value = dest_value';
    //
    dest_value = ctranspose(mwVa(dest_value, "dest_value"));
    //
    // mimio_value = mimio_value';
    //
    mimio_value = ctranspose(mwVa(mimio_value, "mimio_value"));
    //
    // 
    // X0 = mimio_value(:,1);
    //
    X0 = mclArrayRef(mwVsa(mimio_value, "mimio_value"), colon(), _mxarray4_);
    //
    // Y0 = mimio_value(:,2);
    //
    Y0 = mclArrayRef(mwVsa(mimio_value, "mimio_value"), colon(), _mxarray5_);
    //
    // X = dest_value(:,1);
    //
    X = mclArrayRef(mwVsa(dest_value, "dest_value"), colon(), _mxarray4_);
    //
    // Y = dest_value(:,2);
    //
    Y = mclArrayRef(mwVsa(dest_value, "dest_value"), colon(), _mxarray5_);
    //
    // 
    // for i = 1 : num
    //
    {
        int v_(mclForIntStart(1));
        int e_(mclForIntEnd(mwVa(num, "num")));
        if (v_ > e_) {
            i = _mxarray6_;
        } else {
            //
            // B(i*2 - 1,1) = X(i); 
            // B(i*2,1) = Y(i);
            // end;   
            //
            for (; ; ) {
                mclArrayAssign(
                  &B,
                  mclIntArrayRef(mwVsv(X, "X"), v_),
                  v_ * _mxarray5_ - _mxarray4_,
                  _mxarray4_);
                mclArrayAssign(
                  &B,
                  mclIntArrayRef(mwVsv(Y, "Y"), v_),
                  v_ * _mxarray5_,
                  _mxarray4_);
                if (v_ == e_) {
                    break;
                }
                ++v_;
            }
            i = v_;
        }
    }
    //
    // 
    // for i = 1 : num
    //
    {
        int v_(mclForIntStart(1));
        int e_(mclForIntEnd(mwVa(num, "num")));
        if (v_ > e_) {
            i = _mxarray6_;
        } else {
            //
            // A(i*2 - 1, 1) = X0(i);
            // A(i*2, 1) = 0;
            // A(i*2 - 1, 2) = Y0(i);
            // A(i*2, 2) = 0;
            // A(i*2 - 1, 3) = 1;
            // A(i*2, 3) = 0;
            // A(i*2 - 1, 4) = 0;
            // A(i*2, 4) = X0(i);
            // A(i*2 - 1, 5) = 0;
            // A(i*2, 5) = Y0(i);
            // A(i*2 - 1, 6) = 0;
            // A(i*2, 6) = 1;
            // A(i*2 - 1, 7) = -X(i)*X0(i);
            // A(i*2, 7) = -X0(i)*Y(i);
            // A(i*2 - 1, 8) = -Y0(i)*X(i);
            // A(i*2, 8) = -Y(i)*Y0(i);
            // end;
            //
            for (; ; ) {
                mclArrayAssign(
                  &A,
                  mclIntArrayRef(mwVsv(X0, "X0"), v_),
                  v_ * _mxarray5_ - _mxarray4_,
                  _mxarray4_);
                mclArrayAssign(&A, _mxarray7_, v_ * _mxarray5_, _mxarray4_);
                mclArrayAssign(
                  &A,
                  mclIntArrayRef(mwVsv(Y0, "Y0"), v_),
                  v_ * _mxarray5_ - _mxarray4_,
                  _mxarray5_);
                mclArrayAssign(&A, _mxarray7_, v_ * _mxarray5_, _mxarray5_);
                mclArrayAssign(
                  &A, _mxarray4_, v_ * _mxarray5_ - _mxarray4_, _mxarray8_);
                mclArrayAssign(&A, _mxarray7_, v_ * _mxarray5_, _mxarray8_);
                mclArrayAssign(
                  &A, _mxarray7_, v_ * _mxarray5_ - _mxarray4_, _mxarray9_);
                mclArrayAssign(
                  &A,
                  mclIntArrayRef(mwVsv(X0, "X0"), v_),
                  v_ * _mxarray5_,
                  _mxarray9_);
                mclArrayAssign(
                  &A, _mxarray7_, v_ * _mxarray5_ - _mxarray4_, _mxarray10_);
                mclArrayAssign(
                  &A,
                  mclIntArrayRef(mwVsv(Y0, "Y0"), v_),
                  v_ * _mxarray5_,
                  _mxarray10_);
                mclArrayAssign(
                  &A, _mxarray7_, v_ * _mxarray5_ - _mxarray4_, _mxarray11_);
                mclArrayAssign(&A, _mxarray4_, v_ * _mxarray5_, _mxarray11_);
                mclArrayAssign(
                  &A,
                  - mwVe(mclIntArrayRef(mwVsv(X, "X"), v_))
                  * mwVe(mclIntArrayRef(mwVsv(X0, "X0"), v_)),
                  v_ * _mxarray5_ - _mxarray4_,
                  _mxarray12_);
                mclArrayAssign(
                  &A,
                  - mwVe(mclIntArrayRef(mwVsv(X0, "X0"), v_))
                  * mwVe(mclIntArrayRef(mwVsv(Y, "Y"), v_)),
                  v_ * _mxarray5_,
                  _mxarray12_);
                mclArrayAssign(
                  &A,
                  - mwVe(mclIntArrayRef(mwVsv(Y0, "Y0"), v_))
                  * mwVe(mclIntArrayRef(mwVsv(X, "X"), v_)),
                  v_ * _mxarray5_ - _mxarray4_,
                  _mxarray13_);
                mclArrayAssign(
                  &A,
                  - mwVe(mclIntArrayRef(mwVsv(Y, "Y"), v_))
                  * mwVe(mclIntArrayRef(mwVsv(Y0, "Y0"), v_)),
                  v_ * _mxarray5_,
                  _mxarray13_);
                if (v_ == e_) {
                    break;
                }
                ++v_;
            }
            i = v_;
        }
    }
    //
    // 
    // R = A\B;
    //
    R = mldivide(mwVv(A, "A"), mwVv(B, "B"));
    //
    // T = R;
    //
    T = mwVsv(R, "R");
    //
    // 
    // R = [R',1];
    //
    R = horzcat(mwVarargin(ctranspose(mwVv(R, "R")), _mxarray4_));
    //
    // R = reshape(R,3,3);
    //
    R = reshape(mwVv(R, "R"), mwVarargin(_mxarray8_, _mxarray8_));
    //
    // R = inv(R);
    //
    R = inv(mwVv(R, "R"));
    //
    // R = reshape(R,9,1);
    //
    R = reshape(mwVv(R, "R"), mwVarargin(_mxarray14_, _mxarray4_));
    //
    // R = R/R(9,1);
    //
    R = mwVv(R, "R") / mwVe(mclIntArrayRef(mwVsv(R, "R"), 9, 1));
    //
    // 
    // R_inv = R
    //
    *R_inv = mwVsv(R, "R");
    mwVsv(*R_inv, "R_inv").Print("R_inv");
    //
    // R = T
    //
    R = mwVsv(T, "T");
    mwVsv(R, "R").Print("R");
    mwValidateOutput(R, 1, nargout_, "R", "matrix_value");
    mwValidateOutput(*R_inv, 2, nargout_, "R_inv", "matrix_value");
    return R;
}
