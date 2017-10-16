//
// MATLAB Compiler: 2.2
// Date: Fri Feb 13 18:14:26 2004
// Arguments: "-B" "macro_default" "-O" "all" "-O" "fold_scalar_mxarrays:on"
// "-O" "fold_non_scalar_mxarrays:on" "-O" "optimize_integer_for_loops:on" "-O"
// "array_indexing:on" "-O" "optimize_conditionals:on" "-p" "-W" "main" "-L"
// "Cpp" "-t" "-T" "link:exe" "-h" "libmmfile.mlib" "presp_transform" 
//
#include "presp_transform.hpp"
#include "libmatlbm.hpp"

static mxChar _array1_[148] = { 'R', 'u', 'n', '-', 't', 'i', 'm', 'e', ' ',
                                'E', 'r', 'r', 'o', 'r', ':', ' ', 'F', 'i',
                                'l', 'e', ':', ' ', 'p', 'r', 'e', 's', 'p',
                                '_', 't', 'r', 'a', 'n', 's', 'f', 'o', 'r',
                                'm', ' ', 'L', 'i', 'n', 'e', ':', ' ', '1',
                                ' ', 'C', 'o', 'l', 'u', 'm', 'n', ':', ' ',
                                '1', ' ', 'T', 'h', 'e', ' ', 'f', 'u', 'n',
                                'c', 't', 'i', 'o', 'n', ' ', '"', 'p', 'r',
                                'e', 's', 'p', '_', 't', 'r', 'a', 'n', 's',
                                'f', 'o', 'r', 'm', '"', ' ', 'w', 'a', 's',
                                ' ', 'c', 'a', 'l', 'l', 'e', 'd', ' ', 'w',
                                'i', 't', 'h', ' ', 'm', 'o', 'r', 'e', ' ',
                                't', 'h', 'a', 'n', ' ', 't', 'h', 'e', ' ',
                                'd', 'e', 'c', 'l', 'a', 'r', 'e', 'd', ' ',
                                'n', 'u', 'm', 'b', 'e', 'r', ' ', 'o', 'f',
                                ' ', 'o', 'u', 't', 'p', 'u', 't', 's', ' ',
                                '(', '1', ')', '.' };
static mwArray _mxarray0_ = mclInitializeString(148, _array1_);

static mxChar _array3_[147] = { 'R', 'u', 'n', '-', 't', 'i', 'm', 'e', ' ',
                                'E', 'r', 'r', 'o', 'r', ':', ' ', 'F', 'i',
                                'l', 'e', ':', ' ', 'p', 'r', 'e', 's', 'p',
                                '_', 't', 'r', 'a', 'n', 's', 'f', 'o', 'r',
                                'm', ' ', 'L', 'i', 'n', 'e', ':', ' ', '1',
                                ' ', 'C', 'o', 'l', 'u', 'm', 'n', ':', ' ',
                                '1', ' ', 'T', 'h', 'e', ' ', 'f', 'u', 'n',
                                'c', 't', 'i', 'o', 'n', ' ', '"', 'p', 'r',
                                'e', 's', 'p', '_', 't', 'r', 'a', 'n', 's',
                                'f', 'o', 'r', 'm', '"', ' ', 'w', 'a', 's',
                                ' ', 'c', 'a', 'l', 'l', 'e', 'd', ' ', 'w',
                                'i', 't', 'h', ' ', 'm', 'o', 'r', 'e', ' ',
                                't', 'h', 'a', 'n', ' ', 't', 'h', 'e', ' ',
                                'd', 'e', 'c', 'l', 'a', 'r', 'e', 'd', ' ',
                                'n', 'u', 'm', 'b', 'e', 'r', ' ', 'o', 'f',
                                ' ', 'i', 'n', 'p', 'u', 't', 's', ' ', '(',
                                '4', ')', '.' };
static mwArray _mxarray2_ = mclInitializeString(147, _array3_);
static mwArray _mxarray4_ = mclInitializeDoubleVector(0, 0, (double *)NULL);
static mwArray _mxarray5_ = mclInitializeDouble(1.0);
static mwArray _mxarray6_ = mclInitializeDouble(0.0);
static mwArray _mxarray7_ = mclInitializeDouble(2.0);
static mwArray _mxarray8_ = mclInitializeDouble(3.0);
static mwArray _mxarray9_ = mclInitializeDouble(4.0);
static mwArray _mxarray10_ = mclInitializeDouble(5.0);
static mwArray _mxarray11_ = mclInitializeDouble(6.0);
static mwArray _mxarray12_ = mclInitializeDouble(7.0);
static mwArray _mxarray13_ = mclInitializeDouble(8.0);

void InitializeModule_presp_transform() {
}

