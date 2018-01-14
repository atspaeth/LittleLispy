#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "lisp.h"
#include "hash.h"

obj_t *nil;
symt_t *symtable;


obj_t *
car(obj_t *cons) {
  if (cons && cons->tag == TYPE_CONS && cons->cons)
    return &cons->cons->car;
  else return nil;
}

obj_t *
cdr(obj_t *cons) {
  if (cons && cons->tag == TYPE_CONS && cons->cons)
    return &cons->cons->cdr;
  else return nil;
}

obj_t *
sym_value(sym_t *sym) {
  if (sym)
    return car(sym->val);
  else return nil;
}


// Creates a new symbol with a given value.
// If the provided value is NULL (not nil),
//  creates a self-evaluating symbol by making
//  it point to itself.
sym_t *
make_symbol(key_t name, obj_t *val) {
  sym_t **place = symt_find_ll(symtable, name);
}


int main() {
  symtable = symt_create(128);

  return 0;
}

void die() {
  perror(NULL);
  abort();
}

