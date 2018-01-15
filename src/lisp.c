#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "lisp.h"
#include "hash.h"

// nil will be redefined in init code, but some of that code depends
//  on nil having some (any) value; the mint 0 has been chosen arbitrarily
obj_t nil=(obj_t)0L, t;
symt_t *symtable;

obj_t 
car(obj_t cons) {
  if (gettype(cons) == TYPE_CONS)
    return as_cons(cons)->car;
  else return nil;
}

obj_t 
cdr(obj_t cons) {
  if (gettype(cons) == TYPE_CONS)
    return as_cons(cons)->cdr;
  else return nil;
}

obj_t 
sym_value(sym_t *sym) {
  if (sym) {
    if (consp(sym->val))
      return car(sym->val);
    else return sym->val;
  } else return nil;
}

obj_t 
cons(obj_t car, obj_t cdr) {
  cons_t *cons = malloc(sizeof(cons_t));
  if (!cons) die();

  cons->car = car;
  cons->cdr = cdr;
  return make_cons(cons);
}

sym_t *
bind_sym(sym_t *sym, obj_t val) {
  if (consp(sym->val) || eqp(sym->val, nil))
    sym->val = cons(val, sym->val);
  // silently fails if constant. Should it error?
  return sym;
}

sym_t *
intern_name(key_t name) {
  sym_t **place = symt_find_ll(symtable, name);
  if (!*place) symt_add_at(place, name, nil);
  return *place;
}

sym_t *
bind_name(key_t name, obj_t val) {
  return bind_sym(intern_name(name), val);
}

sym_t *
make_self_evaluating(key_t name) {
  sym_t *ret = intern_name(name);
  ret->val = make_sym(ret);
  return ret;
}

obj_t
interpret_function(cons_t *lam, obj_t args, bool eval_first) {
  // TODO: bind each argument in turn and eval the lambda list,
  //   then pop all the newly-created bindings.
  obj_t parlist = lam->car;
  obj_t body = lam->cdr;
  return nil;
}

obj_t
eval_list(obj_t list) {
  if (!consp(list))
    return nil;
  else
    return cons(eval(car(list)), eval_list(cdr(list)));
}

obj_t
funcall(obj_t it, obj_t args) {
  if (!funcp(it)) return nil; // TODO: implement errors!

  func_t *f = as_func(it);
  
  switch (getftype(f)) {
  case FTYPE_COMPILED: {
    return as_compiled(f)(eval_list(args));
  } case FTYPE_INTERP:
    return interpret_function(as_interp(f), args, true);
  case FTYPE_SPECIAL:
    return as_compiled(f)(args);
  case FTYPE_MACRO:
    return eval(interpret_function(as_interp(f), args, false));
  }
}

obj_t
eval(obj_t it) {
  switch (gettype(it)) {
  case TYPE_SYM:
    return sym_value(as_sym(it));
  case TYPE_MINT:
  case TYPE_FUNC:
    return it;
  case TYPE_CONS:
    return funcall(eval(as_cons(it)->car), as_cons(it)->cdr);
  }
}

obj_t
op_plus(obj_t args) {
  mint_t sum = 0;
  while (!eqp(args, nil)) {
    if (!mintp(car(args))) return nil;
    sum += as_mint(car(args));
    args = cdr(args);
  }
  return make_mint(sum);
}

obj_t
op_setf(obj_t args) {
  obj_t name = car(args);
  obj_t val = car(cdr(args));

  // do the assignment

  if (eqp(cdr(cdr(args)), nil)) return val;
  else return op_setf(cdr(cdr(args)));
}

obj_t
op_equal(obj_t args) {
  obj_t item = car(args);
  while (!eqp(args = cdr(args), nil)) {
    if (!eqp(car(args), item))
      return nil;
  }
  return t;
}

obj_t
op_if(obj_t args) {
  obj_t cond = car(args);
  obj_t thn = car(cdr(args));
  obj_t els = car(cdr(cdr(args)));
  if (!eqp(eval(cond), nil))
    return eval(thn);
  else
    return eval(els);
}

obj_t
printy(obj_t arg) {
  switch (gettype(arg)) {
  case TYPE_MINT:
    printf("%ld", as_mint(arg)); break;
  case TYPE_SYM:
    printf("%s", as_sym(arg)->key); break;
  case TYPE_FUNC:
    printf("<function>"); break;
  case TYPE_CONS:
    putchar('(');
    printy(car(arg));
    while (consp(arg = cdr(arg))) {
      putchar(' ');
      printy(car(arg));
    }
    if (!eqp(arg, nil)) {
      printf(" . ");
      printy(arg);
    }
    putchar(')');
  }
  return nil;
}

int main() {
  symtable = symt_create(128);
  nil = make_sym(make_self_evaluating("nil"));
  t = make_sym(make_self_evaluating("t"));
  func_t plusfun = {make_compiled(&op_plus)};
  obj_t plus = make_sym(bind_name("+", make_func(&plusfun)));
  func_t iffun = {make_special(&op_if)};
  obj_t iff = make_sym(bind_name("if", make_func(&iffun)));
  func_t equalfun = {make_compiled(&op_equal)};
  obj_t equal = make_sym(bind_name("=", make_func(&equalfun)));
  

  obj_t code = cons(iff,
		    cons(cons(equal,
			      cons(cons(plus,
					cons(make_mint(2),
					     cons(make_mint(2), nil))),
				   cons(make_mint(5), nil))),
			 cons(make_mint(42),
			      cons(make_mint(-1), nil))));
  printy(code);
  putchar('\n');
  printy(eval(code));

  return 0;
}

void die() {
  perror(NULL);
  abort();
}

