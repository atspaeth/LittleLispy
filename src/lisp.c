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
  if (consp(sym->val) || nullp(sym->val))
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
  while (!nullp(args)) {
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

  if (nullp(cdr(cdr(args)))) return val;
  else return op_setf(cdr(cdr(args)));
}

int main() {
  symtable = symt_create(128);
  nil = make_sym(make_self_evaluating("nil"));
  t = make_sym(make_self_evaluating("t"));
  func_t plusfun = {make_compiled(&op_plus)};
  obj_t plus = make_sym(bind_name("+", make_func(&plusfun)));

  obj_t x = eval(cons(plus,
		      cons(make_mint(2), cons(make_mint(2), nil))));
  if (mintp(x))
    printf("2 + 2 = %ld\n", as_mint(x));
  else
    puts("x was not a number...");

  return 0;
}

void die() {
  perror(NULL);
  abort();
}

