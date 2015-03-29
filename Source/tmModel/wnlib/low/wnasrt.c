/*************************************************************************

wn_set_assert_print(passert_print)

wn_assert(cond)
wn_assert_notreached()
wn_assert_warn(cond)
wn_assert_warn_notreached()

*************************************************************************/
/****************************************************************************

COPYRIGHT NOTICE:

  The source code in this file is provided free of charge
  to the author's consulting clients.  It is in the
  public domain and therefore may be used by anybody for
  any purpose.

AUTHOR:

  Will Naylor, Bill Chapman

****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "wnlib.h"
#include "wnasrt.h"



local bool initialized = FALSE;

#ifdef __cplusplus
  extern "C" {
    local void (*psaved_assert_print)(const char string[]);
    local void (*psaved_assert_crash)(void);
  }
#else /* plain C */
  local void (*psaved_assert_print)(const char string[]);
  local void (*psaved_assert_crash)(void);
#endif


void wn_default_assert_print(const char string[])
{
  fprintf(stderr,"%s",string);
}


void wn_default_assert_crash(void)
{
  abort();
}


local void print_user_message(void)
{
  (*psaved_assert_print)(
      "------------------------------------------------------------\n");
  (*psaved_assert_print)(
      "                       HELP!!!\n");
  (*psaved_assert_print)(
      "This program has encountered a severe internal error.\n");
  (*psaved_assert_print)(
      "Please report the following message to the\n");
  (*psaved_assert_print)(
      "program's developers so that they can fix the problem,\n");
  (*psaved_assert_print)(
      "an override may be available:\n");
  (*psaved_assert_print)(
      "------------------------------------------------------------\n");
}


local void initialize_assert(void)
{
  if(!(initialized))
  {
    psaved_assert_print = &wn_default_assert_print;
    psaved_assert_crash = &wn_default_assert_crash;

    initialized = TRUE;
  }
}


EXTERN void wn_set_assert_print
(
  void (*passert_print)(const char string[])
)
{
  initialize_assert();

  psaved_assert_print = passert_print;
}


EXTERN void wn_set_assert_crash
(
  void (*passert_crash)(void)
)
{
  initialize_assert();

  psaved_assert_crash = passert_crash;
}


void wn_get_assert_override_string(char *output_buffer,
/**/          const char *file_name, int line_num)
{
  unsigned result;
  char *pc;
  int i, j;

  result = 0;
  for (i = j = 0;  file_name[i];  ++i, j = (j+1) % 4)
  {
    ((char *) &result)[j] += file_name[i];
  }

  result += line_num * line_num * line_num * line_num * line_num * line_num;

  sprintf(output_buffer, "WN_ASSERT_SUPPRESS_%s_%d_%x", file_name, line_num,
  /**/                result);
  for (pc = output_buffer;  *pc;  ++pc)
  {
    if ('.' == *pc)
    {
      *pc = '_';
    }
  }
} /* wn_get_assert_override_string */


/* Note that both func_name and/or exp_string might be NULL */

/*[RJL] added prototype */
void wn_general_assert_routine(const char *file_name, int line_num,
/**/  const char *func_name, const char *exp_string, bool warn_only,
/**/            bool notreached);
/*[\RJL] */
void wn_general_assert_routine(const char *file_name, int line_num,
/**/  const char *func_name, const char *exp_string, bool warn_only,
/**/            bool notreached)
{
  char string[1000];
  char override_string[1000];
  char *pc;

  /* if overridden, be absolutely silent, whether warning or fatal */
  wn_get_assert_override_string(override_string, file_name, line_num);
  if (getenv(override_string))
  {
    static bool first_assert_suppressed = TRUE;
    if (first_assert_suppressed)
    {
      (*psaved_assert_print)(
      "Warning: at least one assert suppressed, by environment variable\n");
      (*psaved_assert_print)("    \"");
      (*psaved_assert_print)(override_string);
      (*psaved_assert_print)("\"\n");
      first_assert_suppressed = FALSE;
    }

    return;
  }

  initialize_assert();

  print_user_message();

  if (notreached)
  {
    /* There is no expression, we just reached a notreached */

    (void)sprintf(string,
    /**/  "wn_assert%s_notreached() called -- %sforcing crash\n",
    /**/    warn_only ? "_warn" : "", warn_only ? "NOT " : "");
    (*psaved_assert_print)(string);
  }
  else
  {
    (void)sprintf(string, "Assertion botched -- %sforcing crash\n",
    /**/          warn_only ? "NOT " : "");
    (*psaved_assert_print)(string);

    /*     we want the expression on a separate line because it could be
    ** very long */
    if (exp_string)
    {
      (void) sprintf(string, "Expr (%s) was false\n", exp_string);
      (*psaved_assert_print)(string);
    }
  }

  pc = string;
  if (func_name)
  {
    (void) sprintf(string, "%cn %s() ", exp_string ? 'i' : 'I', func_name);
    for (  ;  *pc;  ++pc) ;
  }
  (void) sprintf(pc, "%ct line %d in file %s\n", func_name ? 'a' : 'A',
  /**/              line_num, file_name);
  (*psaved_assert_print)(string);

  if (!warn_only)
  {
    (*psaved_assert_crash)();
  }
} /* wn_general_assert_routine */


void wn_assert_routine(const char file_name[],int line_num)
{
  wn_general_assert_routine(file_name, line_num, NULL, NULL, FALSE, FALSE);
} /* wn_assert_routine */


void wn_assert_notreached_routine(const char *file_name, int line_num)
{
  wn_general_assert_routine(file_name, line_num, NULL, NULL, FALSE, TRUE);
}


void wn_assert_warn_routine(const char file_name[],int line_num)
{
  wn_general_assert_routine(file_name, line_num, NULL, NULL, TRUE, FALSE);
}


void wn_assert_warn_notreached_r(const char file_name[],int line_num)
{
  wn_general_assert_routine(file_name, line_num, NULL, NULL, TRUE, TRUE);
}


/* **************************************************************** */

void wn_assert_routine_func_exp(const char file_name[], int line_num,
/**/      const char *func_name, const char *exp_string)
{
  wn_general_assert_routine(file_name, line_num, func_name, exp_string,
  /**/                FALSE, FALSE);
} /* wn_assert_routine_func_exp */


void wn_assert_notreached_routine_func(const char *file_name, int line_num,
/**/            const char *func_name)
{
  wn_general_assert_routine(file_name, line_num, func_name, NULL,
  /**/                FALSE, TRUE);
}


void wn_assert_warn_routine_func_exp(const char file_name[], int line_num,
/**/      const char *func_name, const char *exp_string)
{
  wn_general_assert_routine(file_name, line_num, func_name, exp_string,
  /**/                TRUE, FALSE);
}


void wn_assert_warn_notreached_r_func(const char file_name[],int line_num,
/**/            const char *func_name)
{
  wn_general_assert_routine(file_name, line_num, func_name, NULL,
  /**/                TRUE, TRUE);
}
