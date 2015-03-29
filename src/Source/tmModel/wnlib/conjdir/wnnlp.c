#include <stdio.h>
#include <math.h>

#include "wnlib.h"
#include "wnasrt.h"
#include "wnabs.h"
#include "wnmax.h"
#include "wnmem.h"
#include "wnsll.h"
#include "wnvect.h"
#include "wnmat.h"

#include "wnconj.h"

#include "wnnlp.h"



int wn_nlp_verbose;


local int count;
local int num_vars;
local double *solution_vect;
local double *delta_vect;
local double *values,*grad_buffer;
local wn_nonlinear_constraint_type objective;
local wn_sll constraint_list;


local void check_nonlinear_constraint(wn_nonlinear_constraint_type constraint)
{
  int i,var;

  for(i=0;i<constraint->size;++i)
  {
    var = (constraint->vars)[i];

    wn_assert(var >= 0);
    wn_assert(var < num_vars);
  }
}


local void check_linear_constraint(wn_linear_constraint_type constraint)
{
  int i,var;

  for(i=0;i<constraint->size;++i)
  {
    var = (constraint->vars)[i];

    wn_assert(var >= 0);
    wn_assert(var < num_vars);
  }
}


local void check_constraint(wn_nonlinear_constraint_type constraint)
{
  switch(constraint->type)
  {
    default:
      wn_assert_notreached();
      break;
    case(WN_NONLINEAR_CONSTRAINT):
      check_nonlinear_constraint(constraint);
      break;
    case(WN_LINEAR_CONSTRAINT):
      check_linear_constraint((wn_linear_constraint_type)constraint);
      break;
  }
}


local void check_inputs(void)
{
  wn_sll el;
  wn_nonlinear_constraint_type constraint;

  check_constraint(objective);

  for(el=constraint_list;el!=NULL;el=el->next)
  {
    constraint = (wn_nonlinear_constraint_type)(el->contents);

    check_constraint(constraint);
  }
}


local void initialize(void)
{
  wn_sll el;
  wn_nonlinear_constraint_type constraint;

  values = (double *)wn_zalloc(num_vars*sizeof(double));
  grad_buffer = (double *)wn_zalloc(num_vars*sizeof(double));

  for(el=constraint_list;el!=NULL;el=el->next)
  {
    constraint = (wn_nonlinear_constraint_type)(el->contents);

    constraint->offset = 0.0;
  }
}


local void compute_diff_nonlinear
(
  double *pdiff,
  double vect[],
  wn_nonlinear_constraint_type constraint
)
{
  int i,size;
  int *vars;

  wn_assert(constraint->type == WN_NONLINEAR_CONSTRAINT);

  size = constraint->size;
  vars = constraint->vars;

  for(i=0;i<size;++i)
  {
    values[i] = vect[vars[i]];
  }

  *pdiff = (*(constraint->pfunction))(size,values,constraint->client_data);
}


local void compute_diff_linear
(
  double *pdiff,
  double vect[],
  wn_linear_constraint_type constraint
)
{
  int i,size;
  int *vars;
  double *weights;
  double sum;

  wn_assert(constraint->type == WN_LINEAR_CONSTRAINT);

  size = constraint->size;
  vars = constraint->vars;
  weights = constraint->weights;

  sum = 0.0;

  for(i=0;i<size;++i)
  {
    sum += weights[i] * vect[vars[i]];
  }

  *pdiff = sum - constraint->rhs;
}


local void compute_diff
(
  double *pdiff,
  double vect[],
  wn_nonlinear_constraint_type constraint
)
{
  switch(constraint->type)
  {
    default:
      wn_assert_notreached();
      break;
    case(WN_NONLINEAR_CONSTRAINT):
      compute_diff_nonlinear(pdiff,vect,constraint);
      break;
    case(WN_LINEAR_CONSTRAINT):
      compute_diff_linear(pdiff,vect,(wn_linear_constraint_type)constraint);
      break;
  }
}


local double function_1_constraint(double vect[],
           wn_nonlinear_constraint_type constraint)
{
  double diff,offset;

  compute_diff(&diff,vect,constraint);
  offset = constraint->offset;

  diff -= offset; 

  switch(constraint->comparison_type)
  {
    default:
      wn_assert_notreached();
      return(0.0);

    case(WN_EQ_COMPARISON):
      return(diff*diff);

    case(WN_GT_COMPARISON):
      if(diff < 0.0)
      {
  return(diff*diff);
      }
      else
      {
  return(0.0);
      }

    case(WN_LT_COMPARISON):
      if(diff > 0.0)
      {
  return(diff*diff);
      }
      else
      {
  return(0.0);
      }
  }
}


local void increment_gradient_for_linear_constraint
(
  double grad[],
  wn_linear_constraint_type constraint,
  double diff
)
{
  int i,size;
  int *vars;
  double *weights;
  double weight;

  wn_assert(constraint->type == WN_LINEAR_CONSTRAINT);

  diff *= 2.0;

  size = constraint->size;
  vars = constraint->vars;
  weights = constraint->weights;

  for(i=0;i<size;++i)
  {
    weight = weights[i];
  
    grad[vars[i]] += weight*diff;
  }
}


