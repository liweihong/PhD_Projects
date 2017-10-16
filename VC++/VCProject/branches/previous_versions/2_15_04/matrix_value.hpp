//
// MATLAB Compiler: 2.2
// Date: Sat Feb 14 22:22:52 2004
// Arguments: "-B" "macro_default" "-O" "all" "-O" "fold_scalar_mxarrays:on"
// "-O" "fold_non_scalar_mxarrays:on" "-O" "optimize_integer_for_loops:on" "-O"
// "array_indexing:on" "-O" "optimize_conditionals:on" "-t" "-L" "Cpp"
// "matrix_value" 
//
#ifndef __matrix_value_hpp
#define __matrix_value_hpp 1

#include "libmatlb.hpp"

extern void InitializeModule_matrix_value();
extern void TerminateModule_matrix_value();
extern _mexLocalFunctionTable _local_function_table_matrix_value;

extern mwArray matrix_value(mwArray * R_inv,
                            mwArray dest_value = mwArray::DIN,
                            mwArray mimio_value = mwArray::DIN,
                            mwArray num = mwArray::DIN);
#ifdef __cplusplus
extern "C"
#endif
void mlxMatrix_value(int nlhs, mxArray * plhs[], int nrhs, mxArray * prhs[]);

#endif
