/********************************************************************
Last modified: 5/28/03 by bogdan
********************************************************************/
#include <stdio.h>
#include <math.h>

#include "wnlib.h"
#include "wnsqr.h"
#include "wnswap.h"
#include "wnmem.h"
#include "wnmemb.h"
#include "wnasrt.h"
#include "wnabs.h"
#include "wnmax.h"
#include "wncmp.h"
#include "wnrndd.h"
#include "wnrnd.h"
#include "wnpoly.h"

#include "wnvect.h"
#include "wnmat.h"

#include "wnparvect.h"

#include "wnconj.h"

#define DEBUGBC       FALSE
#ifndef DEBUG
  #define DEBUG         FALSE
#endif
#define DEBUG_DEPTH   10

#define NUM_BEST_F    10

/*[RJL] need to check existence before redefining */
#ifdef INFINITY
  #undef INFINITY
/*[\RJL]*/
  #define INFINITY              WN_FHUGE
/*[RJL]*/
#endif
/*[\RJL]*/

#define MAX_FAST_LS_MAGNIFICATION  (10.0)
#define MAX_SLOW_LS_MAGNIFICATION  (10.0)
#define MIN_CONJ_SHRINK            (1.0/100.0)

/* max number of 'good' bisections allowed when a minimum has been lost */
#define BISECTION_MAX              2

/* max number of suspicious bisections allowed in a slow line search */
#define MAX_SUSPECT_BISECTIONS     3

/* max number of lost minima allowed in a slow line search */
#define MAX_LOST_MIN               3

/* improvement in gradient sought when a minimum has been lost */
#define MINLOST_GRAD_IMPROVE       .75

/* max |cos| value allowed for the angle of the interpolated 
   gradient with the search direction */
#define MAX_COS_VALUE       .5

/* max increase in step length allowed when starting new line search */
#define MAX_STEP_LENGTH_INCREASE   8.0

extern bool wn_force_optimize_stop_flag;


/* data for slow linesearch */
typedef struct slow_linesearch_type_struct
{
  double x0,x1,x2;   /* x0 < x1 < x2 */
  double f0,f1,f2;   /* f1 <= f0; f1 <= f2, try to bracket min */
  double df0,df1,df2;  /* derivatives */
  bool isreal0,isreal1,isreal2;  /* point has been initialized */
  bool quadconv1;

  int min_lost; /* number of minimums thrown away while shrinking bracketing interval */
  int bisection_count; /* a robust probe counter */
  int suspect_bisection; /* number of suspicious bisections */

  double fstart,dfstart;   /* start point for slow linesearch, use 
            for termination tests */
} slow_linesearch_struct;

typedef struct context_struct
{
  wn_memgp this_group;

  double *vect;  /* from caller */
  int num_vars;
  double (*pfuncgrad)(double grad[],double vect[]);
  bool (*pterminate)(double vect[],double best_f,int iteration);
  double first_step_len;
  int iteration,max_iterations;

  double *vect0,*vect1;
  double *dir0,*dir1;
  double *grad0,*grad1;
  double *grad0s,*grad0opt,*grad1s;
  double *gradls;
  double f0,f1;
  double f0s,f0opt,f1s;
  double fls;
  double best_f,best_f_stack[NUM_BEST_F];
  int iters_since_best_f0,iters_since_best_f1;
  double x0s,x0opt;    /* multipliers by directions to get sample vects */
  double x1s,x1opt;
  double xls;
  double x0s_init;
  bool isreal0,isreal0opt;   /* is real data, not interpolated */
  bool isreal1;
  bool isrealls;
  double norm2_grad0,norm2_grad1,norm2_grad0opt;
  double norm2_dir0,norm2_dir1;
  double dot_grad0opt_dir0,dot_grad0_dir1;
  double dot_grad0_dir0,dot_grad0s_dir0,dot_gradls_dir0;
  double dot_grad1_dir1,dot_grad1s_dir1,dot_gradls_dir1;

  slow_linesearch_struct sls;

#if DEBUG
  double **debug_dir_stack; /* debug_dir_stack[DEBUG_DEPTH] */
  int debug_dir_stack_count;
#endif

  int iters_since_slow_linesearch_needed0,
      iters_since_slow_linesearch_needed1;

  double step_len0,step_len1;

  int no_reset_run_len,reset_at_iteration,iteration_of_last_reset;
  double reset_expand_factor;

  wn_parvect_context pc;
} cntxt_s,*cntxt;  


/* a method that tests if two numbers are close to each other */
local bool close_to_each_other(double a, double b, double margin)
{
  double absa, absb;

  absa = wn_abs(a);
  absb = wn_abs(b);

  if(
     (wn_sign(a) == wn_sign(b))
       &&
     (absa <= (1.0 + margin)*absb)
       &&
     (absb <= (1.0 + margin)*absa)
    )
  {
    return(TRUE);
  }

  return(FALSE);
}


local bool is_valid_number(double x)
{
  return((-WN_FHUGE < x)&&(x < WN_FHUGE));
}


local int qroot_sign(double x)
{
  double rnd;
  int sign;

  if(!(x >= 0))
  {
    sign = -1;
  }
  else if(!(x <= 0))
  {
    sign = 1;
  }
  else
  {
    if(wn_random_bit())
    {
      sign = -1;
    }
    else
    {
      sign = 1;
    }
  }

  return(sign);
}


local int fcmp(double f1,double f2)
{
  f1 = wn_clip_f(f1);
  f2 = wn_clip_f(f2);

  if(f1 > f2)
  {
    return(1);
  }
  else if(f1 < f2)
  {
    return(-1);
  }
  else
  {
    return(0);
  }
}


/*
  lexicographic comparison of function values and slopes.  
  If function values same, then assume smaller absolute value of 
  slope is smaller f
*/
local int fdcmp(double f1,double df1,double f2,double df2)
{
  int cmp;
  double adf1,adf2;

  cmp = fcmp(f1,f2);
  if(cmp != 0)
  {
    return(cmp);
  }

  adf1 = wn_abs(df1);
  adf2 = wn_abs(df2);

  cmp = fcmp(adf1,adf2);

  return(cmp);
}


#if DEBUG
local void push_debug_dir_stack(cntxt c)
{
  int i;
  double *tmp_debug_dir;

  ++(c->debug_dir_stack_count);
  if(c->debug_dir_stack_count > DEBUG_DEPTH)
  {
    c->debug_dir_stack_count = DEBUG_DEPTH;
  }

  tmp_debug_dir = (c->debug_dir_stack)[c->debug_dir_stack_count-1];

  for(i=c->debug_dir_stack_count-1;i>0;--i)
  {
    (c->debug_dir_stack)[i] = (c->debug_dir_stack)[i-1];
  }

  (c->debug_dir_stack)[0] = tmp_debug_dir;
  wn_copy_vect_par(c->pc,(c->debug_dir_stack)[0],c->dir0,c->num_vars);
}


local void pop_debug_dir_stack(cntxt c)
{
  int i;
  double *tmp_debug_dir;

  --(c->debug_dir_stack_count);

  tmp_debug_dir = (c->debug_dir_stack)[0];

  for(i=0;i<c->debug_dir_stack_count;++i)
  {
    (c->debug_dir_stack)[i] = (c->debug_dir_stack)[i+1];
  }

  (c->debug_dir_stack)[c->debug_dir_stack_count] = tmp_debug_dir;
  wn_zero_vect_par(c->pc,tmp_debug_dir,c->num_vars);
}


local double cor(double v1[],double v2[],int len)
{
  return(wn_dot_vects(v1,v2,len)/
   sqrt(wn_norm2_vect(v1,len)*wn_norm2_vect(v2,len)));
}


local void print_debug_dir_stack(cntxt c)
{
  int i;

  printf("========== debug: dir[i] cor gradient ==========\n");

  for(i=0;i<c->debug_dir_stack_count;++i)
  {
    printf("dir[%d] cor grad0opt = %lg, grad0s = %lg\n",
     i,
     cor((c->debug_dir_stack)[i],c->grad0opt,c->num_vars),
     cor((c->debug_dir_stack)[i],c->grad0s,c->num_vars));
  }

  printf("======\n");
}
#endif

local void forward_shift_state(cntxt c)
{
  int i;

#if DEBUG
  push_debug_dir_stack(c);
  print_debug_dir_stack(c);
#endif

  wn_swap(c->vect1,c->vect0,double *);     /* vect1 <- vect0 */
  wn_swap(c->dir1,c->dir0,double *);       /* dir1 <- dir0 */
  wn_swap(c->grad1,c->grad0,double *);     /* grad1 <- grad0 */
  wn_swap(c->grad0,c->grad0opt,double *);  /* grad0 <- grad0opt */
  wn_swap(c->grad1s,c->grad0s,double *);   /* grad1s <- grad0s */

  c->f1 = c->f0;  
  c->f0 = c->f0opt;  c->f0opt = WN_FHUGE;
  c->f1s = c->f0s;   c->f0s = WN_FHUGE;
  c->x1s = c->x0s;   
  c->x1opt = c->x0opt;
  c->isreal1 = c->isreal0;
  c->isreal0 = c->isreal0opt;  c->isreal0opt = FALSE;
  c->norm2_grad1 = c->norm2_grad0;
  c->norm2_grad0 = c->norm2_grad0opt;  
  c->norm2_dir1 = c->norm2_dir0;

  c->dot_grad0_dir1 = c->dot_grad0opt_dir0;
  c->dot_grad1_dir1 = c->dot_grad0_dir0;
  c->dot_grad1s_dir1 = c->dot_grad0s_dir0;
  c->dot_gradls_dir1 = c->dot_gradls_dir0;

  c->iters_since_slow_linesearch_needed1 = 
        c->iters_since_slow_linesearch_needed0;
  ++(c->iters_since_slow_linesearch_needed0);

  ++(c->iters_since_best_f0);
  c->iters_since_best_f1 = c->iters_since_best_f0;

  for(i=NUM_BEST_F-1;i>=1;--i)
  {
    (c->best_f_stack)[i] = (c->best_f_stack)[i-1];
  }

  wn_add_vect_and_scaled_vect_par(c->pc,c->vect0,c->vect1,c->dir1,c->x1opt,
          c->num_vars);
}


local void backward_shift_state(cntxt c)
{
  int i;

#if DEBUG
  pop_debug_dir_stack(c);
#endif

  wn_swap(c->vect0,c->vect1,double *);     /* vect0 <- vect1 */
  wn_swap(c->dir0,c->dir1,double *);       /* dir0 <- dir1 */
  wn_swap(c->grad0opt,c->grad0,double *);  /* grad0opt <- grad0 */
  wn_swap(c->grad0,c->grad1,double *);     /* grad0 <- grad1 */
  wn_swap(c->grad0s,c->grad1s,double *);   /* grad0s <- grad1s */

  c->f0opt = c->f0;
  c->f0 = c->f1;  c->f1 = WN_FHUGE;
  c->f0s = c->f1s;  c->f1s = WN_FHUGE;
  c->x0s = c->x1s;
  c->x0opt = c->x1opt;
  c->isreal0opt = c->isreal0;
  c->isreal0 = c->isreal1;  c->isreal1 = FALSE;
  c->isrealls = FALSE;
  c->norm2_grad0 = c->norm2_grad1;
  c->norm2_grad0opt = c->norm2_grad0;
  c->norm2_dir0 = c->norm2_dir1;

  c->dot_grad0opt_dir0 = c->dot_grad0_dir1;
  c->dot_grad0_dir0= c->dot_grad1_dir1;
  c->dot_grad0s_dir0 = c->dot_grad1s_dir1;
  c->dot_gradls_dir0 = c->dot_gradls_dir1;

  c->iters_since_slow_linesearch_needed0 = 
      c->iters_since_slow_linesearch_needed1 = 0;

  c->iters_since_best_f0 = c->iters_since_best_f1;

  for(i=0;i<NUM_BEST_F-1;++i)
  {
    (c->best_f_stack)[i] = (c->best_f_stack)[i+1];
  }

  c->step_len0 = c->step_len1;
}


