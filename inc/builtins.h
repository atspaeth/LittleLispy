#ifndef BUILTINS_H
#define BUILTINS_H

#include "lisp.h"


extern obj_t nil, t;

typedef obj_t builtin_t(obj_t);

// Special forms
builtin_t op_if, op_quote, op_lambda, op_mu, op_set, op_let;

// The basic predicates
builtin_t fn_consp, fn_symp, fn_mintp, fn_funp, fn_equal;

// Arithmetic functions
builtin_t fn_add, fn_mul, fn_sub, fn_div;

// List functions
builtin_t fn_list, fn_cons, fn_car, fn_cdr;

// The REPL
builtin_t op_read, op_eval, op_print;




#endif // BUILTINS_H