local void inc_grad_numerical_for_nonlinear_constraint
(
  double grad[],
  double vect[],
  wn_nonlinear_constraint_type constraint,
  double diff
)
{
  int i,size;
  int *vars;
  double delta,fminus,fplus,df_dxi;

  wn_assert(constraint->type == WN_NONLINEAR_CONSTRAINT);

  diff *= 2.0;

  size = constraint->size;
  vars = constraint->vars;

  for(i=0;i<size;++i)
  {
    values[i] = vect[vars[i]];
  }

  for(i=0;i<size;++i)
  {
    delta = delta_vect[vars[i]];

    values[i] = vect[vars[i]] - delta;
    fminus = (*(constraint->pfunction))(size,values,constraint->client_data);

    values[i] = vect[vars[i]] + delta;
    fplus = (*(constraint->pfunction))(size,values,constraint->client_data);

    values[i] = vect[vars[i]];

    df_dxi = (fplus-fminus)/(2.0*delta);

    grad[vars[i]] += diff*df_dxi;
  }
}


local void inc_grad_symbolic_for_nonlinear_constraint
(
  double grad[],
  double vect[],
  wn_nonlinear_constraint_type constraint,
  double diff
)
{
  int i,size;
  int *vars;

  wn_assert(constraint->type == WN_NONLINEAR_CONSTRAINT);

  diff *= 2.0;

  size = constraint->size;
  vars = constraint->vars;

  for(i=0;i<size;++i)
  {
    values[i] = vect[vars[i]];
  }

  (*(constraint->pgradient))(grad_buffer,size,values,constraint->client_data);

  for(i=0;i<size;++i)
  {
    grad[vars[i]] += diff*grad_buffer[i];
  }
}


local void increment_gradient_for_nonlinear_constraint
(
  double grad[],
  double vect[],
  wn_nonlinear_constraint_type constraint,
  double diff
)
{
  if(constraint->pgradient == NULL)
  {
    inc_grad_numerical_for_nonlinear_constraint(grad,vect,constraint,diff);
  }
  else
  {
    inc_grad_symbolic_for_nonlinear_constraint(grad,vect,constraint,diff);
  }
}


local void increment_gradient_for_constraint
(
  double grad[],
  double vect[],
  wn_nonlinear_constraint_type constraint,
  double diff
)
{
  switch(constraint->type)
  {
    default:
      wn_assert_notreached();
      break; 
    case(WN_NONLINEAR_CONSTRAINT):
      increment_gradient_for_nonlinear_constraint(grad,vect,constraint,diff);
      break;
    case(WN_LINEAR_CONSTRAINT):
      increment_gradient_for_linear_constraint(
        grad,(wn_linear_constraint_type)constraint,diff);
      break;
  }
}


local void gradient_1_constraint(double grad[],
         double vect[],
         wn_nonlinear_constraint_type constraint)
{
  double diff,offset;

  compute_diff(&diff,vect,constraint);
  offset = constraint->offset;

  diff -= offset; 

  switch(constraint->comparison_type)
  {
    default:
      wn_assert_notreached();
      break;

    case(WN_EQ_COMPARISON):
      increment_gradient_for_constraint(grad,vect,constraint,diff);      
      break;

    case(WN_GT_COMPARISON):
      if(diff < 0.0)
      {
        increment_gradient_for_constraint(grad,vect,constraint,diff);      
      }
      break;

    case(WN_LT_COMPARISON):
      if(diff > 0.0)
      {
        increment_gradient_for_constraint(grad,vect,constraint,diff);      
      }
      break;
  }
}


local double function(double vect[])
{
  double sum;
  wn_nonlinear_constraint_type constraint;
  wn_sll el;

  compute_diff(&sum,vect,objective);

  for(el=constraint_list;el!=NULL;el=el->next)
  {
    constraint = (wn_nonlinear_constraint_type)(el->contents);

    sum += function_1_constraint(vect,constraint);
  }

  if(wn_nlp_verbose >= 2)
  {
    printf("ob = %20.20lf\n",sum);
    fflush(stdout);
  }

  return(sum);
}


local void gradient(double grad[],double vect[])
{
  wn_nonlinear_constraint_type constraint;
  wn_sll el;

  if(wn_nlp_verbose >= 2)
  {
    printf("gradient. count = %d\n",count);
    fflush(stdout);
  }
  ++count;

  wn_zero_vect(grad,num_vars);

  increment_gradient_for_constraint(grad,vect,objective,0.5);      

  for(el=constraint_list;el!=NULL;el=el->next)
  {
    constraint = (wn_nonlinear_constraint_type)(el->contents);

    gradient_1_constraint(grad,vect,constraint);
  }
}