void TerminateModule_presp_transform() {
}

static mwArray Mpresp_transform(int nargout_,
                                mwArray Coorb,
                                mwArray Coora,
                                mwArray B,
                                mwArray pointnumber);

_mexLocalFunctionTable _local_function_table_presp_transform
  = { 0, (mexFunctionTableEntry *)NULL };

//
// The function "presp_transform" contains the normal interface for the
// "presp_transform" M-function from file
// "C:\Liwh\CUNY\Projects\Data_hao\test\presp_transform.m" (lines 1-30). This
// function processes any input arguments and passes them to the implementation
// version of the function, appearing above.
//
mwArray presp_transform(mwArray Coorb,
                        mwArray Coora,
                        mwArray B,
                        mwArray pointnumber) {
    int nargout(1);
    mwArray R(mclGetUninitializedArray());
    R = Mpresp_transform(nargout, Coorb, Coora, B, pointnumber);
    return R;
}

//
// The function "mlxPresp_transform" contains the feval interface for the
// "presp_transform" M-function from file
// "C:\Liwh\CUNY\Projects\Data_hao\test\presp_transform.m" (lines 1-30). The
// feval function calls the implementation version of presp_transform through
// this function. This function processes any input arguments and passes them
// to the implementation version of the function, appearing above.
//
void mlxPresp_transform(int nlhs,
                        mxArray * plhs[],
                        int nrhs,
                        mxArray * prhs[]) {
    MW_BEGIN_MLX();
    {
        mwArray mprhs[4];
        mwArray mplhs[1];
        int i;
        mclCppUninitializeArrays(1, mplhs);
        if (nlhs > 1) {
            error(_mxarray0_);
        }
        if (nrhs > 4) {
            error(_mxarray2_);
        }
        for (i = 0; i < 4 && i < nrhs; ++i) {
            mprhs[i] = mwArray(prhs[i], 0);
        }
        for (; i < 4; ++i) {
            mprhs[i].MakeDIN();
        }
        mplhs[0]
          = Mpresp_transform(nlhs, mprhs[0], mprhs[1], mprhs[2], mprhs[3]);
        plhs[0] = mplhs[0].FreezeData();
    }
    MW_END_MLX();
}

