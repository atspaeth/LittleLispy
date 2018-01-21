#include "builtins.h"
#include <stdlib.h>

void
assert_argcount(obj_t args, size_t count) {
  size_t counted = 0;
  while (!nullp(args)) {
    counted++;
    args = cdr(args);
  }
  if (counted != count)
    error(E_WRONG_ARGCOUNT, make_mint(counted));
}

obj_t
create_builtin(const char *name, builtin_t *func) {
  func_t *funobj = malloc(sizeof(func_t));
  *funobj = (func_t){make_compiled(func)};
  return make_sym(make_const(name, make_func(funobj)));
}

obj_t
create_special_form(const char *name, builtin_t *func) {
  func_t *funobj = malloc(sizeof(func_t));
  *funobj = (func_t){make_special(func)};
  return make_sym(make_const(name, make_func(funobj)));
}

void
setup_builtins() {
  create_special_form("cond", &op_cond);
  create_special_form("if", &op_if);
  create_special_form("quote", &op_quote);
  create_special_form("lambda", &op_lambda);
  create_special_form("mu", &op_mu);
  create_special_form("set", &op_set);
  create_special_form("do", &op_do);
  create_special_form("and", &op_and);
  create_special_form("or", &op_or);
  create_special_form("nand", &op_nand);
  create_special_form("nor", &op_nor);

  create_builtin("!=", &fn_notequal);
  create_builtin("=", &fn_equal);
  create_builtin("<", &fn_less);
  create_builtin(">", &fn_greater);
  create_builtin("<=", &fn_lesseq);
  create_builtin(">=", &fn_greatereq);
  create_builtin("cons?", &fn_consp);
  create_builtin("sym?", &fn_symp);
  create_builtin("mint?", &fn_mintp);
  create_builtin("fun?", &fn_funp);
  create_builtin("null?", fn_nullp);

  create_builtin("+", &fn_add);
  create_builtin("-", &fn_sub);
  create_builtin("*", &fn_mul);
  create_builtin("%", &fn_mod);

  create_builtin("list", &fn_list);
  create_builtin("cons", &fn_cons);
  create_builtin("car", &fn_car);
  create_builtin("cdr", &fn_cdr);

  create_builtin("err", &fn_error);
  create_builtin("print", &fn_print);
  create_builtin("eval", &fn_eval);
  create_builtin("apply", &fn_apply);
}

obj_t
op_cond(obj_t args) {
  obj_t cond;
  while (!nullp(cond = car(args))) {
    if (!nullp(eval(cond)))
      return eval(car(cdr(args)));
    args = cdr(cdr(args));
  }
  return nil;
}

obj_t
op_nand(obj_t args) {
  return nullp(op_and(args))? t : nil;
}

obj_t
op_nor(obj_t args) {
  return nullp(op_or(args))? t : nil;
}

obj_t
op_and(obj_t args) {
  obj_t ret = t;
  while (!nullp(args)) {
    if (nullp(ret = eval(car(args))))
      return nil;
    args = cdr(args);
  }
  return ret;
}

obj_t 
op_or(obj_t args) {
  obj_t ret = nil;
  while (!nullp(args)) {
    if (!nullp(ret = eval(car(args))))
      return ret;
    args = cdr(args);
  }
  return nil;
}

obj_t
fn_eval(obj_t args) {
  obj_t ret = nil;
  while (consp(args)) {
    ret = eval(car(args));
    args = cdr(args);
  }
  return ret;
}


obj_t
dottify(obj_t args) {
  if (!consp(cdr(args))) return car(args);
  return cons(car(args), dottify(cdr(args)));
}

obj_t interpret_function(cons_t*, obj_t, bool);
obj_t
fn_apply(obj_t args) {
  obj_t fun = car(args);
  args = dottify(cdr(args));

  if (!funcp(fun)) error(E_NO_FUNCTION, fun);
  func_t *f = as_func(fun);
  switch (getftype(f)) {
  case FTYPE_COMPILED:
    return as_compiled(f)(args);
  case FTYPE_INTERP:
    return interpret_function(as_interp(f), args, false);
  default:
    error(E_INVALID_ARG, fun);
    return nil;
  }
}


void printy(obj_t);
obj_t
fn_print(obj_t args) {
  while (consp(args)) {
    printy(car(args));
    args = cdr(args);
  }
  if (!nullp(args)) printy(args);
  return nil;
}


obj_t
op_do(obj_t args) {
  obj_t ret = nil;
  while (consp(args)) {
    ret = eval(car(args));
    args = cdr(args);
  }
  return ret;
}

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
  return car(args);
}

obj_t
op_mu(obj_t args) {
  func_t *fun = malloc(sizeof(func_t));
  fun->f = make_macro(as_cons(args));
  return make_func(fun);
}

obj_t
fn_error(obj_t args) {
  error(E_RUNTIMEY, args);
  // unreachable
  return nil;
}

