#include "builtins.h"
#include <stdlib.h>


obj_t
op_if(obj_t args) {
  obj_t cond = car(args);
  obj_t thn = car(cdr(args));
  obj_t els = car(cdr(cdr(args)));
  
  if (!nullp(eval(cond)))
    return eval(thn);
  else
    return eval(els);
}

obj_t
op_quote(obj_t args) {
  return args;
}

obj_t
op_lambda(obj_t args) {
  func_t *fun = malloc(sizeof(func_t));
  fun->f = make_interp(as_cons(args));
  return make_func(fun);
}

obj_t
fn_equal(obj_t args) {
  obj_t item = car(args);
  while (!nullp(args = cdr(args))) {
    if (!eqp(car(args), item))
      return nil;
  }
  return t;
}

obj_t
op_set(obj_t args) {
  obj_t name = car(args);
  obj_t val = eval(car(cdr(args)));

  if (gettype(name) == TYPE_SYM && as_sym(name) != as_sym(nil)) {
    bind_sym(as_sym(name), val);
  }

  if (nullp(cdr(cdr(args)))) return val;
  else return op_set(cdr(cdr(args)));
}

obj_t
fn_add(obj_t args) {
  mint_t sum = 0;
  while (!nullp(args)) {
    if (!mintp(car(args))) return nil;
    sum += as_mint(car(args));
    args = cdr(args);
  }
  return make_mint(sum);
}

