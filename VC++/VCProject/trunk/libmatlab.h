//
// MATLAB Compiler: 4.3 (R14SP3)
// Date: Tue Apr 18 00:24:12 2006
// Arguments: "-B" "macro_default" "-W" "cpplib:libmatlab" "-T" "link:lib"
// "get6points.m" "presp_transform.m" "matrix_value.m" "-v" 
//

#ifndef __libmatlab_h
#define __libmatlab_h 1

#if defined(__cplusplus) && !defined(mclmcr_h) && defined(__linux__)
#  pragma implementation "mclmcr.h"
#endif
#include "mclmcr.h"
#include "mclcppclass.h"
#ifdef __cplusplus
extern "C" {
#endif

#if defined(__SUNPRO_CC)
/* Solaris shared libraries use __global, rather than mapfiles
 * to define the API exported from a shared library. __global is
 * only necessary when building the library -- files including
 * this header file to use the library do not need the __global
 * declaration; hence the EXPORTING_<library> logic.
 */

#ifdef EXPORTING_libmatlab
#define PUBLIC_libmatlab_C_API __global
#else
#define PUBLIC_libmatlab_C_API /* No import statement needed. */
#endif

#define LIB_libmatlab_C_API PUBLIC_libmatlab_C_API

#elif defined(_HPUX_SOURCE)

#ifdef EXPORTING_libmatlab
#define PUBLIC_libmatlab_C_API __declspec(dllexport)
#else
#define PUBLIC_libmatlab_C_API __declspec(dllimport)
#endif

#define LIB_libmatlab_C_API PUBLIC_libmatlab_C_API


#else

#define LIB_libmatlab_C_API

#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_libmatlab_C_API 
#define LIB_libmatlab_C_API /* No special import/export declaration */
#endif

extern LIB_libmatlab_C_API 
bool libmatlabInitializeWithHandlers(mclOutputHandlerFcn error_handler,
                                     mclOutputHandlerFcn print_handler);

extern LIB_libmatlab_C_API 
bool libmatlabInitialize(void);

extern LIB_libmatlab_C_API 
void libmatlabTerminate(void);


extern LIB_libmatlab_C_API 
void mlxGet6points(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

extern LIB_libmatlab_C_API 
void mlxPresp_transform(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

extern LIB_libmatlab_C_API 
void mlxMatrix_value(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/* On Windows, use __declspec to control the exported API */
#if defined(_MSC_VER) || defined(__BORLANDC__)

#ifdef EXPORTING_libmatlab
#define PUBLIC_libmatlab_CPP_API __declspec(dllexport)
#else
#define PUBLIC_libmatlab_CPP_API __declspec(dllimport)
#endif

#define LIB_libmatlab_CPP_API PUBLIC_libmatlab_CPP_API

#else

#if !defined(LIB_libmatlab_CPP_API)
#if defined(LIB_libmatlab_C_API)
#define LIB_libmatlab_CPP_API LIB_libmatlab_C_API
#else
#define LIB_libmatlab_CPP_API /* empty! */ 
#endif
#endif

#endif

extern LIB_libmatlab_CPP_API void get6points(int nargout, mwArray& coord
                                             , const mwArray& angle
                                             , const mwArray& dis
                                             , const mwArray& w
                                             , const mwArray& h);

extern LIB_libmatlab_CPP_API void presp_transform(int nargout, mwArray& R
                                                  , mwArray& R1
                                                  , const mwArray& Coorb
                                                  , const mwArray& Coora
                                                  , const mwArray& B
                                                  , const mwArray& pointnumber);

extern LIB_libmatlab_CPP_API void matrix_value(int nargout, mwArray& R
                                               , mwArray& R_inv
                                               , const mwArray& dest_value
                                               , const mwArray& mimio_value
                                               , const mwArray& num);

#endif

#endif