local double funcgrad(double grad[],double vect[])
{
  double func;

  func = function(vect);
  gradient(grad,vect);

  return(func);
}


local void adjust_offsets(double offset_adjust_rate)
{
  wn_sll el;
  wn_nonlinear_constraint_type constraint;
  double diff,adiff,max_adiff,sum_adiff;

  sum_adiff = 0.0;
  max_adiff = 0.0;

  for(el=constraint_list;el!=NULL;el=el->next)
  {
    constraint = (wn_nonlinear_constraint_type)(el->contents);

    compute_diff(&diff,solution_vect,constraint);

    adiff = 0.0;

    switch(constraint->comparison_type)
    {
      default:
        wn_assert_notreached();
        break;
  
      case(WN_EQ_COMPARISON):
  constraint->offset += (-diff*offset_adjust_rate);
  adiff = wn_abs(diff);
        break;
  
      case(WN_GT_COMPARISON):
        if(diff < constraint->offset)
        {
    constraint->offset += (-diff*offset_adjust_rate);
    if(constraint->offset < 0.0)
    {
      constraint->offset = 0.0;
          }
    adiff = wn_abs(diff);
        }
        break;
  
      case(WN_LT_COMPARISON):
        if(diff > constraint->offset)
        {
    constraint->offset += (-diff*offset_adjust_rate);
    if(constraint->offset > 0.0)
    {
      constraint->offset = 0.0;
          }
    adiff = wn_abs(diff);
        }
        break;
    }

    max_adiff = wn_max(max_adiff,adiff);
    sum_adiff += adiff;
  }

  if(wn_nlp_verbose >= 1)
  {
    printf("max_adiff = %lf,mean_adiff = %lf\n",
     max_adiff,sum_adiff/wn_sllcount(constraint_list));
    fflush(stdout);
  }
}


void wn_nlp_conj_method
(
  int *pcode,double *pval_min,double passed_solution_vect[],
  double passed_delta_vect[],
  wn_nonlinear_constraint_type passed_objective,wn_sll passed_constraint_list,
  int passed_num_vars,int conj_iterations,int offset_iterations,
  double offset_adjust_rate
)
{
  int i;

  wn_gpmake("no_free");

  solution_vect = passed_solution_vect;
  delta_vect = passed_delta_vect;
  objective = passed_objective;
  constraint_list = passed_constraint_list;
  num_vars = passed_num_vars;

  check_inputs();

  initialize();

  for(i=0;i<offset_iterations;++i) 
  {
    count = 0;

    /*
    wn_conj_gradient_method(pcode,pval_min,
                solution_vect,num_vars,(function),(gradient),
          conj_iterations);
    */
    wn_conj_funcgrad_method(pcode,pval_min,
                            solution_vect,num_vars,
                            &funcgrad,
                            (bool (*)(double *,double,int))NULL,
                            1.0,
          0.0,
                            0.0,
          conj_iterations); 
    if((*pcode != WN_SUCCESS)&&(*pcode != WN_SUBOPTIMAL))
    {
      break;
    }

    compute_diff(pval_min,solution_vect,objective);

    if(wn_nlp_verbose >= 3)
    {
      wn_print_vect(solution_vect,num_vars);
    }
    if(wn_nlp_verbose >= 1)
    {
      printf("iteration=%d,real ob = %lf\n",i,*pval_min);
      fflush(stdout);
    }

    adjust_offsets(offset_adjust_rate);
  }

  wn_gpfree();
}


void wn_make_linear_constraint(wn_linear_constraint_type *pconstraint,
                   int size,double rhs,int comparison_type)
{
  int i;

  *pconstraint = (wn_linear_constraint_type)
   wn_zalloc(sizeof(struct wn_linear_constraint_type_struct));

  (*pconstraint)->type = WN_LINEAR_CONSTRAINT;

  (*pconstraint)->size = size;
  (*pconstraint)->rhs = rhs;
  (*pconstraint)->comparison_type = comparison_type;

  (*pconstraint)->vars = (int *)wn_zalloc(size*sizeof(int));
  (*pconstraint)->weights = (double *)wn_zalloc(size*sizeof(double));

  for(i=0;i<size;++i)
  {
    ((*pconstraint)->vars)[i] = -1;
  }
}


void wn_make_nonlinear_constraint(wn_nonlinear_constraint_type *pconstraint,
                      int size,int comparison_type)
{
  int i;

  *pconstraint = (wn_nonlinear_constraint_type)
  wn_zalloc(sizeof(struct wn_nonlinear_constraint_type_struct));

  (*pconstraint)->type = WN_NONLINEAR_CONSTRAINT;

  (*pconstraint)->size = size;
  (*pconstraint)->comparison_type = comparison_type;

  (*pconstraint)->vars = (int *)wn_zalloc(size*sizeof(int));

  (*pconstraint)->pfunction = NULL; 
  (*pconstraint)->pgradient = NULL; 

  for(i=0;i<size;++i)
  {
    ((*pconstraint)->vars)[i] = -1;
  }
}