local bool default_terminate(double vect[],double best_f,int iteration)
{
  return(FALSE);
}


local void update_best_f(cntxt c,double vect[],double f)
{
  int cmp;

  cmp = fcmp(f,c->best_f);

  if(cmp < 0)
  {
    c->best_f = f;

    c->iters_since_best_f0 = 0;
  }
  
  if(cmp <= 0)
  {
    if((*(c->pterminate))(vect,f,c->iteration))
    {
      wn_force_optimize_stop_flag = TRUE;
    }
  }
}


local void initialize(cntxt c)
{
  wn_gpmake("no_free");
  c->this_group = wn_curgp();

  wn_force_optimize_stop_flag = FALSE;

  wn_make_vect_par(c->pc,&(c->vect0),c->num_vars);
  wn_make_vect_par(c->pc,&(c->vect1),c->num_vars);
  wn_make_vect_par(c->pc,&(c->dir0),c->num_vars);
  wn_make_vect_par(c->pc,&(c->dir1),c->num_vars);
  wn_make_vect_par(c->pc,&(c->grad0),c->num_vars);
  wn_make_vect_par(c->pc,&(c->grad1),c->num_vars);
  wn_make_vect_par(c->pc,&(c->grad0s),c->num_vars);
  wn_make_vect_par(c->pc,&(c->grad0opt),c->num_vars);
  wn_make_vect_par(c->pc,&(c->grad1s),c->num_vars);
  wn_make_vect_par(c->pc,&(c->gradls),c->num_vars);

#if DEBUG
  wn_make_mat(&(c->debug_dir_stack),DEBUG_DEPTH,c->num_vars);
#endif

  c->best_f = WN_FHUGE;
  c->iters_since_best_f0 = 0;

  c->reset_at_iteration = c->no_reset_run_len;

  wn_gppop();
}


local void do_first_funcgrad_call(cntxt c)
{
  int i;
 
  c->f0 = wn_clip_f((*(c->pfuncgrad))(c->grad0,c->vect));
  update_best_f(c,c->vect,c->f0);

  for(i=0;i<NUM_BEST_F;++i)
  {
    (c->best_f_stack)[i] = c->best_f;
  }

  wn_copy_vect_par(c->pc,c->vect0,c->vect,c->num_vars);

  /* search direction same as first gradient */
  wn_copy_vect_par(c->pc,c->dir0,c->grad0,c->num_vars);

  /*
  wn_random_vect(c->dir0,c->num_vars);
  */

  c->isreal0 = TRUE;    /* this is real data, we just got it */

  c->norm2_grad0 = wn_norm2_vect_par(c->pc,c->grad0,c->num_vars);
  c->norm2_dir0 = c->norm2_grad0;
  c->dot_grad0_dir0 = c->norm2_grad0;
  /*
  c->norm2_dir0 = wn_norm2_vect_par(c->pc,c->dir0,c->num_vars);
  c->dot_grad0_dir0 = wn_dot_vects_par(c->pc,c->grad0,c->dir0,c->num_vars);
  */

  if(c->norm2_dir0 == 0.0)
  {
    c->x1opt = 1.0;
  }
  else
  {
    c->x1opt = -wn_abs(c->first_step_len)*sqrt(c->num_vars/c->norm2_dir0);
  }
  c->x0s_init = c->x1opt;
  c->step_len0 = wn_abs(c->x1opt);

  c->iteration_of_last_reset = 0;
}


/*************************************************************************
Linear interpolate *pyout, assuming it is at xout.
*************************************************************************/
local void linear_interpolate
(
  double *pyout,double xout,
  double y1,double x1,
  double y2,double x2
)
{
  double denom;

  denom = x1-x2;

  if(denom == 0.0)
  {
    *pyout = y1*0.5 + y2*0.5;
  }
  else
  {
    *pyout = y1*(xout-x2)/denom + y2*(x1-xout)/denom;
  }
}


/*************************************************************************
Interpolate gradient gopt at coord xopt, 
given gradients at coords x1 and x2.
Use linear interpolation.
*************************************************************************/
local void interpolate_gradopt
(
  cntxt c,
  double *gopt, double xopt,
  double *g1, double x1,
  double *g2, double x2
)
{
  double denom;

  denom = x1-x2;

  if(denom == 0.0)
  {
    wn_add_scaled_vects_par(c->pc,gopt,
          g1,0.5,
          g2,0.5,
          c->num_vars);
  }
  else
  {
    wn_add_scaled_vects_par(c->pc,gopt,
          g1,(xopt-x2)/denom,
          g2,(x1-xopt)/denom,
          c->num_vars);
  }
}


local void fit_minimizing_parabola_2d
(
  bool *psuccess,double *pa,double *px0,double *pb,
  double x1,double f1,double df1,
  double x2,double f2,double df2
)
{
  int code;
  double dist1,dist2;

  wn_fit_parabola_2d(&code,pa,px0,x1,df1,x2,df2);

  if(code == WN_SUCCESS)
  {
    dist1 = wn_abs(x1 - *px0);
    dist2 = wn_abs(x2 - *px0);

    if(dist1 < dist2)
    {
      *pb = f1 - (*pa)*wn_square(dist1);
    }
    else
    {
      *pb = f2 - (*pa)*wn_square(dist2);
    }
  }

  *psuccess = (code == WN_SUCCESS);
}


local bool accuracy_sufficient(double b,double f1,double f2)
{
  double fmax;
  double d,last_d;
  int i;

  wn_assert(b <= f1);
  wn_assert(b <= f2);

  fmax = wn_max(f1,f2);

  d = b;

  for(i=0;i<10;++i)
  {
    last_d  = d;
    d = 0.5*(fmax+last_d);
  }

  return(!((d == fmax)||(d == last_d)));
}


#if 0
local void min_of_cubic(bool *psuccess,double *pb,double *pxmin,
      double coef[4])
{
  double d_coef[3];
  double discriminant;
  double remainder;

  wn_differentiate_poly(d_coef,coef,4);

  discriminant = wn_square(d_coef[1]) - 4.0*d_coef[2]*d_coef[0];
  if(!(discriminant > 0.0))
  {
    *psuccess = FALSE;
    return;
  }

  remainder = 4.0*d_coef[2]*d_coef[0]/wn_square(d_coef[1]);

  if((-0.001 < remainder)&&(remainder < 0.001))
  {
    if(!(d_coef[1] > 0.0))
    {
      *psuccess = FALSE;
      return;
    }

#   if DEBUG
      printf("correction.\n");
#   endif
    *pxmin = -d_coef[0]/d_coef[1] -
    d_coef[2]*wn_square(d_coef[0])/wn_cube(d_coef[1]);
    *pb = wn_eval_poly(*pxmin,coef,4);
    *psuccess = TRUE;
  }
  else
  {
    double x1,x2,y1,y2;
    double sqrt_discriminant;

#   if DEBUG
      printf("quadratic formula.\n");
#   endif

    sqrt_discriminant = sqrt(discriminant);

    x1 = (-d_coef[1] + sqrt_discriminant)/(2.0*d_coef[2]);
    x2 = (-d_coef[1] - sqrt_discriminant)/(2.0*d_coef[2]);
    y1 = wn_eval_poly(x1,coef,4);
    y2 = wn_eval_poly(x2,coef,4);

    if(y1 < y2)
    {
      *pxmin = x1;
      *pb = y1;
      *psuccess = TRUE;
      return;
    }
    else if(y2 < y1)
    {
      *pxmin = x2;
      *pb = y2;
      *psuccess = TRUE;
      return;
    }
    else
    {
      *psuccess = FALSE;
      return;
    }
  }
}
#endif


local void min_of_cubic(bool *psuccess,double *pb,double *pxmin,
      double coef[4])
{
  double d_coef[3];
  double dd_coef[2];
  double discriminant;
  double remainder;
  double x1,x2,y1,y2,ddy1,ddy2,w;
  double sqrt_discriminant;

  wn_differentiate_poly(d_coef,coef,4);
  wn_differentiate_poly(dd_coef,d_coef,3);

  discriminant = wn_square(d_coef[1]) - 4.0*d_coef[2]*d_coef[0];
  if(!(discriminant > 0.0))
  {
    *psuccess = FALSE;
    return;
  }

  sqrt_discriminant = sqrt(discriminant);
  if(!(sqrt_discriminant > 0.0))
  {
    *psuccess = FALSE;
    return;
  }

  w = -0.5*(d_coef[1] + ((double)(qroot_sign(d_coef[1])))*sqrt_discriminant);
  if(!(w != 0.0))
  {
    *psuccess = FALSE;
    return;
  }

  x1 = d_coef[0]/w;  /* near root: the linear equation root for degenerate
            quadratic */
  if(!is_valid_number(x1))
  {
    *psuccess = FALSE;
    return;
  }

  y1 = wn_eval_poly(x1,coef,4);
  ddy1 = wn_eval_poly(x1,dd_coef,2);

  if(ddy1 > 0.0)  /* cubic upward facing at root */
  {
    if(!is_valid_number(y1))
    {
      *psuccess = FALSE;
      return;
    }

    *pxmin = x1;
    *pb = y1;
    *psuccess = TRUE;
    return;
  }

  if(!(d_coef[2] != 0.0))
  {
    *psuccess = FALSE;  /* perfect quadratic, but downward facing */
    return;
  }

  x2 = w/d_coef[2];  /* far root: goes to infinity for degenerate quadratic */
  if(!is_valid_number(x2))
  {
    *psuccess = FALSE;
    return;
  }

  y2 = wn_eval_poly(x2,coef,4);
  ddy2 = wn_eval_poly(x2,dd_coef,2);

  if(ddy2 > 0.0)  /* cubic upward facing at root */
  {
    if(!is_valid_number(y2) || (y2 >= y1))
    {
      *psuccess = FALSE;
      return;
    }

    *pxmin = x2;
    *pb = y2;
    *psuccess = TRUE;
    return;
  }

  *psuccess = FALSE;
  return;
}


/*
  2pd fit gives the best asymtotic convergence rate; however,
  2d fit gives better resistance to roundoff near final convergence.

  Use 2pd, test if accuracy adequate; if not, use 2d.
*/
local void fit_minimizing_parabola
(
  bool *psuccess,double *px0,double *pb,
  double x1,double f1,double df1,
  double x2,double f2,double df2
)
{
  double a;
  double quadratic_x0,quadratic_b;
  int code;

  if(fdcmp(f1,df1,f2,df2) > 0)
  {
    wn_swap(x1,x2,double);
    wn_swap(f1,f2,double);
    wn_swap(df1,df2,double);
  }

# if DEBUG
    printf("fit_minimizing_parabola: x1=%lg,f1=%lg,df1=%lg\n",
     x1,f1,df1);
    printf("fit_minimizing_parabola: x2=%lg,f2=%lg,df2=%lg\n",
     x2,f2,df2);
# endif

  wn_fit_parabola_2pd(&code,&a,px0,pb,
          x1,f1,df1,
          x2,f2);
# if DEBUG
    if(a < 0.0)
    {
      printf("negative curvature detected code = %d,a = %lg\n",code,a);
    }
# endif

  *psuccess = ((code == WN_SUCCESS)&&(a > 0.0));
  if(!(*psuccess))
  {
#   if DEBUG
      printf("failed because wn_fit_parabola_2pd failed.\n");
#   endif
    return;
  }

  if(accuracy_sufficient(*pb,f1,f2))
  {
    double coefs[4];

    wn_fit_cubic_2p2d(&code,coefs,x1,f1,df1,
          x2,f2,df2);
    if(code != WN_SUCCESS)
    {
#     if DEBUG
        printf("wn_fit_cubic_2p2d failed - using quadratic fit b=%lg,x0=%lg.\n",
         *pb,*px0);
#     endif
      *psuccess = TRUE;
      return;
    }

    quadratic_b = *pb;
    quadratic_x0 = *px0;

    min_of_cubic(psuccess,pb,px0,coefs);

#   if DEBUG
      printf("cubic fit: success=%d,b=%lg,x0=%lg.\n",
       *psuccess,*pb,*px0);
#   endif
    if(!(*psuccess))
    {
      *pb = quadratic_b;
      *px0 = quadratic_x0;
      *psuccess = TRUE;

#     if DEBUG
        printf("min_of_cubic failed - using quadratic fit b=%lg,x0=%lg.\n",
         *pb,*px0);
#     endif
    }

    return;
  }

# if DEBUG
    printf("accuracy insufficient -- using 2d fit.\n");
# endif

  fit_minimizing_parabola_2d(psuccess,&a,px0,pb,
           x1,f1,df1,
           x2,f2,df2);
# if DEBUG
  printf("fit_minimizing_parabola_2d: success=%d,a=%lg,b=%lg,x0=%lg.\n",
       *psuccess,*pb,*px0);
# endif
  *psuccess = ((*psuccess)&&(a > 0.0));
}


