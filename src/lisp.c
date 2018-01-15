#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "lisp.h"
#include "hash.h"
#include "builtins.h"

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
name_value(key_t name) {
  return sym_value(intern_name(name));
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

  // prevent assigning to nil
  if (nullp(make_sym(sym))) return as_sym(nil);

  // prevent assigning to constants
  if (consp(sym->val) || nullp(sym->val))
    sym->val = cons(val, sym->val);

  return sym;
}

sym_t *
unbind_sym(sym_t *sym) {
  if (consp(sym->val))
    sym->val = cdr(sym->val);
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
make_const(key_t name, obj_t val) {
  sym_t *ret = intern_name(name);
  ret->val = val;
  return ret;
}

sym_t *
make_self_evaluating(key_t name) {
  sym_t *ret = intern_name(name);
  ret->val = make_sym(ret);
  return ret;
}

void
bind_list(obj_t names, obj_t args) {
  while (consp(names) && consp(args)) {
    if (symp(car(names)))
      bind_sym(as_sym(car(names)), car(args));
    names = cdr(names);
    args = cdr(args);
  }
}

void
unbind_list(obj_t names) {
  while (consp(names)) {
    if (symp(car(names)))
      unbind_sym(as_sym(car(names)));
    names = cdr(names);
  }
}

obj_t
interpret_function(cons_t *lam, obj_t args, bool eval_first) {
  obj_t parlist = lam->car;
  obj_t body = lam->cdr;

  bind_list(parlist, args);

  obj_t ret = eval(body);

  unbind_list(parlist);

  return ret;
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
    if (!nullp(arg)) {
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

  func_t plusfun = {make_compiled(&fn_add)};
  obj_t plus = make_sym(make_const("+", make_func(&plusfun)));

  func_t iffun = {make_special(&op_if)};
  obj_t iff = make_sym(make_const("if", make_func(&iffun)));

  func_t equalfun = {make_compiled(&fn_equal)};
  obj_t equal = make_sym(make_const("=", make_func(&equalfun)));

  func_t setfun = {make_special(&op_set)};
  obj_t set = make_sym(make_const("set", make_func(&setfun)));

  obj_t x = make_sym(intern_name("x"));
  
  obj_t asg = cons(set, cons(x, cons(make_mint(42), nil)));
  obj_t code = cons(iff,
		    cons(cons(equal,
			      cons(cons(plus,
					cons(x,
					     cons(make_mint(-38), nil))),
				   cons(make_mint(4), nil))),
			 cons(cons(set,
				   cons(x,
					cons(cons(plus,
						  cons(x,
						       cons(make_mint(-1),
							    nil))), nil))),
			      cons(cons(set,
					cons(x,
					     cons(make_mint(-1), nil))),
				   nil))));
  printy(asg);
  putchar('\n');
  printy(code);
  putchar('\n');

  eval(asg);
  printy(eval(code));
  putchar('\n');

  return 0;
}

void error(const char *err) {
  fputs(err, stderr);
  abort();
}

void die() {
  perror(NULL);
  abort();
}

