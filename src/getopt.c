/* Windows is missing getopt(), so we include a version of this function.
 */

#include <string.h>
#include <stdio.h>

#include <dicom/dicom.h>

#include "pdicom.h"

#define BADCH   (int)'#'
#define BADARG  (int)':'
#define EMSG    ""

int dcm_opterr = 1,             /* if error message should be printed */
    dcm_optind = 1,             /* index into parent argv vector */
    dcm_optopt,                 /* character checked for validity */
    dcm_optreset;               /* reset getopt */
char *dcm_optarg;               /* argument associated with option */

int dcm_getopt(int nargc, char * const nargv[], const char *ostr)
{
  static char *place = EMSG;              /* option letter processing */
  const char *oli;                        /* option letter list index */

  if (dcm_optreset || !*place) {              /* update scanning pointer */
    dcm_optreset = 0;
    if (dcm_optind >= nargc || *(place = nargv[dcm_optind]) != '-') {
      place = EMSG;
      return (-1);
    }
    if (place[1] && *++place == '-') {      /* found "--" */
      ++dcm_optind;
      place = EMSG;
      return (-1);
    }
  }                                       /* option letter okay? */
  if ((dcm_optopt = (int)*place++) == (int)':' ||
    !(oli = strchr(ostr, dcm_optopt))) {
      /*
      * if the user didn't specify '-' as an option,
      * assume it means -1.
      */
      if (dcm_optopt == (int)'-')
        return (-1);
      if (!*place)
        ++dcm_optind;
      if (dcm_opterr && *ostr != ':')
        (void)printf("illegal option -- %c\n", dcm_optopt);
      return (BADCH);
  }
  if (*++oli != ':') {                    /* don't need argument */
    dcm_optarg = NULL;
    if (!*place)
      ++dcm_optind;
  }
  else {                                  /* need an argument */
    if (*place)                     /* no white space */
      dcm_optarg = place;
    else if (nargc <= ++dcm_optind) {   /* no arg */
      place = EMSG;
      if (*ostr == ':')
        return (BADARG);
      if (dcm_opterr)
        (void)printf("option requires an argument -- %c\n", dcm_optopt);
      return (BADCH);
    }
    else                            /* white space */
      dcm_optarg = nargv[dcm_optind];
    place = EMSG;
    ++dcm_optind;
  }
  return (dcm_optopt);                        /* dump back option letter */
}