local double signed_len(double sign,double len)
{
  if(sign < 0)
  {
    return(-len);
  }
  else if(sign > 0)
  {
    return(len);
  }
  else
  {
    if(wn_random_bit())
    {
      return(len);
    }
    else
    {
      return(-len);
    }
  }
}


local void set_jumplen(cntxt c)
{
  c->x0s = c->step_len0;
  if(!(c->x0s != 0.0))
  {
    c->x0s = c->x0s_init;
#if DEBUGBC
    printf("\n");
    printf("Re-setting x0s = x0s_init = %lg\n", c->x0s);
    printf("\n");
#endif
  }

  /* The curvature-based guess of the step length is usually best */
  /* However, it is also more vulnerable to noise */
  /* Check that it does not differ too much from the previous step length: */

  if((c->x1opt != 0)&&(wn_abs(c->x0s) > MAX_STEP_LENGTH_INCREASE*wn_abs(c->x1opt)))
  {
    c->x0s = c->x1opt;
#if DEBUGBC
    printf("\n");
    printf("Step length increased too much, re-setting x0s = %lg\n", c->x0s);
    printf("\n");
#endif
  }

  if(c->iters_since_best_f0 > 6)
  {
    c->x0s *= wn_random_double_between(0.75, 1.25);
#if DEBUGBC
    printf("\n");
    printf("Randomizing jump length: x0s = %lg\n", c->x0s);
    printf("\n");
#endif
  }

  /* force search to be in descent direction */
  c->x0s = -signed_len(c->dot_grad0_dir0,wn_abs(c->x0s));

#if DEBUGBC
  printf("\n");
  printf("After sign check: x0s = %lg\n", c->x0s);
  printf("Delta x = %lg\n", c->x0s*sqrt(c->norm2_dir0));
  printf("\n");
#endif

#if DEBUG
  printf("x0s=%lg\n",c->x0s);
#endif
}


/*
  check that distance from parabola min shrinks by factor "shrink"

  Check 1st and 2nd order conditions.
*/
local bool wolfe_condition_satisfied(double x0,double f0,double df0,
             double xs,double fs,double dfs,
             double shrink)
{
  if(!(fs <= f0-wn_abs(0.25*(1.0-wn_square(shrink))*(xs-x0)*df0)))
  {
    return(FALSE);
  }
  if(!(wn_abs(dfs) < shrink*wn_abs(df0)))
  {
    return(FALSE);
  }

  return(TRUE);
}


local bool opt_improvement_adequate
(
  double xopt,double fopt,double dfopt,
  double xstart,double fstart,double dfstart,
  double narrow_amount
)
{
  double improve_amount;
  double x0,b;
  bool success;

  improve_amount = wn_square(narrow_amount);

# if DEBUG
    printf("opt_improvement_adequate: xopt=%lg,fopt=%lg,dfopt=%lg\n",
     xopt,fopt,dfopt);
    printf("opt_improvement_adequate: xstart=%lg,fstart=%lg,dfstart=%lg\n",
     xstart,fstart,dfstart);
    printf("opt_improvement_adequate: narrow_amount=%lg,improve_amount=%lg\n",
     narrow_amount,improve_amount);
# endif

  fit_minimizing_parabola(&success,&x0,&b,
        xopt,fopt,dfopt,
        xstart,fstart,dfstart);
# if DEBUG
    printf(
      "success=%d,actual_fopt=%lg,desired_fopt=%lg,b=%lg,fstart=%lg\n",
     success,
     fopt,
           (1.0-improve_amount)*b + improve_amount*fstart,
     b,fstart);
# endif
  if(!success)
  {
#   if DEBUG
      printf("quadratic fit failed for opt improvement test.\n");
#   endif
    return(FALSE);
  }

  /* function improvement adequate? */
  if(fopt < (1.0-improve_amount)*b + improve_amount*fstart)
  {
#   if DEBUG
      printf("opt improvement is adequate.\n");
#   endif
    return(TRUE);
  }

  /* if can't tell from function, test derivative */
  if(
      (fopt == (1.0-improve_amount)*b + improve_amount*fstart)
  &&
      (wn_abs(dfopt) < narrow_amount*wn_abs(dfstart))
    )
  {
#   if DEBUG
      printf("opt improvement from derivative is adequate.\n");
#   endif
    return(TRUE);
  }

# if DEBUG
    printf("opt improvement is NOT adequate.\n");
# endif
  return(FALSE);
}


/*
  This routine tests to prevent explosive buildup of error in
  the interpolated gradient.  The growth factor must be
  less than 1.0 for stability, preferably substantially less
*/
local bool gradient_interpolation_error_small_enough
(
  double xopt,    /* x of gradient to be interpolated */
  double xghost,  /* x of gradient already interpolated, containing error */
  double xs,      /* x of actually sampled gradient */
  double shrink_factor
)
{
  bool ret;

  wn_assert(shrink_factor > 0.0);
  wn_assert(shrink_factor < 1.0);

  ret = (wn_abs(xopt-xs) < shrink_factor*wn_abs(xghost-xs));

# if DEBUG
    printf("gradient_interpolation_error_small_enough ret=%d\n",ret);
# endif

  return(ret);
}


local void determine_how_far_to_backout(cntxt c,bool *psuccess0,bool *psuccess1)
{
  *psuccess0 = *psuccess1 = TRUE;

  if(c->isreal0)  /* last linesearch was successful */
  {
#   if DEBUG
      printf("not backing out because last linesearch used real data point\n");
#   endif

    update_best_f(c,c->vect,c->f0s);
    *psuccess0 = FALSE;  /* bad probe, do more careful this linesearch */
    return;
  }
  else  /* !(c->isreal0) */
  {
    c->f0 = wn_clip_f((*(c->pfuncgrad))(c->grad0,c->vect0));
    c->isreal0 = TRUE;
    update_best_f(c,c->vect0,c->f0);
    c->dot_grad0_dir1 = wn_dot_vects_par(c->pc,c->grad0,c->dir1,c->num_vars);

    /* turn this on to include cosine test: (part 1 of 2) */
    /* c->norm2_grad0 = wn_norm2_vect_par(c->pc,c->grad0,c->num_vars); */

    /* Was last linesearch adequate ?                              */
    /* We are going to compare f1 and f1s with both f0 and f0s     */
    /* make sure we are not leaving the smallest value of f behind */
    if(
        opt_improvement_adequate(c->x1opt,c->f0,c->dot_grad0_dir1,
         0.0,c->f1,c->dot_grad1_dir1,
         0.75)
        /*
        (
          (fdcmp(c->f0,c->dot_grad0_dir1,c->f1,c->dot_grad1_dir1) <= 0)
            ||
          (fcmp(c->f0s,c->f1) <= 0)
        )
        */
          &&
        (
          (fdcmp(c->f0,c->dot_grad0_dir1,c->f1s,c->dot_grad1s_dir1) <= 0)
            ||
          (fcmp(c->f0s,c->f1s) <= 0) 
        )
        /* turn this on to include cosine test: (part 2 of 2) */
        /*
          &&
        (wn_abs(c->dot_grad0_dir1)/sqrt(c->norm2_dir1)/sqrt(c->norm2_grad0) < .25)
       */
      )
    {
#     if DEBUG
      printf("not backing out because last linesearch improvement adequate.\n");
#     endif

      update_best_f(c,c->vect,c->f0s);
      c->dot_grad0_dir0 = 
             wn_dot_vects_par(c->pc,c->grad0,c->dir0,c->num_vars);
      c->isreal0opt = FALSE;  /* no opt data */
      *psuccess0 = FALSE;  /* bad probe, do more careful linesearch */
    }
    else  /* last linesearch not adequate, redo it */
    {
#     if DEBUG
      printf("backout out because last linesearch improvement not adequate.\n");
#     endif

      *psuccess1 = FALSE;  /* last linesearch was bad, back out this one 
            and re-do the last linesearch carefully */
    }
  }
}


