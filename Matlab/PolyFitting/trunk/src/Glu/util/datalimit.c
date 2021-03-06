/**CFile************************************************************************

  FileName    [datalimit.c]

  PackageName [util]

  Synopsis [Routine to obtain the maximum data size available to a program. The
  routine is based on "getrlimit". If the system does not have this function,
  the default value RLIMIT_DATA_DEFAULT is assumed. This function provides an
  informative value, it does not restrict the size of the program in any way.]

  Author      [Fabio Somenzi <fabio@colorado.edu>]

  Copyright   [This file was created at the University of Colorado at
  Boulder.  The University of Colorado at Boulder makes no warranty
  about the suitability of this software for any purpose.  It is
  presented on an AS IS basis.]

******************************************************************************/

#include "util.h"

static char rcsid[] UNUSED = "$Id: datalimit.c,v 1.3 2004/09/29 08:43:24 ilya Exp $";

#if HAVE_SYS_RESOURCE_H
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/resource.h>
#endif

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#ifndef RLIMIT_DATA_DEFAULT
#define RLIMIT_DATA_DEFAULT 67108864	/* assume 64MB by default */
#endif

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Function that computes the data limit of the process.]

  SideEffects        []

******************************************************************************/
int
getSoftDataLimit(void)
{
#if HAVE_SYS_RESOURCE_H && HAVE_GETRLIMIT && defined(RLIMIT_DATA)
    struct rlimit rl;
    int result;

    result = getrlimit(RLIMIT_DATA, &rl);
    /*printf("rlimit result=%d cur=%ld max=%d\n", result, (long)rl.rlim_cur, (long)rl.rlim_max);*/
    if (result != 0 || rl.rlim_cur == RLIM_INFINITY) {
      /*printf("rlimit() failed: returning default datalimit of %ld\n", (long)RLIMIT_DATA_DEFAULT);*/
	return(RLIMIT_DATA_DEFAULT);
    }
    else {
	return(rl.rlim_cur);
    }
#else
    printf("no rlimit(): returning default datalimit of %ld\n", (long)RLIMIT_DATA_DEFAULT);
    return(RLIMIT_DATA_DEFAULT);
#endif

} /* end of getSoftDataLimit */

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
