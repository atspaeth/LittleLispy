#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "lisp.h"
#include "hash.h"

obj_t nil, t;
symt_t *symtable;

obj_t 
car(obj_t cons) {
  if (gettype(cons) == TYPE_CONS && as_cons(cons))
    return as_cons(cons)->car;
  else return nil;
}

obj_t 
cdr(obj_t cons) {
  if (gettype(cons) == TYPE_CONS && as_cons(cons))
    return as_cons(cons)->cdr;
  else return nil;
}

obj_t 
sym_value(sym_t *sym) {
  if (sym) {
    if (gettype(sym->val) == TYPE_CONS) return car(sym->val);
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
bind_symbol(key_t name, obj_t val) {
  obj_t cell = cons(val, nil);
  obj_t old = symt_rplac(symtable, name, cell);
  as_cons(cell)->cdr = old;
  return symt_find(symtable, name);
}

sym_t *
make_self_evaluating(key_t name) {
  sym_t **place = symt_find_ll(symtable, name);
  if (!*place) symt_add_at(place, name, make_mint(0));
  (*place)->val = make_sym(*place);
  return *place;
}

obj_t
funcall(obj_t it, obj_t args) {
  switch(gettype(it)) {
  case TYPE_MINT:
    return nil; // TODO: implement errors
  case TYPE_SYM:
    // NYI: call the symbol's function
    return nil;
  case TYPE_CFUNC:
    return as_cfunc(it)(args);
  case TYPE_CONS:
    // NYI treat it as a lambda expression
    return nil;
  }
}

obj_t
eval(obj_t it) {
  switch(gettype(it)) {
  case TYPE_SYM:
    return sym_value(as_sym(it));
  case TYPE_MINT:
  case TYPE_CFUNC:
    return it;
  case TYPE_CONS:
    return funcall(as_cons(it)->car, as_cons(it)->cdr);
  }
}

obj_t
plus(obj_t args) {
  mint_t sum = 0;
  while (as_sym(args) != as_sym(nil)) {
    if (!mintp(car(args))) return nil;
    sum += as_mint(car(args));
    args = cdr(args);
  }
  return make_mint(sum);
}

int main() {
  symtable = symt_create(128);
  nil = make_sym(make_self_evaluating("nil"));
  t = make_sym(make_self_evaluating("t"));

  obj_t x = eval(cons(make_cfunc(plus),
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