/*
  Fast line search, which attempts to give CG algorithm which uses only
  1 funcgrad call per linesearch and achieves quadratic convergence.
  For this to work, several conditions must hold:
    1)  Step-length guess must be close.  If not, gradient interpolation
  will be sufficiently inaccurate to cause divergence, and also
  quadratic convergence requires that funcgrad samples be at
  least as good as best several iterations past.
    2)  Improvement in objective function must be sufficient.

  If these conditions are not satisfied, it is necessary to do
  either the current linesearch, or possibly the past linesearch,
  more carefully.
*/
local void do_fast_linesearch0(cntxt c, bool *psuccess0, bool *psuccess1)
{
  bool success;

#if DEBUG
  printf("fast linesearch0 ----------------------------------\n");
#endif

  c->isrealls = FALSE;

  *psuccess0 = *psuccess1 = TRUE;

  wn_add_vect_and_scaled_vect_par(c->pc,c->vect,c->vect0,c->dir0,
          c->x0s,c->num_vars);
  c->f0s = wn_clip_f((*(c->pfuncgrad))(c->grad0s,c->vect));
  c->dot_grad0s_dir0 = wn_dot_vects_par(c->pc,c->grad0s,c->dir0,c->num_vars);

  /* test if sample result consistent with quadratic convergence */
  if(!(
        (c->f0s < (c->best_f_stack)[NUM_BEST_F-2])  
    ||
  (
          (c->f0s == (c->best_f_stack)[NUM_BEST_F-2])  
      &&
          (wn_abs(c->dot_grad0s_dir0) <= wn_abs(c->dot_grad0_dir0))
  )
    ))
  { /* not consistent with quadratic convergence */
#   if DEBUG
      printf("sample point inconsistent with quadratic convergence.\n");
#   endif

    determine_how_far_to_backout(c,psuccess0,psuccess1);
    return;
  }

  fit_minimizing_parabola(&success,&(c->x0opt),&(c->f0opt),
        c->x0s,c->f0s,c->dot_grad0s_dir0,
        0.0,c->f0,c->dot_grad0_dir0);
#if DEBUG
  printf("fit: success=%d,x0opt=%lg,f0opt=%lg\n",
   success,c->x0opt,c->f0opt);
#endif
  if(!success)
  {
#   if DEBUG
      printf("parabolic/cubic fit failed.\n");
#   endif
    determine_how_far_to_backout(c,psuccess0,psuccess1);
    return;
  }
#if DEBUG
  printf("     x0opt/x0s=%lg\n",c->x0opt/c->x0s);
#endif
  if(!(wn_abs(c->x0opt) <= MAX_FAST_LS_MAGNIFICATION*wn_abs(c->x0s)))
  {
#   if DEBUG
      printf("magnification too large.\n");
#   endif
    determine_how_far_to_backout(c,psuccess0,psuccess1);
    return;
  }

  if(
      /* is f0s a significant improvement ? */
      opt_improvement_adequate(c->x0s,c->f0s,c->dot_grad0s_dir0,
             0.0,c->f0,c->dot_grad0_dir0,
             0.95)
  &&
      /* error from cascaded interpolations of gradients must shrink */
      gradient_interpolation_error_small_enough(c->x0opt,0.0,c->x0s,0.9)
    )
  {
    interpolate_gradopt(c,
            c->grad0opt,c->x0opt,
            c->grad0,0.0,
            c->grad0s,c->x0s);
    linear_interpolate(&(c->dot_grad0opt_dir0),c->x0opt,
           c->dot_grad0_dir0,0.0,
           c->dot_grad0s_dir0,c->x0s);
    c->norm2_grad0opt = wn_norm2_vect_par(c->pc, c->grad0opt, c->num_vars);

    /* interpolated gradient at the optimum should make sufficiently 
       large angle with the search direction */
    if(wn_abs(c->dot_grad0opt_dir0)/
       (sqrt(c->norm2_dir0)*sqrt(c->norm2_grad0opt))
       < MAX_COS_VALUE)
    {
      /* use interpolated opt */

#   if DEBUG
      printf("using interpolated f0opt\n");
#   endif

      update_best_f(c,c->vect,c->f0s);
      c->isreal0opt = FALSE;  /* not real data, interpolated data */

#if 0  /* for reasons unclear, forcing orthogonality hurts */
      /* subtract amount of dir0 to force grad0opt orthogonal to dir0 */
      wn_add_scaled_vect(c->grad0opt,c->dir0,
       -(c->dot_grad0opt_dir0/c->norm2_dir0),
       c->num_vars);
      c->dot_grad0opt_dir0 = 0.0;  /* this is forced */
#endif
#   if DEBUG
      printf("interpolated f0opt=%lg\n",c->f0opt);
#   endif

      return;
    }
  }

  /* We now know that f0s not enough improvement */
  /* test to see if true f0opt is enough improvement */
  wn_add_vect_and_scaled_vect_par(c->pc,c->vect,c->vect0,c->dir0,
          c->x0opt,c->num_vars);
  c->f0opt = wn_clip_f((*(c->pfuncgrad))(c->grad0opt,c->vect));
  c->norm2_grad0opt = wn_norm2_vect_par(c->pc, c->grad0opt, c->num_vars);
  c->dot_grad0opt_dir0 = 
    wn_dot_vects_par(c->pc,c->grad0opt,c->dir0,c->num_vars);
#   if DEBUG
  printf("truef0opt=%lg\n",c->f0opt);
#   endif

  /* if f0opt worse than f0s, swap them */
  if(fdcmp(c->f0s,c->dot_grad0s_dir0,c->f0opt,c->dot_grad0opt_dir0) < 0)
  {
#   if DEBUG
    printf("swapped f0opt=%lg, f0s=%lg\n",c->f0opt,c->f0s);
#   endif
    wn_swap(c->x0s,c->x0opt,double);
    wn_swap(c->f0s,c->f0opt,double);
    wn_swap(c->dot_grad0s_dir0,c->dot_grad0opt_dir0,double);
    wn_swap(c->grad0s,c->grad0opt,double *);
    c->norm2_grad0opt = wn_norm2_vect_par(c->pc, c->grad0opt, c->num_vars);
  }
  
  /* is f0opt adequate improvement ? */
  if(
      opt_improvement_adequate(c->x0opt,c->f0opt,c->dot_grad0opt_dir0,
             0.0,c->f0,c->dot_grad0_dir0,
             0.75)
  &&
      (wn_abs(c->dot_grad0opt_dir0)/
        (sqrt(c->norm2_dir0)*sqrt(c->norm2_grad0opt))
         < MAX_COS_VALUE)
    )
  {
#   if DEBUG
    printf("improvement is adequate!!!!!!\n");
#   endif
    update_best_f(c,c->vect,c->f0s);
    c->isreal0opt = TRUE;
    update_best_f(c,c->vect,c->f0opt);

    if(c->f0s < c->f0)
    {
      c->xls = c->x0s;
      c->fls = c->f0s;
      wn_copy_vect_par(c->pc,c->gradls,c->grad0s,c->num_vars);
      c->dot_gradls_dir0 = c->dot_grad0s_dir0;
      c->isrealls = TRUE;
    }
  }
  else  /* f0opt improvement not adequate */
  {
#   if DEBUG
    printf("improvement NOT adequate.\n");
#   endif
    determine_how_far_to_backout(c,psuccess0,psuccess1);
    c->isreal0opt = TRUE;  /* no opt data */
    return;
  }
}


/*
  Calculate weights for generating a search direction conjugate
  to dir1.  Also calculate function curvature along dir1.

  Use Hestenes-Steifel formula.
*/
local void calc_conj_weights
(
  bool *psuccess,
  double *pcurvature,
  double *pwdir1,
  double x0,double x1,
  double dotg0g0,double dotg1g1,double dotg0g1,
  double dotg0dir1,double dotg1dir1,
  double dotdir1dir1
)
{
  double denom;

  if(x1 == x0)      /* bad input data */
  {
#if DEBUGBC
    printf("\n");
    printf("Curvature estimation failure: identical input points.\n");
    printf("\n");
#endif
    *psuccess = FALSE;
    *pwdir1 = 0.0;
    *pcurvature = 1.0;
    return;
  }

  denom = dotg1dir1-dotg0dir1;

  *pcurvature = denom/(dotdir1dir1*(x1-x0));

  if(!((*pcurvature) > 0.0))  /* not parabola opens upward */
  {
#if DEBUGBC
    printf("\n");
    printf("Curvature estimation failure: negative output.\n");
    printf("\n");
#endif
    *psuccess = FALSE;
    *pwdir1 = 0.0;
    return;
  }

  *pwdir1 = (dotg0g0-dotg0g1)/denom;

  *psuccess = TRUE;
}


local void advance_to_new_linesearch(cntxt c)
{
  double dot_grad0_grad1;
  double wdir1;
  double curvature;
  bool success;

#if DEBUG
  printf("x0opt = %lg\n",c->x0opt);
#endif

  forward_shift_state(c);

#if DEBUG
  wn_assert(c->norm2_grad0 == wn_norm2_vect_par(c->pc, c->grad0, c->num_vars));
#endif

  if((c->norm2_grad0 == 0.0)&&(!(c->isreal0)))
  {
    c->f0 = wn_clip_f((*(c->pfuncgrad))(c->grad0,c->vect0));
    update_best_f(c,c->vect0,c->f0);

    c->isreal0 = TRUE;    /* this is real data, we just got it */

    c->norm2_grad0 = wn_norm2_vect_par(c->pc,c->grad0,c->num_vars);
    c->dot_grad0_dir1 = wn_dot_vects_par(c->pc,c->grad0,c->dir1,c->num_vars);
  }
  (c->best_f_stack)[0] = c->best_f;
  if((c->norm2_grad0 == 0.0)&&(c->isreal0))
  {
    return;  /* optimum found */
  }

  if(c->isrealls)
  {
    /* 
      Try to use optimum probe and closed probe next to optimum
      probe to calculate new search direction, using Hestenes-Steifel
      formula.

      linesearch uses this data for scratch data:
        optimimum point:  x0opt, f0opt, grad0opt
        2nd best point:   xls,   fls,   gradls
      After forward_shift_state(...), this becomes:
        optimimum point:  x1opt, f0,    grad0
        2nd best point:   xls,   fls,   gradls
      which is used here.
    */
    calc_conj_weights(&success,&curvature,&wdir1,
          c->x1opt,c->xls,
          c->norm2_grad0,
          wn_norm2_vect_par(c->pc,c->gradls,c->num_vars),
          wn_dot_vects_par(c->pc,c->grad0,c->gradls,c->num_vars),
          c->dot_grad0_dir1,
          c->dot_gradls_dir1,
          c->norm2_dir1);
    if(
  (!success)
    ||
  (wn_abs(c->x1opt - c->xls) < MIN_CONJ_SHRINK*wn_abs(c->step_len1))
      )
    {
      dot_grad0_grad1 = wn_dot_vects_par(c->pc,c->grad0,c->grad1,c->num_vars);

      calc_conj_weights(&success,&curvature,&wdir1,
            c->x1opt,0.0,
            c->norm2_grad0,c->norm2_grad1,dot_grad0_grad1,
            c->dot_grad0_dir1,c->dot_grad1_dir1,
            c->norm2_dir1);
    }
  }
  else  /* !(c->isrealls) */
  {
    dot_grad0_grad1 = wn_dot_vects_par(c->pc,c->grad0,c->grad1,c->num_vars);

    calc_conj_weights(&success,&curvature,&wdir1,
          c->x1opt,0.0,
          c->norm2_grad0,c->norm2_grad1,dot_grad0_grad1,
          c->dot_grad0_dir1,c->dot_grad1_dir1,
          c->norm2_dir1);
  }

  if((!(wdir1 > 0.0))||(!success))
  {
#   if DEBUGBC
      printf("\n");
      if (!success)
  printf("Resetting because (!success)\n");
      else
  printf("Resetting because (!(wdir1 > 0.0))\n");
      printf("\n");
#   endif
#   if DEBUG
      printf("reset !!!!!!!!!!!!!!!!!!!!!!!!!\n");
#   endif
    wdir1 = 0.0;
    c->reset_at_iteration = c->iteration+c->no_reset_run_len;
    c->iteration_of_last_reset = c->iteration+1;
  } 
  else if(c->no_reset_run_len > 0)
  {
    if(c->iteration >= c->reset_at_iteration)
    {
#   if DEBUGBC
      printf("\n");
      printf("Resetting because (no_reset_run_len > 0)&&(iteration >= reset_at_iteration)\n");
      printf("\n");
#   endif
#     if DEBUG
  printf("reset !!!!!!!!!!!!!!!!!!!!!!!!!\n");
#     endif
      wdir1 = 0.0;
      c->no_reset_run_len = (int)(c->no_reset_run_len * c->reset_expand_factor);
      c->reset_at_iteration = c->iteration+c->no_reset_run_len;
      c->iteration_of_last_reset = c->iteration+1;
    }
  }

  wn_add_vect_and_scaled_vect_par(c->pc,c->dir0,
          c->grad0,
                c->dir1,wdir1,
          c->num_vars);

  c->dot_grad0_dir0 = c->norm2_grad0+wdir1*c->dot_grad0_dir1;
  c->norm2_dir0 = c->norm2_grad0+wdir1*wdir1*c->norm2_dir1+
      2.0*wdir1*c->dot_grad0_dir1;

  if(c->norm2_dir0 < 0.0)
  {
    c->norm2_dir0 = wn_norm2_vect_par(c->pc,c->dir0,c->num_vars);
    /*
    c->norm2_dir0 = 0.0;
    */
  }
  if(c->norm2_dir0 == 0.0)
  {
    return;  /* optimum found */
  }

  if(curvature > 0.0)
  {
    c->step_len0 = c->dot_grad0_dir0/(curvature*c->norm2_dir0);
#if DEBUGBC
    printf("\n");
    printf("Positive curvature: curvature = %lg, step_len0 = %lg\n", curvature, c->step_len0);
    printf("\n");
#endif
  }
  else
  {
    c->step_len0 = c->x1opt;  
#if DEBUGBC
    printf("\n");
    printf("Non-positive curvature:  curvature = %lg, step_len0 = %lg\n", curvature, c->step_len0);
    printf("\n");
#endif
  }

#if DEBUG
  printf("wdir1=%lg\n",wdir1);
#endif
}


