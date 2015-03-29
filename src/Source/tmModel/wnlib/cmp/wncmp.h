/****************************************************************************

COPYRIGHT NOTICE:

  The source code in this file is provided free of charge
  to the author's consulting clients.  It is in the
  public domain and therefore may be used by anybody for
  any purpose.

AUTHOR:

  Will Naylor

****************************************************************************/
#ifndef wncmpH
#define wncmpH



#include "wnlib.h"



EXTERN int wn_intcmp(int i1,int i2);
EXTERN int wn_longcmp(long i1,long i2);
EXTERN int wn_ptrcmp(ptr p1,ptr p2);
EXTERN int wn_ptrNULLcmp(bool *psuccess,ptr p1,ptr p2);
EXTERN int wn_boolcmp(bool b1,bool b2);
EXTERN int wn_doublecmp(double d1,double d2);
EXTERN int wn_pdoublecmp(double *pd1,double *pd2);
EXTERN int wn_numstrcmp(char *s1,char *s2);
EXTERN int wn_memcmp(ptr m1,ptr m2,int len);



#endif
