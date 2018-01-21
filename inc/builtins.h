#ifndef BUILTINS_H
#define BUILTINS_H

#include "lisp.h"


extern obj_t nil, t;

typedef obj_t builtin_t(obj_t);

// Special forms
builtin_t op_cond, op_quote, op_lambda, op_mu, op_do;
builtin_t op_set, op_def, op_and, op_or;

// The basic predicates
builtin_t fn_consp, fn_nullp, fn_symp, fn_mintp, fn_funp;
builtin_t fn_less, fn_greater, fn_lesseq, fn_greatereq;
builtin_t fn_equal, fn_notequal;

// Arithmetic functions
builtin_t fn_add, fn_mul, fn_sub, fn_div, fn_mod;

// List functions
builtin_t fn_list, fn_cons, fn_car, fn_cdr;

// Little bits of magic
builtin_t fn_error, fn_eval, fn_print, fn_printnl;


void setup_builtins(void);



#endif // BUILTINS_H