local void funcgrad_at_x(cntxt c,double x)
{
  c->x0s = x;
  wn_add_vect_and_scaled_vect_par(c->pc,c->vect,c->vect0,c->dir0,c->x0s,
          c->num_vars);
  c->f0s = wn_clip_f((*(c->pfuncgrad))(c->grad0s,c->vect));
  c->dot_grad0s_dir0 = wn_dot_vects_par(c->pc,c->grad0s,c->dir0,
          c->num_vars);
}


local bool points_are_convex(double x0,double f0,double df0,
           double x1,double f1,double df1)
{
  wn_assert(x0 != x1);

  if(x0 > x1)
  {
    wn_swap(x0,x1,double);
    wn_swap(f0,f1,double);
    wn_swap(df0,df1,double);
  }

  return(
    (df0 < df1)
      &&
          (f0 >= f1-(x1-x0)*df1)
      &&
          (f1 >= f0+(x1-x0)*df0)
        );
}


local bool min_is_bracketed(double x0,double f0,double df0,
          double x1,double f1,double df1)
{
  wn_assert(x0 != x1);

  if(x0 > x1)
  {
    wn_swap(x0,x1,double);
    wn_swap(f0,f1,double);
    wn_swap(df0,df1,double);
  }

  return(
    ((df0 < 0.0)&&(df1 > 0.0))
      ||
          ((df0 < 0.0)&&(f0 < f1))
      ||
          ((df1 > 0.0)&&(f0 > f1))
        );
}


local void clear_slow_linesearch_data(cntxt c)
{
  c->sls.isreal0 = FALSE;
  c->sls.isreal1 = FALSE;
  c->sls.isreal2 = FALSE;

  c->sls.quadconv1 = FALSE;

  c->sls.x0 = 0.0;
  c->sls.x1 = 0.0;
  c->sls.x2 = 0.0;

  c->sls.f0 = WN_FHUGE;
  c->sls.f1 = WN_FHUGE;
  c->sls.f2 = WN_FHUGE;

  c->sls.df0 = -WN_FHUGE;
  c->sls.df1 = 0.0;
  c->sls.df2 = WN_FHUGE;

  c->sls.fstart = WN_FHUGE;
  c->sls.dfstart = WN_FHUGE;

  c->sls.min_lost = 0;
  c->sls.bisection_count = 0;
  c->sls.suspect_bisection = 0;
}


local void copy_vect(cntxt c,double *to,double *from)
{
  if(to != from)
  {
    wn_copy_vect_par(c->pc,to,from,c->num_vars);
  }
}


local void update_point0
(
  cntxt c,
  bool *psubstantial_change,
  double x,double f,double df,double *grad
)
{
  bool newls;
  int cmp;

  if(c->sls.isreal0)
  {
    wn_assert(c->sls.x0 < x);
  }
  else
  {
    c->sls.isreal0 = TRUE;
    *psubstantial_change = TRUE;
  }

  if(fdcmp(f,df,c->sls.f0,c->sls.df0) != 0)
  {
    *psubstantial_change = TRUE;
  }

  c->sls.x0 = x;
  c->sls.f0 = f;
  c->sls.df0 = df;

  newls = !(c->sls.isreal2);
  if(!newls)
  {
    cmp = fdcmp(f,df,c->sls.f2,c->sls.df2);

    if(cmp < 0)
    {
      newls = TRUE;
    }
    else if(cmp == 0)
    {
      if(wn_abs(c->sls.x0 - c->sls.x1) < wn_abs(c->sls.x2 - c->sls.x1)) 
      {
        newls = TRUE;
      }
    }
  }

  if(newls)
  {
    c->xls = x;
    c->fls = f;
    copy_vect(c,c->gradls,grad);
    c->dot_gradls_dir0 = df;
    c->isrealls = TRUE;
  }
}


local void update_point2
(
  cntxt c,
  bool *psubstantial_change,
  double x,double f,double df,double *grad
)
{
  bool newls;
  int cmp;

  if(c->sls.isreal2)
  {
    wn_assert(x < c->sls.x2);
  }
  else
  {
    c->sls.isreal2 = TRUE;
    *psubstantial_change = TRUE;
  }

  if(fdcmp(f,df,c->sls.f2,c->sls.df2) != 0)
  {
    *psubstantial_change = TRUE;
  }

  c->sls.x2 = x;
  c->sls.f2 = f;
  c->sls.df2 = df;

  newls = !(c->sls.isreal0);
  if(!newls)
  {
    cmp = fdcmp(f,df,c->sls.f0,c->sls.df0);

    if(cmp < 0)
    {
      newls = TRUE;
    }
    else if(cmp == 0)
    {
      if(wn_abs(c->sls.x2 - c->sls.x1) < wn_abs(c->sls.x0 - c->sls.x1)) 
      {
        newls = TRUE;
      }
    }
  }

  if(newls)
  {
    c->xls = x;
    c->fls = f;
    copy_vect(c,c->gradls,grad);
    c->dot_gradls_dir0 = df;
    c->isrealls = TRUE;
  }
}


local void update_point1
(
  cntxt c,
  bool *psubstantial_change,
  double x,double f,double df,double *grad,
  bool quadconv
)
{
#if DEBUGBC
  printf("Updating c->sls.x1...\n");
#endif

  if(!(c->sls.isreal1))
  {
    c->sls.isreal1 = TRUE;
    *psubstantial_change = TRUE;
  }

  if(fdcmp(f,df,c->sls.f1,c->sls.df1) != 0)
  {
    *psubstantial_change = TRUE;
  }

  c->sls.x1 = x;
  c->sls.f1 = f;
  c->sls.df1 = df;

  c->x0opt = x;
  c->f0opt = f;
  copy_vect(c,c->grad0opt,grad);
  c->dot_grad0opt_dir0 = df;
  c->isreal0opt = TRUE;

  c->sls.quadconv1 = quadconv;
}


local int compare_new_f_with_f1(cntxt c,double x,double f,double df)
{
  int cmp;

  cmp = wn_doublecmp(f,c->sls.f1);  /* new point is best? */
  if(cmp != 0)
  {
    return(cmp);
  }

  if(df == 0.0)
  {
    if(c->sls.df1 == 0.0)
    {
      return(0);
    }
    else
    {
      return(-1);
    }
  }
  else if(df < 0.0)
  {
    if(c->sls.df1 == 0.0)
    {
      return(1);
    }
    else if(c->sls.df1 < 0.0)
    {
      return(-wn_doublecmp(x,c->sls.x1));
    }
    else if(c->sls.df1 > 0.0)
    {
      if(x < c->sls.x1)
      {
  return(wn_doublecmp(wn_abs(df),c->sls.df1));
      }
      else if(x > c->sls.x1)
      {
  return(-wn_doublecmp(wn_abs(df),c->sls.df1));
      }
      else
      {
  wn_assert_notreached();
  return(0);
      }
    }
  }
  else if(df > 0.0)
  {
    if(c->sls.df1 == 0.0)
    {
      return(1);
    }
    else if(c->sls.df1 < 0.0)
    {
      if(x < c->sls.x1)
      {
  return(-wn_doublecmp(wn_abs(df),c->sls.df1));
      }
      else if(x > c->sls.x1)
      {
  return(wn_doublecmp(wn_abs(df),c->sls.df1));
      }
      else
      {
  wn_assert_notreached();
  return(0);
      }
    }
    else if(c->sls.df1 > 0.0)
    {
      return(wn_doublecmp(x,c->sls.x1));
    }
  }

  wn_assert_notreached();
  return(0);
}


local bool insert_adds_info
(
  cntxt c,
  double xnew,
  double fnew,
  double dfnew
)
{
  if(c->sls.isreal1)
  {
    if(!(xnew != c->sls.x1))
    {
      return(FALSE);
    }

    if(fdcmp(fnew,dfnew,c->sls.f1,c->sls.df1) < 0)
    {
      return(TRUE);
    }
  }

  if(c->sls.isreal0)
  {
    if(!(c->sls.x0 < xnew))
    {
      return(FALSE);
    }
  }

  if(c->sls.isreal2)
  {
    if(!(xnew < c->sls.x2))
    {
      return(FALSE);
    }
  }

  return(TRUE);
}


local void insert_funcgrad_into_slow_linesearch_data
(
  cntxt c,
  bool *psubstantial_change,
  double x,double f,double df,double *grad,
  bool quadconv
)
{
  int cmp;

  /* 
    update_point*(...) changes *psubstantial_change to TRUE if it makes
    any change in f or df.  If not, it leaves old value
  */
  *psubstantial_change = FALSE;

  if(!insert_adds_info(c,x,f,df))
  {
#if DEBUG
    printf("Inserting new point would not add info.\n");
#endif
    return;
  }

  if(!(c->sls.isreal1))
  {
    update_point1(c,psubstantial_change,x,f,df,grad,quadconv);
#if DEBUG
    printf("Slow line search structure empty. Adding new point as x1.\n");
#endif
    return;
  }

  cmp = fdcmp(f,df,c->sls.f1,c->sls.df1);

  /* If the new point is best but outside the present bracketing interval, it gets preferential treatment */
  if(cmp  < 0)
  {
    /* Check whether the best point is to the left of the bracketing interval */
    if(
  c->sls.isreal0
   &&
  (x < c->sls.x0)
      )
    {
#if DEBUG
      printf("The best point is to the left of the bracketing interval: (x1, x2) = (x, x0)\n");
#endif

      /* Present version of this method cannot account for the cases when */
      /* we have three points and the best f value comes from outside the bracketing interval */
      /* We would need additional storage variables: c->sls.grad0, c->sls.grad2 */
      wn_assert(!(c->sls.isreal2));

      update_point1(c,psubstantial_change,x,f,df,grad,FALSE);
      update_point2(c,psubstantial_change,c->sls.x0,c->sls.f0,c->sls.df0,
        c->gradls);
      c->sls.isreal0 = FALSE;
      return;
    }
    /* Check whether the best point is to the right of the bracketing interval */
    else if(
       c->sls.isreal2
         &&
       (x > c->sls.x2)
     )
    {
#if DEBUG
      printf("The best point is to the right of the bracketing interval: (x0, x1) = (x2, x)\n");
#endif

      /* Present version of this method cannot account for the cases when */
      /* we have three points and the best f value comes from outside the bracketing interval */
      /* We would need additional storage variables: c->sls.grad0, c->sls.grad2 */
      wn_assert(!(c->sls.isreal0));

      update_point1(c,psubstantial_change,x,f,df,grad,FALSE);
      update_point0(c,psubstantial_change,c->sls.x2,c->sls.f2,c->sls.df2,
              c->gradls);
      c->sls.isreal2 = FALSE;
      return;
    }
  }

  /* We now know that the new point is inside the bracketing interval */
  /* To prevent certain bugs, we do a more careful analysis of the new data */
  cmp = compare_new_f_with_f1(c,x,f,df);  /* new point is best? */

  if(cmp <= 0)  /* new point is best */
  {
    if(c->sls.x1 < x)
    {
      if(c->sls.isreal0 && c->sls.isreal1 && 
   min_is_bracketed(c->sls.x1,c->sls.f1,c->sls.df1,
        c->sls.x0,c->sls.f0,c->sls.df0))
      {
#if DEBUG
        printf("Minimum lost: (x0, x1, x2) -> (x1, xnew, x2).\n");
#endif
        ++(c->sls.min_lost);
      }
      update_point0(c,psubstantial_change,
        c->sls.x1,c->sls.f1,c->sls.df1,c->grad0opt);
      update_point1(c,psubstantial_change,
        x,f,df,grad,quadconv);
#if DEBUG
      printf("New point is best: (x0, x1) = (x1, x)\n");
#endif
    }
    else if(c->sls.x1 > x)
    {
      if(c->sls.isreal2 && c->sls.isreal1 && 
   min_is_bracketed(c->sls.x1,c->sls.f1,c->sls.df1,
        c->sls.x2,c->sls.f2,c->sls.df2))
      {
#if DEBUG
        printf("Minimum lost: (x0, x1, x2) -> (x0, xnew, x1).\n");
#endif
        ++(c->sls.min_lost);
      }              
      update_point2(c,psubstantial_change,
        c->sls.x1,c->sls.f1,c->sls.df1,c->grad0opt);
      update_point1(c,psubstantial_change,
        x,f,df,grad,quadconv);
#if DEBUG
      printf("New point is best: (x1, x2) = (x, x1).\n");
#endif
    }
    else  /* c->sls.x1 == x */
    {
      wn_assert_notreached();
    }
  }
  else /* !(cmp <= 0)  new point is NOT best */
  {
    if(x < c->sls.x1)
    {
      if(c->sls.isreal0 && 
   min_is_bracketed(x,f,df,c->sls.x0,c->sls.f0,c->sls.df0))
      {
#if DEBUG
        printf("Minimum lost: (x0, x1, x2) -> (xnew, x1, x2).\n");
#endif
        ++(c->sls.min_lost);
      }              
      update_point0(c,psubstantial_change,x,f,df,grad);
#if DEBUG
      printf("The new point takes the position x0.\n");
#endif
    }
    else if(x > c->sls.x1)
    {
      if(c->sls.isreal2 && 
   min_is_bracketed(x,f,df,c->sls.x2,c->sls.f2,c->sls.df2))
      {
#if DEBUG
        printf("Minimum lost: (x0, x1, x2) -> (x0, x1, xnew).\n");
#endif
        ++(c->sls.min_lost);
      }          
      update_point2(c,psubstantial_change,x,f,df,grad);
#if DEBUG
      printf("The new point takes the position x2.\n");
#endif
    }
    else  /* x == c->sls.x1 */
    {
      wn_assert_notreached();
    }
  }
}