obj_t
op_lambda(obj_t args) {
  func_t *fun = malloc(sizeof(func_t));
  fun->f = make_interp(as_cons(args));
  return make_func(fun);
}

obj_t
fn_greatereq(obj_t args) {
  if (nullp(args)) return t;

  obj_t item = car(args);
  if (!mintp(item))
    error(E_INVALID_ARG, item);

  while (!nullp(args = cdr(args))) {
    if (!mintp(car(args)))
      error(E_INVALID_ARG, item);
    if (!(as_mint(item) >= as_mint(car(args))))
      return nil;
    item = car(args);
  }
  return t;
}

obj_t
fn_lesseq(obj_t args) {
  if (nullp(args)) return t;

  obj_t item = car(args);
  if (!mintp(item))
    error(E_INVALID_ARG, item);

  while (!nullp(args = cdr(args))) {
    if (!mintp(car(args)))
      error(E_INVALID_ARG, item);
    if (!(as_mint(item) <= as_mint(car(args))))
      return nil;
    item = car(args);
  }
  return t;
}

obj_t
fn_greater(obj_t args) {
  if (nullp(args)) return t;

  obj_t item = car(args);
  if (!mintp(item))
    error(E_INVALID_ARG, item);

  while (!nullp(args = cdr(args))) {
    if (!mintp(car(args)))
      error(E_INVALID_ARG, item);
    if (!(as_mint(item) > as_mint(car(args))))
      return nil;
    item = car(args);
  }
  return t;
}

obj_t
fn_less(obj_t args) {
  if (nullp(args)) return t;

  obj_t item = car(args);
  if (!mintp(item))
    error(E_INVALID_ARG, item);

  while (!nullp(args = cdr(args))) {
    if (!mintp(car(args)))
      error(E_INVALID_ARG, item);
    if (!(as_mint(item) < as_mint(car(args))))
      return nil;
    item = car(args);
  }
  return t;
}

obj_t
fn_notequal(obj_t args) {
  return nullp(fn_equal(args))? t : nil;
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
    if (!mintp(car(args)))
      error(E_INVALID_ARG, car(args));
    sum += as_mint(car(args));
    args = cdr(args);
  }
  return make_mint(sum);
}

obj_t
fn_sub(obj_t args) {
  if (nullp(args)) return make_mint(0);
  if (!mintp(car(args)))
    error(E_INVALID_ARG, car(args));

  if (nullp(cdr(args)))
    return make_mint(-as_mint(car(args)));

  mint_t sum = as_mint(car(args));
  do {
    args = cdr(args);
    if (!mintp(car(args)))
      error(E_INVALID_ARG, car(args));
    sum -= as_mint(car(args));
  } while(!nullp(cdr(args)));
  return make_mint(sum);
}

obj_t
fn_mul(obj_t args) {
  mint_t prod = 1;
  while (!nullp(args)) {
    if (!mintp(car(args)))
      error(E_INVALID_ARG, car(args));
    prod *= as_mint(car(args));
    args = cdr(args);
  }
  return make_mint(prod);
}

obj_t
fn_div(obj_t args) {
  mint_t prod = 1;
  while (!nullp(args)) {
    if (!mintp(car(args))) 
      error(E_INVALID_ARG, car(args));
    prod /= as_mint(car(args));
    args = cdr(args);
  }
  return make_mint(prod);
}

obj_t
fn_mod(obj_t args) {
  assert_argcount(args, 2);
  obj_t n = car(args);
  obj_t p = car(cdr(args));
  if (!mintp(n))
    error(E_INVALID_ARG, n);
  if (!mintp(p))
    error(E_INVALID_ARG, p);
  return make_mint(as_mint(n) % as_mint(p));
}

obj_t
fn_car(obj_t args) {
  assert_argcount(args, 1);
  return car(car(args));
}

obj_t
fn_cdr(obj_t args) {
  assert_argcount(args, 1);
  return cdr(car(args));
}

obj_t
fn_cons(obj_t args) {
  assert_argcount(args, 2);
  return cons(car(args), car(cdr(args)));
}

obj_t
fn_consp(obj_t args) {
  assert_argcount(args, 1);
  return consp(car(args))? t: nil;
}

obj_t
fn_funp(obj_t args) {
  assert_argcount(args, 1);
  return funcp(car(args))? t: nil;
}

obj_t
fn_mintp(obj_t args) {
  assert_argcount(args, 1);
  return mintp(car(args))? t: nil;
}

obj_t
fn_nullp(obj_t args) {
  assert_argcount(args, 1);
  return nullp(car(args))? t: nil;
}

obj_t
fn_symp(obj_t args) {
  assert_argcount(args, 1);
  return symp(car(args))? t: nil;
}

obj_t
copy_list(obj_t args) {
  if (consp(args))
    return cons(car(args), copy_list(cdr(args)));
  return args;
}

obj_t
fn_list(obj_t args) {
  return copy_list(args);
}
