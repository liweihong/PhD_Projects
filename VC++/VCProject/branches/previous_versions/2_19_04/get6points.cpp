//
// MATLAB Compiler: 2.2
// Date: Sun Feb 15 18:11:57 2004
// Arguments: "-B" "macro_default" "-O" "all" "-O" "fold_scalar_mxarrays:on"
// "-O" "fold_non_scalar_mxarrays:on" "-O" "optimize_integer_for_loops:on" "-O"
// "array_indexing:on" "-O" "optimize_conditionals:on" "-t" "-L" "Cpp"
// "get6points" 
//
#include "get6points.hpp"
#include "libmatlbm.hpp"

static mxChar _array1_[138] = { 'R', 'u', 'n', '-', 't', 'i', 'm', 'e', ' ',
                                'E', 'r', 'r', 'o', 'r', ':', ' ', 'F', 'i',
                                'l', 'e', ':', ' ', 'g', 'e', 't', '6', 'p',
                                'o', 'i', 'n', 't', 's', ' ', 'L', 'i', 'n',
                                'e', ':', ' ', '1', ' ', 'C', 'o', 'l', 'u',
                                'm', 'n', ':', ' ', '1', ' ', 'T', 'h', 'e',
                                ' ', 'f', 'u', 'n', 'c', 't', 'i', 'o', 'n',
                                ' ', '"', 'g', 'e', 't', '6', 'p', 'o', 'i',
                                'n', 't', 's', '"', ' ', 'w', 'a', 's', ' ',
                                'c', 'a', 'l', 'l', 'e', 'd', ' ', 'w', 'i',
                                't', 'h', ' ', 'm', 'o', 'r', 'e', ' ', 't',
                                'h', 'a', 'n', ' ', 't', 'h', 'e', ' ', 'd',
                                'e', 'c', 'l', 'a', 'r', 'e', 'd', ' ', 'n',
                                'u', 'm', 'b', 'e', 'r', ' ', 'o', 'f', ' ',
                                'o', 'u', 't', 'p', 'u', 't', 's', ' ', '(',
                                '1', ')', '.' };
static mwArray _mxarray0_ = mclInitializeString(138, _array1_);

static mxChar _array3_[137] = { 'R', 'u', 'n', '-', 't', 'i', 'm', 'e', ' ',
                                'E', 'r', 'r', 'o', 'r', ':', ' ', 'F', 'i',
                                'l', 'e', ':', ' ', 'g', 'e', 't', '6', 'p',
                                'o', 'i', 'n', 't', 's', ' ', 'L', 'i', 'n',
                                'e', ':', ' ', '1', ' ', 'C', 'o', 'l', 'u',
                                'm', 'n', ':', ' ', '1', ' ', 'T', 'h', 'e',
                                ' ', 'f', 'u', 'n', 'c', 't', 'i', 'o', 'n',
                                ' ', '"', 'g', 'e', 't', '6', 'p', 'o', 'i',
                                'n', 't', 's', '"', ' ', 'w', 'a', 's', ' ',
                                'c', 'a', 'l', 'l', 'e', 'd', ' ', 'w', 'i',
                                't', 'h', ' ', 'm', 'o', 'r', 'e', ' ', 't',
                                'h', 'a', 'n', ' ', 't', 'h', 'e', ' ', 'd',
                                'e', 'c', 'l', 'a', 'r', 'e', 'd', ' ', 'n',
                                'u', 'm', 'b', 'e', 'r', ' ', 'o', 'f', ' ',
                                'i', 'n', 'p', 'u', 't', 's', ' ', '(', '4',
                                ')', '.' };
static mwArray _mxarray2_ = mclInitializeString(137, _array3_);
static mwArray _mxarray4_ = mclInitializeDouble(3.1415926);
static mwArray _mxarray5_ = mclInitializeDouble(180.0);
static mwArray _mxarray6_ = mclInitializeDouble(2.0);
static mwArray _mxarray7_ = mclInitializeDouble(1.0);
static mwArray _mxarray8_ = mclInitializeDouble(3.0);
static mwArray _mxarray9_ = mclInitializeDouble(4.0);
static mwArray _mxarray10_ = mclInitializeDouble(5.0);
static mwArray _mxarray11_ = mclInitializeDouble(6.0);

void InitializeModule_get6points() {
}

void TerminateModule_get6points() {
}

static mwArray Mget6points(int nargout_,
                           mwArray angle,
                           mwArray dis,
                           mwArray w,
                           mwArray h);

_mexLocalFunctionTable _local_function_table_get6points
  = { 0, (mexFunctionTableEntry *)NULL };

//
// The function "get6points" contains the normal interface for the "get6points"
// M-function from file "C:\Liwh\CUNY\Projects\Data_hao\test\get6points.m"
// (lines 1-24). This function processes any input arguments and passes them to
// the implementation version of the function, appearing above.
//
mwArray get6points(mwArray angle, mwArray dis, mwArray w, mwArray h) {
    int nargout(1);
    mwArray coord(mclGetUninitializedArray());
    coord = Mget6points(nargout, angle, dis, w, h);
    return coord;
}