//
// The function "Mpresp_transform" is the implementation version of the
// "presp_transform" M-function from file
// "C:\Liwh\CUNY\Projects\Data_hao\test\presp_transform.m" (lines 1-30). It
// contains the actual compiled code for that M-function. It is a static
// function and must only be called from one of the interface functions,
// appearing below.
//
//
// function R = presp_transform(Coorb, Coora, B, pointnumber)
//
static mwArray Mpresp_transform(int nargout_,
                                mwArray Coorb,
                                mwArray Coora,
                                mwArray B,
                                mwArray pointnumber) {
    mwLocalFunctionTable save_local_function_table_
      (&_local_function_table_presp_transform);
    mwArray R(mclGetUninitializedArray());
    mwArray V(mclGetUninitializedArray());
    mwArray S(mclGetUninitializedArray());
    mwArray U(mclGetUninitializedArray());
    mwArray A(mclGetUninitializedArray());
    mwArray AA(mclGetUninitializedArray());
    mwArray cnt(mclGetUninitializedArray());
    //
    // 
    // 
    // for cnt=1 : pointnumber,
    //
    int v_(mclForIntStart(1));
    int e_(mclForIntEnd(mwVa(pointnumber, "pointnumber")));
    if (v_ > e_) {
        cnt = _mxarray4_;
    } else {
        //
        // AA(cnt, 1) = Coora(cnt, 1) ;
        // AA(cnt, 2) = Coora(cnt, 2) ;
        // AA(cnt, 3) = 1;
        // AA(cnt, 4) = 0;
        // AA(cnt, 5) = 0;
        // AA(cnt, 6) = 0;
        // AA(cnt, 7) = -(Coora(cnt, 1))*(Coorb(cnt,1));
        // AA(cnt, 8) = -(Coora(cnt, 2))*(Coorb(cnt,1));
        // end;
        //
        for (; ; ) {
            mclIntArrayAssign(
              &AA, mclIntArrayRef(mwVsa(Coora, "Coora"), v_, 1), v_, 1);
            mclIntArrayAssign(
              &AA, mclIntArrayRef(mwVsa(Coora, "Coora"), v_, 2), v_, 2);
            mclIntArrayAssign(&AA, _mxarray5_, v_, 3);
            mclIntArrayAssign(&AA, _mxarray6_, v_, 4);
            mclIntArrayAssign(&AA, _mxarray6_, v_, 5);
            mclIntArrayAssign(&AA, _mxarray6_, v_, 6);
            mclIntArrayAssign(
              &AA,
              - mwVe(mclIntArrayRef(mwVsa(Coora, "Coora"), v_, 1))
              * mwVe(mclIntArrayRef(mwVsa(Coorb, "Coorb"), v_, 1)),
              v_,
              7);
            mclIntArrayAssign(
              &AA,
              - mwVe(mclIntArrayRef(mwVsa(Coora, "Coora"), v_, 2))
              * mwVe(mclIntArrayRef(mwVsa(Coorb, "Coorb"), v_, 1)),
              v_,
              8);
            if (v_ == e_) {
                break;
            }
            ++v_;
        }
        cnt = v_;
    }
    //
    // 
    // for cnt= pointnumber+1 : 2*pointnumber,
    //
    {
        mwForLoopIterator viter__;
        for (viter__.Start(
               mwVa(pointnumber, "pointnumber") + _mxarray5_,
               _mxarray7_ * mwVa(pointnumber, "pointnumber"),
               mwArray::DIN);
             viter__.Next(&cnt);
             ) {
            //
            // AA(cnt, 1) = 0;
            //
            mclArrayAssign(&AA, _mxarray6_, mwVsv(cnt, "cnt"), _mxarray5_);
            //
            // AA(cnt, 2) = 0;
            //
            mclArrayAssign(&AA, _mxarray6_, mwVsv(cnt, "cnt"), _mxarray7_);
            //
            // AA(cnt, 3) = 0;
            //
            mclArrayAssign(&AA, _mxarray6_, mwVsv(cnt, "cnt"), _mxarray8_);
            //
            // AA(cnt, 4) = Coora(cnt-pointnumber, 1) ;
            //
            mclArrayAssign(
              &AA,
              mclArrayRef(
                mwVsa(Coora, "Coora"),
                mwVv(cnt, "cnt") - mwVa(pointnumber, "pointnumber"),
                _mxarray5_),
              mwVsv(cnt, "cnt"),
              _mxarray9_);
            //
            // AA(cnt, 5) = Coora(cnt-pointnumber, 2) ;
            //
            mclArrayAssign(
              &AA,
              mclArrayRef(
                mwVsa(Coora, "Coora"),
                mwVv(cnt, "cnt") - mwVa(pointnumber, "pointnumber"),
                _mxarray7_),
              mwVsv(cnt, "cnt"),
              _mxarray10_);
            //
            // AA(cnt, 6) = 1;
            //
            mclArrayAssign(&AA, _mxarray5_, mwVsv(cnt, "cnt"), _mxarray11_);
            //
            // AA(cnt, 7) = -(Coora(cnt-pointnumber, 1))*(Coorb(cnt-pointnumber, 2));
            //
            mclArrayAssign(
              &AA,
              - mwVe(
                  mclArrayRef(
                    mwVsa(Coora, "Coora"),
                    mwVv(cnt, "cnt") - mwVa(pointnumber, "pointnumber"),
                    _mxarray5_))
              * mwVe(
                  mclArrayRef(
                    mwVsa(Coorb, "Coorb"),
                    mwVv(cnt, "cnt") - mwVa(pointnumber, "pointnumber"),
                    _mxarray7_)),
              mwVsv(cnt, "cnt"),
              _mxarray12_);
            //
            // AA(cnt, 8) = -(Coora(cnt-pointnumber, 2))*(Coorb(cnt-pointnumber, 2));
            //
            mclArrayAssign(
              &AA,
              - mwVe(
                  mclArrayRef(
                    mwVsa(Coora, "Coora"),
                    mwVv(cnt, "cnt") - mwVa(pointnumber, "pointnumber"),
                    _mxarray7_))
              * mwVe(
                  mclArrayRef(
                    mwVsa(Coorb, "Coorb"),
                    mwVv(cnt, "cnt") - mwVa(pointnumber, "pointnumber"),
                    _mxarray7_)),
              mwVsv(cnt, "cnt"),
              _mxarray13_);
        //
        // end;
        //
        }
    }
    //
    // 
    // R = ones(8,1);
    //
    R = ones(mwVarargin(_mxarray13_, _mxarray5_));
    //
    // A = AA' * AA;
    //
    A = ctranspose(mwVv(AA, "AA")) * mwVv(AA, "AA");
    //
    // [U,S,V] = svd(A);
    //
    U = svd(&S, &V, mwVv(A, "A"));
    //
    // R = V* inv (S) * U' * AA' * B;
    //
    R
      = mwVv(V, "V") * mwVe(inv(mwVv(S, "S"))) * ctranspose(mwVv(U, "U"))
        * ctranspose(mwVv(AA, "AA"))
        * mwVa(B, "B");
    mwValidateOutput(R, 1, nargout_, "R", "presp_transform");
    return R;
}
