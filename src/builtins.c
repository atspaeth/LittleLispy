#include "builtins.h"
#include "hash.h"
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
  create_special_form("quote", &op_quote);
  create_special_form("quasiquote", &op_quasiquote);
  create_special_form("lambda", &op_lambda);
  create_special_form("mu", &op_mu);
  create_special_form("set", &op_set);
  create_special_form("def", &op_def);
  create_special_form("do", &op_do);
  create_special_form("and", &op_and);
  create_special_form("or", &op_or);

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
  create_builtin("printnl", &fn_printnl);
  create_builtin("eval", &fn_eval);
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


void printy(obj_t);
int putchar(int);
obj_t
fn_print(obj_t args) {
  obj_t ret = nil;
  while (consp(args)) {
    printy(ret = car(args));
    putchar(' ');
    args = cdr(args);
  }
  if (!nullp(args)) printy(args);
  return ret;
}

obj_t
fn_printnl(obj_t args) {
  obj_t ret = fn_print(args);
  putchar('\n');
  return ret;
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

obj_t eval_list(obj_t);

obj_t
nappend(obj_t a, obj_t b) {
  if (consp(a) && !nullp(b)) {
    if (nullp(cdr(a)))
      as_cons(a)->cdr = b;
    else nappend(cdr(a), b);
  }
  return a;
}

obj_t
op_quasiquote(obj_t args) {
  if (!consp(args)) return args;

  if (eqp(car(args), unquote))
    return eval(cdr(args));

  if (eqp(car(car(cdr(args))), unquote_splice)) {
    return cons(op_quasiquote(car(args)),
		nappend(eval(cdr(car(cdr(args)))),
			op_quasiquote(cdr(cdr(args)))));
  }
		
		

  return cons(op_quasiquote(car(args)),
	      op_quasiquote(cdr(args)));
}

obj_t
op_quote(obj_t args) {
  return args;
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

// defines a constant
obj_t
op_def(obj_t args) {
  // must have an even number of arguments
  if (!listp(args)) return args;
  if (!listp(cdr(args)))
    error(E_FAILED_BIND, cons(cons(car(args), nil), cdr(args)));
  
  obj_t name = car(args);
  obj_t val = eval(car(cdr(args)));

  if (!symp(name))
    error(E_INVALID_NAME, name);
  if (nullp(name))
    error(E_REDEFINE, name);

  sym_t *sym = as_sym(name);

  if (!nullp(sym->val))
    error(E_REDEFINE, name);
  else
    sym->val = val;

  if (nullp(cdr(cdr(args)))) return name;
  else return op_def(cdr(cdr(args)));
}

// mutates an already-extant variable, or creates a mutable variable
obj_t
op_set(obj_t args) {
  // must have an even number of arguments
  if (!listp(args)) return nil;
  if (!listp(cdr(args)))
    error(E_FAILED_BIND, cons(cons(car(args), nil), cdr(args)));
  
  obj_t name = car(args);
  obj_t val = eval(car(cdr(args)));

  if (!symp(name))
    error(E_INVALID_NAME, name);
  if (nullp(name))
    error(E_REDEFINE, name);

  sym_t *sym = as_sym(name);

  if (nullp(sym->val))
    sym->val = cons(val, nil);
  else if (consp(sym->val))
    as_cons(sym->val)->car = val;
  else
    error(E_REDEFINE, name);

  if (nullp(cdr(cdr(args)))) return name;
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
