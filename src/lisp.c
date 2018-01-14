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


int main() {
  symtable = symt_create(128);
  nil = make_sym(make_self_evaluating("nil"));
  t = make_sym(make_self_evaluating("t"));

  bool tworks = sym_value(symt_find(symtable, "t")).sym == t.sym;
  bool nilworks = sym_value(symt_find(symtable, "nil")).sym == nil.sym;

  if (tworks) puts("T works!");
  if (nilworks) puts("Nil works!");

  return 0;
}

void die() {
  perror(NULL);
  abort();
}