local bool probe_legal
(
  cntxt c,
  double xnew
)
{
  if(c->sls.isreal0)
  {
    if(!(c->sls.x0 < xnew))
    {
      return(FALSE);
    }
  }
  else
  {
    wn_assert(c->sls.isreal2);

    if(!((c->sls.x1 - xnew) < 
   MAX_SLOW_LS_MAGNIFICATION*(c->sls.x2 - c->sls.x1)))
    {
      return(FALSE);  /* magnification too big */
    }
  }

  if(c->sls.isreal2)
  {
    if(!(xnew < c->sls.x2))
    {
      return(FALSE);
    }
  }
  else
  {
    wn_assert(c->sls.isreal0);

    if(!((xnew - c->sls.x1) < 
   MAX_SLOW_LS_MAGNIFICATION*(c->sls.x1 - c->sls.x0)))
    {
      return(FALSE);  /* magnification too big */
    }
  }

  wn_assert(c->sls.isreal1);

  if(xnew == c->sls.x1)
  {
    return(FALSE);
  }

  return(TRUE);
}


#if 0
local bool bisection_close_to_parabola
(
  double x0,double f0,double df0,
  double f1,double df1,
  double x2,double f2,double df2)
{
  bool success;
  double xopt,b;
  double x1;
  double f1_projection;
  double f_min,f_max;

  fit_minimizing_parabola(&success,&xopt,&b,
        x0,f0,df0,
        x2,f2,df2);

  if(!success)
  {
    return(FALSE);
  }

  if(df0 < df2)
  {
    if(!((df0 <= df1)&&(df1 <= df2)))
    {
      return(FALSE);
    }
  }
  else if(df2 < df0)
  {
    if(!((df2 <= df1)&&(df1 <= df0)))
    {
      return(FALSE);
    }
  }
  else
  {
    return(FALSE);
  }

  x1 = 0.5*(x0+x2);
  f1_projection = a*wn_square(x1-xopt) + b;

  f_min = f0;
  if(f1 < f_min) f_min = f1;
  if(f2 < f_min) f_min = f2;

  f_max = f0;
  if(f1 > f_max) f_max = f1;
  if(f2 > f_max) f_max = f2;

  return(wn_abs(f1_projection - f1) <= 0.1*(f_max - f_min));
}


local bool robust_close_to_parabola(cntxt c,
            double xnew,double fnew,double dfnew)
{
  double bisect_x_lo,bisect_x_hi;

  if(xnew < c->sls.x1)
  {
    if(!(c->sls.x0 < xnew))
    {
      return(FALSE);
    }

    bisect_x_lo = 0.51*c->sls.x0 + 0.49*c->sls.x1;
    bisect_x_hi = 0.49*c->sls.x0 + 0.51*c->sls.x1;

    if(!((bisect_x_lo <= xnew)&&(xnew <= bisect_x_hi)))
    {
      return(FALSE);
    }
      
    return(bisection_close_to_parabola(c->sls.x0,c->sls.f0,c->sls.df0,
               fnew,dfnew,
               c->sls.x1,c->sls.f1,c->sls.df1));
  }
  else if(c->sls.x1 < xnew)
  {
    if(!(xnew < c->sls.x2))
    {
      return(FALSE);
    }

    bisect_x_lo = 0.49*c->sls.x1 + 0.51*c->sls.x2;
    bisect_x_hi = 0.51*c->sls.x1 + 0.49*c->sls.x2;

    if(!((bisect_x_lo <= xnew)&&(xnew <= bisect_x_hi)))
    {
      return(FALSE);
    }
      
    return(bisection_close_to_parabola(c->sls.x1,c->sls.f1,c->sls.df1,
               fnew,dfnew,
               c->sls.x2,c->sls.f2,c->sls.df2));
  }
  else
  {
    wn_assert_notreached();
    return(FALSE);
  }
}
#endif


local bool test_quadconv_point(cntxt c,double f,double df)
{
  return(
    (f < (c->best_f_stack)[NUM_BEST_F-2])
      ||
          (fdcmp(f,df,c->sls.fstart,c->sls.dfstart) <= 0)
        );
}


/*
  Test whether parabola fit from 2 points is consistent with
  quadratic convergence of conjugate-gradient algorithm.
*/
local bool test_quadconv(cntxt c,double f0,double df0,double f1,double df1)
{
  return(
    test_quadconv_point(c,f0,df0)
      &&
    test_quadconv_point(c,f1,df1)
        );
}


/*
  Generate probe point using min of parabola appropriate points
*/
local void gen_parabola_probe
(
  cntxt c,
  bool *psuccess,
  bool *pquadconv,
  double *pxnew
)
{
  double b;

# if DEBUG
    printf("parabola probe.\n");
# endif

  if(!(c->sls.isreal0))
  {
    wn_assert(c->sls.isreal2);

    fit_minimizing_parabola(psuccess,pxnew,&b,
          c->sls.x1,c->sls.f1,c->sls.df1,
          c->sls.x2,c->sls.f2,c->sls.df2);

    if(*psuccess)
    {
      *psuccess = probe_legal(c,*pxnew);
      *pquadconv = test_quadconv(c,c->sls.f2,c->sls.df2,
           c->sls.f1,c->sls.df1);
      return;
    }
  }
  else if(!(c->sls.isreal2))
  {
    fit_minimizing_parabola(psuccess,pxnew,&b,
          c->sls.x1,c->sls.f1,c->sls.df1,
          c->sls.x0,c->sls.f0,c->sls.df0);

    if(*psuccess)
    {
      *psuccess = probe_legal(c,*pxnew);
      *pquadconv = test_quadconv(c,c->sls.f0,c->sls.df0,
           c->sls.f1,c->sls.df1);
      return;
    }
  }
  else /* c->sls.isreal0 && c->sls.isreal2 */
  {
    int side;

    side = fdcmp(c->sls.f0,c->sls.df0,c->sls.f2,c->sls.df2);

    if(side == 0)
    {
      side = wn_random_bit()?(1):(-1);
    }

    if(side < 0)
    {
      fit_minimizing_parabola(psuccess,pxnew,&b,
            c->sls.x1,c->sls.f1,c->sls.df1,
            c->sls.x0,c->sls.f0,c->sls.df0);
      if(*psuccess && probe_legal(c,*pxnew))
      {
  *pquadconv = test_quadconv(c,c->sls.f0,c->sls.df0,
             c->sls.f1,c->sls.df1);
        return;
      }

      fit_minimizing_parabola(psuccess,pxnew,&b,
            c->sls.x1,c->sls.f1,c->sls.df1,
            c->sls.x2,c->sls.f2,c->sls.df2);
      if(*psuccess)
      {
        *psuccess = probe_legal(c,*pxnew);
        *pquadconv = test_quadconv(c,c->sls.f2,c->sls.df2,
             c->sls.f1,c->sls.df1);
  return;
      }
    }
    else if(side > 0)
    {
      fit_minimizing_parabola(psuccess,pxnew,&b,
            c->sls.x1,c->sls.f1,c->sls.df1,
            c->sls.x2,c->sls.f2,c->sls.df2);
      if(*psuccess && probe_legal(c,*pxnew))
      {
  *pquadconv = test_quadconv(c,c->sls.f2,c->sls.df2,
             c->sls.f1,c->sls.df1);
        return;
      }

      fit_minimizing_parabola(psuccess,pxnew,&b,
            c->sls.x1,c->sls.f1,c->sls.df1,
            c->sls.x0,c->sls.f0,c->sls.df0);
      if(*psuccess)
      {
        *psuccess = probe_legal(c,*pxnew);
  *pquadconv = test_quadconv(c,c->sls.f0,c->sls.df0,
             c->sls.f1,c->sls.df1);
  return;
      }
    }
    else
    {
      wn_assert_notreached();
    }
  }

  *pquadconv = FALSE;
  *psuccess = FALSE;
}