//
// The function "mlxGet6points" contains the feval interface for the
// "get6points" M-function from file
// "C:\Liwh\CUNY\Projects\Data_hao\test\get6points.m" (lines 1-24). The feval
// function calls the implementation version of get6points through this
// function. This function processes any input arguments and passes them to the
// implementation version of the function, appearing above.
//
void mlxGet6points(int nlhs, mxArray * plhs[], int nrhs, mxArray * prhs[]) {
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
        mplhs[0] = Mget6points(nlhs, mprhs[0], mprhs[1], mprhs[2], mprhs[3]);
        plhs[0] = mplhs[0].FreezeData();
    }
    MW_END_MLX();
}

//
// The function "Mget6points" is the implementation version of the "get6points"
// M-function from file "C:\Liwh\CUNY\Projects\Data_hao\test\get6points.m"
// (lines 1-24). It contains the actual compiled code for that M-function. It
// is a static function and must only be called from one of the interface
// functions, appearing below.
//
//
// function coord = get6points(angle, dis, w, h)
//
static mwArray Mget6points(int nargout_,
                           mwArray angle,
                           mwArray dis,
                           mwArray w,
                           mwArray h) {
    mwLocalFunctionTable save_local_function_table_
      (&_local_function_table_get6points);
    mwArray coord(mclGetUninitializedArray());
    mwArray distance(mclGetUninitializedArray());
    mwArray diag(mclGetUninitializedArray());
    mwArray m6(mclGetUninitializedArray());
    mwArray m5(mclGetUninitializedArray());
    mwArray m4(mclGetUninitializedArray());
    mwArray m3(mclGetUninitializedArray());
    mwArray m2(mclGetUninitializedArray());
    mwArray m1(mclGetUninitializedArray());
    mwArray ang4(mclGetUninitializedArray());
    mwArray ang3(mclGetUninitializedArray());
    mwArray ang2(mclGetUninitializedArray());
    mwArray ang1(mclGetUninitializedArray());
    mwArray pi(mclGetUninitializedArray());
    //
    // 
    // pi=3.1415926;
    //
    pi = _mxarray4_;
    //
    // ang1 = angle(1)*pi/180;
    //
    ang1
      = mwVe(mclIntArrayRef(mwVsa(angle, "angle"), 1)) * mwVv(pi, "pi")
        / _mxarray5_;
    //
    // ang2 = angle(2)*pi/180;
    //
    ang2
      = mwVe(mclIntArrayRef(mwVsa(angle, "angle"), 2)) * mwVv(pi, "pi")
        / _mxarray5_;
    //
    // ang3 = angle(3)*pi/180;
    //
    ang3
      = mwVe(mclIntArrayRef(mwVsa(angle, "angle"), 3)) * mwVv(pi, "pi")
        / _mxarray5_;
    //
    // ang4 = angle(4)*pi/180;
    //
    ang4
      = mwVe(mclIntArrayRef(mwVsa(angle, "angle"), 4)) * mwVv(pi, "pi")
        / _mxarray5_;
    //
    // m1 = [cos(ang1) sin(ang1); cos(ang2) sin(ang2)];
    //
    m1
      = vertcat(
          mwVarargin(
            horzcat(
              mwVarargin(
                mwVe(cos(mwVv(ang1, "ang1"))), mwVe(sin(mwVv(ang1, "ang1"))))),
            horzcat(
              mwVarargin(
                mwVe(cos(mwVv(ang2, "ang2"))),
                mwVe(sin(mwVv(ang2, "ang2")))))));
    //
    // m2 = [cos(ang2) sin(ang2); cos(ang3) sin(ang3)];
    //
    m2
      = vertcat(
          mwVarargin(
            horzcat(
              mwVarargin(
                mwVe(cos(mwVv(ang2, "ang2"))), mwVe(sin(mwVv(ang2, "ang2"))))),
            horzcat(
              mwVarargin(
                mwVe(cos(mwVv(ang3, "ang3"))),
                mwVe(sin(mwVv(ang3, "ang3")))))));
    //
    // m3 = [cos(ang3) sin(ang3); cos(ang4) sin(ang4)];
    //
    m3
      = vertcat(
          mwVarargin(
            horzcat(
              mwVarargin(
                mwVe(cos(mwVv(ang3, "ang3"))), mwVe(sin(mwVv(ang3, "ang3"))))),
            horzcat(
              mwVarargin(
                mwVe(cos(mwVv(ang4, "ang4"))),
                mwVe(sin(mwVv(ang4, "ang4")))))));
    //
    // m4 = [cos(ang4) sin(ang4); cos(ang1) sin(ang1)];
    //
    m4
      = vertcat(
          mwVarargin(
            horzcat(
              mwVarargin(
                mwVe(cos(mwVv(ang4, "ang4"))), mwVe(sin(mwVv(ang4, "ang4"))))),
            horzcat(
              mwVarargin(
                mwVe(cos(mwVv(ang1, "ang1"))),
                mwVe(sin(mwVv(ang1, "ang1")))))));
    //
    // m5 = [cos(ang1) sin(ang1); cos(ang3) sin(ang3)];
    //
    m5
      = vertcat(
          mwVarargin(
            horzcat(
              mwVarargin(
                mwVe(cos(mwVv(ang1, "ang1"))), mwVe(sin(mwVv(ang1, "ang1"))))),
            horzcat(
              mwVarargin(
                mwVe(cos(mwVv(ang3, "ang3"))),
                mwVe(sin(mwVv(ang3, "ang3")))))));
    //
    // m6 = [cos(ang2) sin(ang2); cos(ang4) sin(ang4)];
    //
    m6
      = vertcat(
          mwVarargin(
            horzcat(
              mwVarargin(
                mwVe(cos(mwVv(ang2, "ang2"))), mwVe(sin(mwVv(ang2, "ang2"))))),
            horzcat(
              mwVarargin(
                mwVe(cos(mwVv(ang4, "ang4"))),
                mwVe(sin(mwVv(ang4, "ang4")))))));
    //
    // diag = sqrt(w*w+h*h);
    //
    diag = sqrt(mwVa(w, "w") * mwVa(w, "w") + mwVa(h, "h") * mwVa(h, "h"));
    //
    // distance = dis - diag/2;
    //
    distance = mwVa(dis, "dis") - mwVv(diag, "diag") / _mxarray6_;
    //
    // coord(1,:) = (inv(m1)*[distance(1); distance(2)])';
    //
    mclArrayAssign(
      &coord,
      ctranspose(
        mwVe(inv(mwVv(m1, "m1")))
        * vertcat(
            mwVarargin(
              mwVe(mclIntArrayRef(mwVsv(distance, "distance"), 1)),
              mwVe(mclIntArrayRef(mwVsv(distance, "distance"), 2))))),
      _mxarray7_,
      colon());
    //
    // coord(2,:) = (inv(m2)*[distance(2); distance(3)])';
    //
    mclArrayAssign(
      &coord,
      ctranspose(
        mwVe(inv(mwVv(m2, "m2")))
        * vertcat(
            mwVarargin(
              mwVe(mclIntArrayRef(mwVsv(distance, "distance"), 2)),
              mwVe(mclIntArrayRef(mwVsv(distance, "distance"), 3))))),
      _mxarray6_,
      colon());
    //
    // coord(3,:) = (inv(m3)*[distance(3); distance(4)])';
    //
    mclArrayAssign(
      &coord,
      ctranspose(
        mwVe(inv(mwVv(m3, "m3")))
        * vertcat(
            mwVarargin(
              mwVe(mclIntArrayRef(mwVsv(distance, "distance"), 3)),
              mwVe(mclIntArrayRef(mwVsv(distance, "distance"), 4))))),
      _mxarray8_,
      colon());
    //
    // coord(4,:) = (inv(m4)*[distance(4); distance(1)])';
    //
    mclArrayAssign(
      &coord,
      ctranspose(
        mwVe(inv(mwVv(m4, "m4")))
        * vertcat(
            mwVarargin(
              mwVe(mclIntArrayRef(mwVsv(distance, "distance"), 4)),
              mwVe(mclIntArrayRef(mwVsv(distance, "distance"), 1))))),
      _mxarray9_,
      colon());
    //
    // coord(5,:) = (inv(m5)*[distance(1); distance(3)])';
    //
    mclArrayAssign(
      &coord,
      ctranspose(
        mwVe(inv(mwVv(m5, "m5")))
        * vertcat(
            mwVarargin(
              mwVe(mclIntArrayRef(mwVsv(distance, "distance"), 1)),
              mwVe(mclIntArrayRef(mwVsv(distance, "distance"), 3))))),
      _mxarray10_,
      colon());
    //
    // coord(6,:) = (inv(m6)*[distance(2); distance(4)])';
    //
    mclArrayAssign(
      &coord,
      ctranspose(
        mwVe(inv(mwVv(m6, "m6")))
        * vertcat(
            mwVarargin(
              mwVe(mclIntArrayRef(mwVsv(distance, "distance"), 2)),
              mwVe(mclIntArrayRef(mwVsv(distance, "distance"), 4))))),
      _mxarray11_,
      colon());
    //
    // coord(:,1) = coord(:,1) + w/2;
    //
    mclArrayAssign(
      &coord,
      mwVe(mclArrayRef(mwVsv(coord, "coord"), colon(), _mxarray7_))
      + mwVa(w, "w") / _mxarray6_,
      colon(),
      _mxarray7_);
    //
    // coord(:,2) = coord(:,2) + h/2;
    //
    mclArrayAssign(
      &coord,
      mwVe(mclArrayRef(mwVsv(coord, "coord"), colon(), _mxarray6_))
      + mwVa(h, "h") / _mxarray6_,
      colon(),
      _mxarray6_);
    mwValidateOutput(coord, 1, nargout_, "coord", "get6points");
    return coord;
}
