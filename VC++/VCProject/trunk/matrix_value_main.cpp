//
// MATLAB Compiler: 2.2
// Date: Sat Feb 14 11:50:39 2004
// Arguments: "-B" "macro_default" "-O" "all" "-O" "fold_scalar_mxarrays:on"
// "-O" "fold_non_scalar_mxarrays:on" "-O" "optimize_integer_for_loops:on" "-O"
// "array_indexing:on" "-O" "optimize_conditionals:on" "-p" "-W" "main" "-L"
// "Cpp" "-t" "-T" "link:exe" "-h" "libmmfile.mlib" "matrix_value" 
//
#include "libmatlb.hpp"
#include "matrix_value.hpp"

static mexFunctionTableEntry function_table[1]
  = { { "matrix_value", mlxMatrix_value, 0, 1,
        &_local_function_table_matrix_value } };

static _mexcppInitTermTableEntry init_term_table[1]
  = { { InitializeModule_matrix_value, TerminateModule_matrix_value } };

static _mexcpp_information _main_info
  = { 1, 1, function_table, 0, NULL, 0, NULL, 1, init_term_table };

//
// The function "main" is a Compiler-generated main wrapper, suitable for
// building a stand-alone application.  It calls a library function to perform
// initialization, call the main function, and perform library termination.
//
int main(int argc, const char * * argv) {
    return mwMain(argc, argv, mlxMatrix_value, 1, &_main_info);
}