local void gen_robust_probe
(
  cntxt c,
  bool *psuccess,
  double *pxnew,
  bool *pbisection,
  double *pdf1,
  double *pdf2
)
{
# if DEBUG
    printf("robust probe.\n");
# endif

  *psuccess = TRUE;
  *pbisection = FALSE;

  if(!(c->sls.isreal0))
  {
    wn_assert(c->sls.isreal2);

    if(min_is_bracketed(c->sls.x2,c->sls.f2,c->sls.df2,
                        c->sls.x1,c->sls.f1,c->sls.df1))
    {
      *pxnew = 0.5*(c->sls.x2 + c->sls.x1);
      *pbisection = TRUE;
      *pdf1 = c->sls.df1;
      *pdf2 = c->sls.df2;
      if(
   (fdcmp(c->sls.f2, c->sls.df2,c->sls.fstart, c->sls.dfstart) <= 0)
     &&
   (fdcmp(c->sls.f1, c->sls.df1, c->sls.fstart, c->sls.dfstart) <= 0)
  )
      {
        ++(c->sls.bisection_count);
      }
      return;
    }
    else
    {
      *pxnew = c->sls.x1 - MAX_SLOW_LS_MAGNIFICATION*(c->sls.x2 - c->sls.x1);
      *pbisection = FALSE;
      *pdf1 = c->sls.df1;
      *pdf2 = c->sls.df2;
      return;
    }
  }
  else if(!(c->sls.isreal2))
  {
    if(min_is_bracketed(c->sls.x0,c->sls.f0,c->sls.df0,
                        c->sls.x1,c->sls.f1,c->sls.df1))
    {
      *pxnew = 0.5*(c->sls.x0 + c->sls.x1);
      *pbisection = TRUE;
      *pdf1 = c->sls.df0;
      *pdf2 = c->sls.df1;
      if(
   (fdcmp(c->sls.f0, c->sls.df0, c->sls.fstart, c->sls.dfstart) <= 0)
     &&
   (fdcmp(c->sls.f1, c->sls.df1, c->sls.fstart, c->sls.dfstart) <= 0)
  )
      {
        ++(c->sls.bisection_count);
      }
      return;
    }
    else
    {
      *pxnew = c->sls.x1 + MAX_SLOW_LS_MAGNIFICATION*(c->sls.x1 - c->sls.x0);
      *pbisection = FALSE;
      *pdf1 = c->sls.df0;
      *pdf2 = c->sls.df1;
      return;
    }
  }
  else /* c->sls.isreal0 && c->sls.isreal2 */
  {
    int side;

    side = fdcmp(c->sls.f0,c->sls.df0,c->sls.f2,c->sls.df2);
    if(side == 0)
    {
      side = wn_random_bit()?(1):(-1);
    }

    if(side < 0)
    {
      if(min_is_bracketed(c->sls.x0,c->sls.f0,c->sls.df0,
                          c->sls.x1,c->sls.f1,c->sls.df1))
      {
        *pxnew = 0.5*(c->sls.x0 + c->sls.x1);
  *pbisection = TRUE;
  *pdf1 = c->sls.df0;
  *pdf2 = c->sls.df1;
        if(
      (fdcmp(c->sls.f0, c->sls.df0, c->sls.fstart, c->sls.dfstart) <= 0)
        &&
      (fdcmp(c->sls.f1, c->sls.df1, c->sls.fstart, c->sls.dfstart) <= 0)
    )
  {
    ++(c->sls.bisection_count);
  }
        return;
      }
      if(min_is_bracketed(c->sls.x2,c->sls.f2,c->sls.df2,
                          c->sls.x1,c->sls.f1,c->sls.df1))
      {
        *pxnew = 0.5*(c->sls.x2 + c->sls.x1);
  *pbisection = TRUE;
  *pdf1 = c->sls.df1;
  *pdf2 = c->sls.df2;
        if(
      (fdcmp(c->sls.f2, c->sls.df2, c->sls.fstart, c->sls.dfstart) <= 0)
        &&
      (fdcmp(c->sls.f1, c->sls.df1, c->sls.fstart, c->sls.dfstart) <= 0)
    )
  {
    ++(c->sls.bisection_count);
  }
        return;
      }

      *psuccess = FALSE;
      *pbisection = FALSE;
      return;
    }
    else if(side > 0)
    {
      if(min_is_bracketed(c->sls.x2,c->sls.f2,c->sls.df2,
                          c->sls.x1,c->sls.f1,c->sls.df1))
      {
        *pxnew = 0.5*(c->sls.x2 + c->sls.x1);
  *pbisection = TRUE;
  *pdf1 = c->sls.df1;
  *pdf2 = c->sls.df2;
        if(
      (fdcmp(c->sls.f2, c->sls.df2, c->sls.fstart, c->sls.dfstart) <= 0)
        &&
      (fdcmp(c->sls.f1, c->sls.df1, c->sls.fstart, c->sls.dfstart) <= 0)
    )
  {
    ++(c->sls.bisection_count);
  }
        return;
      }
      if(min_is_bracketed(c->sls.x0,c->sls.f0,c->sls.df0,
                          c->sls.x1,c->sls.f1,c->sls.df1))
      {
        *pxnew = 0.5*(c->sls.x0 + c->sls.x1);
  *pbisection = TRUE;
  *pdf1 = c->sls.df0;
  *pdf2 = c->sls.df1;
        if(
      (fdcmp(c->sls.f0, c->sls.df0, c->sls.fstart, c->sls.dfstart) <= 0)
        &&
      (fdcmp(c->sls.f1, c->sls.df1, c->sls.fstart, c->sls.dfstart) <= 0)
    )
  {
    ++(c->sls.bisection_count);
  }
        return;
      }

      *psuccess = FALSE;
      *pbisection = FALSE;
      return;
    }
    else
    {
      wn_assert_notreached();
    }
  }
}


/*
  Slow linesearch using 3 points x, f, slope.
  Often uses only 2 points and does not bracket if convergence
  is fast.
  To enable quadratic convergence, exit when new projected
  optimum is good enough, and source points for projection are at least
  as good as previous optimum.

  Linesearch data must be saved for later use in  
  advance_to_new_linesearch(...), to help compute a new conjugate
  search direction, using gradient points which are closest to
  optimum on the linesearch.

      linesearch uses this data for scratch data:
        optimimum point:  x0opt, f0opt, grad0opt
        2nd best point:   xls,   fls,   gradls
      After forward_shift_state(...), this becomes:
        optimimum point:  x1opt, f0,    grad0
        2nd best point:   xls,   fls,   gradls
*/
local void do_slow_linesearch(cntxt c)
{
  int iteration_count;
  double xnew,fnew,dfnew,df0,df1,df2,last_df0,last_df1,last_df2;
  bool success;
  double width01,width12,last_width01,last_width12;
  bool robust;
  bool quadconv;
  bool substantial_change;
  bool test_bisection;
  double bdf1, bdf2;
  int iterations_since_no_substantial_change;

# if DEBUG
  printf("do_slow_linesearch: entering.\n");
# endif

  robust = FALSE;

  last_df0 = WN_FHUGE;
  last_df1 = WN_FHUGE;
  last_df2 = WN_FHUGE;
  last_width01 = WN_FHUGE;
  last_width12 = WN_FHUGE;

  iterations_since_no_substantial_change = 0;
  iteration_count = 0;

  /* no minimum has been lost through bracketing shrinking */
  c->sls.min_lost = 0;
  /* no robust probes have been generated yet */
  c->sls.bisection_count = 0; 
  /* no suspicious bisections have been done yet */
  c->sls.suspect_bisection = 0;

  for(;;++iteration_count)
  {
    df0 = c->sls.df0;
    df1 = c->sls.df1;
    df2 = c->sls.df2;

    if(c->sls.isreal0)
    {
      width01 = c->sls.x1 - c->sls.x0;
    }
    else
    {
      width01 = WN_FHUGE;
    }
    if(c->sls.isreal2)
    {
      width12 = c->sls.x2 - c->sls.x1;
    }
    else
    {
      width12 = WN_FHUGE;
    }

#   if DEBUG
      printf("iteration_count=%d:\n",iteration_count);
      printf("  isreal0=%d,x0=%lg,f0=%lg,df0=%lg\n",
       c->sls.isreal0,c->sls.x0,c->sls.f0,c->sls.df0);
      printf("  isreal1=%d,x1=%lg,f1=%lg,df1=%lg\n",
       c->sls.isreal1,c->sls.x1,c->sls.f1,c->sls.df1);
      printf("  isreal2=%d,x2=%lg,f2=%lg,df2=%lg\n",
       c->sls.isreal2,c->sls.x2,c->sls.f2,c->sls.df2);
      printf("  width01=%lg,last_width01=%lg,width12=%lg,last_width12=%lg\n",
       width01,last_width01,width12,last_width12);
      printf("  df1=%lg,last_df1=%lg\n",
       df1,last_df1);
      printf("  df0=%lg,last_df0=%lg,df2=%lg,last_df2=%lg\n",
       df0,last_df0,df2,last_df2);
#   endif

    if(robust)
    {
      /* if last iteration robust and min is bracketed,
   or number of iterations small, 
   try quadratic probe */
      if(
    (iteration_count <= 2)
      ||
    (
            (
        c->sls.isreal2 
          ||
        min_is_bracketed(c->sls.x0,c->sls.f0,c->sls.df0,
                               c->sls.x1,c->sls.f1,c->sls.df1)
            )
        &&
            (
        c->sls.isreal0 
          ||
              min_is_bracketed(c->sls.x2,c->sls.f2,c->sls.df2,
                         c->sls.x1,c->sls.f1,c->sls.df1)
      )
    )
        )
      {
        robust = FALSE;
      }
    }
    else /* !robust */
    {
      if(
    (width01 > 0.75*last_width01)
      &&
    (width12 > 0.75*last_width12)
      &&
    (wn_abs(df0) > 0.75*wn_abs(last_df0))
      &&
    (wn_abs(df1) > 0.75*wn_abs(last_df1))
      &&
    (wn_abs(df2) > 0.75*wn_abs(last_df2))
        )
      {
  robust = TRUE;
      }
    }

    if(robust)
    {
      gen_robust_probe(c,&success,&xnew,&test_bisection,&bdf1,&bdf2);
      quadconv = FALSE;
    }
    else
    {
      gen_parabola_probe(c,&success,&quadconv,&xnew);
      if(!success)
      {
        gen_robust_probe(c,&success,&xnew,&test_bisection,&bdf1,&bdf2);
  robust = TRUE;
        quadconv = FALSE;
      }
    }

    if(!success)
    {
      break;  /* success */
    }
    if((xnew == c->sls.x0)||(xnew == c->sls.x1)||(xnew == c->sls.x2))
    {
      break;  /* success */
    }
    if(!is_valid_number(xnew))
    {
      break;  /* fail */
    }

#if DEBUGBC
    if(robust)
      printf("Robust probe: xnew = %20.20lg\n", xnew);
    else
      printf("Parabola probe: xnew = %20.20lg\n", xnew);
#endif

    funcgrad_at_x(c,xnew);
    fnew = c->f0s;
    dfnew = c->dot_grad0s_dir0;

    if((robust)&&(success)&&(test_bisection))
    {
      if(
    close_to_each_other(bdf1, bdf2, 0.1)
      &&
          close_to_each_other(bdf1, dfnew, 0.1)
      &&
          close_to_each_other(bdf2, dfnew, 0.1)
  )
      {
        ++(c->sls.suspect_bisection);
      }
    }

#if DEBUGBC
    printf("New derivative = %20.20lg\n", dfnew);
#endif

#   if DEBUG
      printf("do_slow_linesearch: xnew=%lg,fnew=%lg,dfnew=%lg\n",
       xnew,fnew,dfnew);
#   endif 

#if 0
    if(robust)
    {
      if(robust_close_to_parabola(c,xnew,fnew,dfnew))
      {
  robust = FALSE;
      }
    }
#endif

    insert_funcgrad_into_slow_linesearch_data(c,
      &substantial_change,
      xnew,fnew,dfnew,c->grad0s,
      quadconv);

    if(substantial_change)
    {
      iterations_since_no_substantial_change = 0;
    }
    else
    {
      ++iterations_since_no_substantial_change;
      if(iterations_since_no_substantial_change >= 3)
      {
  break;
      }
    }

    if(c->sls.f1 <= -WN_FHUGE)
    {
      break;  /* -infinity result */
    }
    if(c->sls.df1 == 0.0)  /* unlikely */
    {
      break;
    }

    /* normal termination condition */
    if(
  (c->sls.f1 <= c->sls.fstart)
    &&
        (wn_abs(c->sls.df1) <= 0.75*wn_abs(c->sls.dfstart))
    &&
  c->sls.quadconv1
          &&
        points_are_convex(c->x0opt,c->f0opt,c->dot_grad0opt_dir0,
    c->xls,c->fls,c->dot_gradls_dir0)
      )
    {
      break;
    }

    /* Three termination conditions less vulnerable to noise */
    if(
       (fcmp(c->sls.f1, c->sls.fstart) <= 0)
         &&
       (c->sls.min_lost > 0)
         &&
       (c->sls.bisection_count >= BISECTION_MAX)
      )
    {
#if DEBUG
      printf("Exiting slow line search because we bisected %d times and we lost a minimum.\n", BISECTION_MAX);
#endif
      break;
    }

    if(
       (fcmp(c->sls.f1, c->sls.fstart) <= 0)
         &&
       (c->sls.min_lost > MAX_LOST_MIN)
         &&
       (c->sls.suspect_bisection > MAX_SUSPECT_BISECTIONS)
      )
    {
#if DEBUG
      printf("Exiting slow line search because we did %d suspicious bisections and we lost %d minima.\n", MAX_SUSPECT_BISECTIONS, MAX_LOST_MIN);
#endif
      break;
    }

    if(
       (fcmp(c->sls.f1, c->sls.fstart) <= 0)
         &&
       (c->sls.min_lost > 0)
         &&
       (wn_abs(c->sls.df1) <= MINLOST_GRAD_IMPROVE*wn_abs(c->sls.dfstart))
      )
    {
#if DEBUG
      printf("Exiting slow line search because we have improvement and we lost a minimum.\n");
#endif
      break;
    }

    last_df0 = df0;
    last_df1 = df1;
    last_df2 = df2;
    last_width01 = width01;
    last_width12 = width12;
  }

  update_best_f(c, c->grad0opt, c->f0opt);
  c->norm2_grad0opt = wn_norm2_vect_par(c->pc,c->grad0opt,c->num_vars);

# if DEBUGBC
    printf("\n");
    printf("Slow linesearch: exiting.\n");
    printf("Final point: xopt = %lg\n", c->sls.x1);
    printf("Delta x final = %lg\n", c->sls.x1*sqrt(c->norm2_dir0));
    printf("\n");
# endif
# if DEBUG
    printf("do_slow_linesearch: exiting.\n");
# endif
}


local void do_slow_linesearch_backout0(cntxt c)
{
  bool substantial_change;

#if DEBUG
  printf("\n");
  printf("Entering slow linesearch backout.\n");
  printf("xopt = %lg, x0s = %lg, xls = %lg\n", c->x0opt, c->x0s, c->xls);
  printf("\n");
#endif

#if DEBUG
  printf("slow linesearch backout\n");
#endif

  clear_slow_linesearch_data(c);

  if(c->isreal0)
  {
    c->sls.fstart = c->f0;
    c->sls.dfstart = c->dot_grad0_dir0;
  }
  else
  {
    c->sls.fstart = c->f0s;
    c->sls.dfstart = c->dot_grad0s_dir0;
  }

  wn_assert(c->isreal0opt);

#if DEBUG
    printf("Inserting x0opt into slow line search structure...\n");
#endif

  insert_funcgrad_into_slow_linesearch_data(c,
    &substantial_change,
    c->x0opt,c->f0opt,c->dot_grad0opt_dir0,c->grad0opt,
    FALSE);

  if(c->isreal0)
  {
#if DEBUG
    printf("Inserting x0 into slow line search structure...\n");
#endif

    insert_funcgrad_into_slow_linesearch_data(c,
      &substantial_change,
      0.0,c->f0,c->dot_grad0_dir0,c->grad0,
      FALSE);
  }

#if DEBUG
    printf("Inserting x0s into slow line search structure...\n");
#endif

  insert_funcgrad_into_slow_linesearch_data(c,
    &substantial_change,
    c->x0s,c->f0s,c->dot_grad0s_dir0,c->grad0s,        
    FALSE);

  do_slow_linesearch(c);
}
    

local void do_slow_linesearch_fitfail0(cntxt c)
{
  bool substantial_change;

#if DEBUGBC
  printf("\n");
  printf("Entering slow linesearch fitfail.\n");
  printf("xopt = %lg, x0s = %lg, xls = %lg\n", c->x0opt, c->x0s, c->xls);
  printf("\n");
#endif

#if DEBUG
  printf("slow linesearch fitfail0\n");
#endif

  clear_slow_linesearch_data(c);

  wn_assert(c->isreal0);

  c->sls.fstart = c->f0;
  c->sls.dfstart = c->dot_grad0_dir0;

  if(c->isreal0opt)
  {
#if DEBUG
    printf("Inserting x0opt into slow line search structure...\n");
#endif
    insert_funcgrad_into_slow_linesearch_data(c,
      &substantial_change,
      c->x0opt,c->f0opt,c->dot_grad0opt_dir0,c->grad0opt,
      FALSE);
  }

#if DEBUG
    printf("Inserting x0 into slow line search structure...\n");
#endif
  insert_funcgrad_into_slow_linesearch_data(c,
    &substantial_change,
    0.0,c->f0,c->dot_grad0_dir0,c->grad0,
    FALSE);

#if DEBUG
    printf("Inserting x0s into slow line search structure...\n");
#endif
  insert_funcgrad_into_slow_linesearch_data(c,
    &substantial_change,
    c->x0s,c->f0s,c->dot_grad0s_dir0,c->grad0s,
    FALSE);
  
  do_slow_linesearch(c);
}

local bool termination_condition(cntxt c)
{
  if(wn_force_optimize_stop_flag)
  {
    return(TRUE);
  }
  if((c->isreal0)&&(c->norm2_grad0 == 0.0))  /* unlikely */
  {
    return(TRUE);
  }
  if(c->norm2_dir0 == 0.0)  /* unlikely */
  {
    return(TRUE);
  }
  if(c->iters_since_best_f0 > 10)  /* not improving any more */
  {
    return(TRUE);
  }

  return(FALSE);
}


local int solution_cmp(bool isreal1,double f1,
           bool isreal2,double f2)
{
  if((int)isreal1 < (int)isreal2)
  {
    return(1);  /* reverse order */
  }
  else if((int)isreal1 > (int)isreal2)
  {
    return(-1);  /* reverse order */
  }

  if(f1 < f2)
  {
    return(-1);
  }
  else if(f2 > f1)
  {
    return(1);
  }

  return(0);
}


local void find_best_final_solution(cntxt c)
{
  bool best_isreal;
  double best_f;
  int best_index;

  best_isreal = FALSE;
  best_f = WN_FHUGE;
  best_index = -1;

  if(solution_cmp(c->isreal0,c->f0,best_isreal,best_f) < 0)
  {
    best_index = 1;

    best_isreal = c->isreal0;
    best_f = c->f0;
  }
  if(solution_cmp(c->isreal1,c->f1,best_isreal,best_f) < 0)
  {
    best_index = 2;

    best_isreal = c->isreal1;
    best_f = c->f1;
  }
  if(solution_cmp((c->f0s < WN_FHUGE),c->f0s,best_isreal,best_f) < 0)
  {
    best_index = 3;

    best_isreal = (c->f0s < WN_FHUGE);
    best_f = c->f0s;
  }
  if(solution_cmp((c->f1s < WN_FHUGE),c->f1s,best_isreal,best_f) < 0)
  {
    best_index = 4;

    best_isreal = (c->f1s < WN_FHUGE);
    best_f = c->f1s;
  }
  if(solution_cmp(c->isreal0opt,c->f0opt,best_isreal,best_f) < 0)
  {
    best_index = 5;

    best_isreal = c->isreal0opt;
    best_f = c->f0opt;
  }
  if(solution_cmp(c->isrealls,c->fls,best_isreal,best_f) < 0)
  {
    best_index = 6;

    best_isreal = c->isrealls;
    best_f = c->fls;
  }

  switch(best_index)
  {
    default:
      wn_assert_notreached();
      break;
    case(1):
      wn_copy_vect_par(c->pc,c->vect,c->vect0,c->num_vars);
      break;
    case(2):
      wn_copy_vect_par(c->pc,c->vect,c->vect1,c->num_vars);
      break;
    case(3):
      wn_add_vect_and_scaled_vect_par(c->pc,
              c->vect,c->vect0,c->dir0,c->x0s,
              c->num_vars);
      break;
    case(4):
      wn_add_vect_and_scaled_vect_par(c->pc,
              c->vect,c->vect1,c->dir1,c->x1s,
              c->num_vars);
      break;
    case(5):
      wn_add_vect_and_scaled_vect_par(c->pc,
              c->vect,c->vect0,c->dir0,c->x0opt,
              c->num_vars);
      break;
    case(6):
      wn_add_vect_and_scaled_vect_par(c->pc,
              c->vect,c->vect0,c->dir0,c->xls,
              c->num_vars);
      break;
  }
}


local void finish(cntxt c,int *pcode,double *pval_min)
{
  *pval_min = c->best_f;

  find_best_final_solution(c);

  if(!(c->best_f > -WN_FHUGE))
  {
    *pcode = WN_UNBOUNDED;
  }
  else if(wn_force_optimize_stop_flag)
  {
    *pcode = WN_SUBOPTIMAL;
  }
  else
  {
    *pcode = WN_SUCCESS;
  }

#if DEBUG
  /* printf("Checking returned minimum value of f...\n"); */
  wn_assert(wn_clip_f((*(c->pfuncgrad))(c->grad0,c->vect)) == *pval_min);
#endif

  wn_gppush(c->this_group);
  wn_gpfree();
}


EXTERN void wn_conj_funcgrad_method_parallel
(
  int *pcode,
  double *pval_min,
  double vect[],
  int num_vars,
  double (*pfuncgrad)(double grad[],double vect[]),
  bool (*pterminate)(double vect[],double best_f,int iteration),
  double first_step_len,
  int no_reset_run_len,
  double reset_expand_factor,
  int max_iterations,
  wn_parallel_callback par_cb,
  int num_threads
)
{
  cntxt_s cntxt_mem,*c;
  bool success0,success1;

  c = &cntxt_mem;
  wn_memzero((ptr)c,sizeof(cntxt_s));

  c->pc = wn_parvect_init(par_cb, num_threads, num_vars);

  c->vect = vect;
  c->num_vars = num_vars;
  c->pfuncgrad = pfuncgrad;
  if(pterminate == NULL)
  {
    c->pterminate = &default_terminate;
  }
  else
  {
    c->pterminate = pterminate;
  }
  if(first_step_len == 0.0)
  {
    c->first_step_len = 1.0;
  }
  else
  {
    c->first_step_len = first_step_len;
  }
  c->no_reset_run_len = no_reset_run_len;
  c->reset_expand_factor = reset_expand_factor;
  c->max_iterations = max_iterations;

  initialize(c);

  do_first_funcgrad_call(c);

  for(c->iteration=0;;++(c->iteration))
  {
    if(termination_condition(c))
    {
      break;
    }
    if((c->iteration >= max_iterations)&&(max_iterations < WN_IHUGE))
    {
      wn_force_optimize_stop_flag = TRUE;
      break;
    }

    set_jumplen(c);

#if DEBUGBC
    printf("\n");
    printf("Starting new line search.\n");
    printf("Function value in origin point is %20.100lg\n", c->f0);
    printf("\n");
#endif

    do_fast_linesearch0(c,&success0,&success1);

#if DEBUGBC
    printf("\n");
    printf("Finished fast line search.\n");
    printf("xopt = %lg, x0s = %lg, xls = %lg\n", c->x0opt, c->x0s, c->xls);
    printf("\n");
#endif

    if(!success1)
    {
      backward_shift_state(c);
      do_slow_linesearch_backout0(c);
    }
    else if(!success0)
    {
      do_slow_linesearch_fitfail0(c);
    }

#if DEBUGBC
    printf("\n");
    printf("Closing line search.\n");
    printf("xopt = %lg, x0s = %lg, xls = %lg\n", c->x0opt, c->x0s, c->xls);
    printf("\n");
#endif

    advance_to_new_linesearch(c);

#if DEBUGBC
    printf("\n");
    printf("MIN = %20.100lg\n", c->best_f);
    printf("Iterations since best f = %d\n", c->iters_since_best_f0);
    printf("\n");
#endif
  }

  finish(c,pcode,pval_min);

  wn_parvect_free(c->pc);
}


EXTERN void wn_conj_funcgrad_method
(
  int *pcode,
  double *pval_min,
  double vect[],
  int num_vars,
  double (*pfuncgrad)(double grad[],double vect[]),
  bool (*pterminate)(double vect[],double best_f,int iteration),
  double first_step_len,
  int no_reset_run_len,
  double reset_expand_factor,
  int max_iterations
)
{
  wn_conj_funcgrad_method_parallel(pcode, pval_min, vect, num_vars, pfuncgrad,
                 pterminate, first_step_len, 
           no_reset_run_len, reset_expand_factor,
           max_iterations,
                 /*par_cb*/NULL, /*num_threads*/1);
}
