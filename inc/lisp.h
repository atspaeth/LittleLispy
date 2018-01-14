#ifndef LISP_H
#define LISP_H

typedef struct obj *symtitem_t;

typedef struct symt symt_t;
typedef struct symtentry sym_t;

enum type {
  TYPE_CONS,
  TYPE_MINT,
  TYPE_SYM,
  TYPE_CFUN,
  NUM_TYPES
};


struct cons; 
typedef struct obj *(*cfunc_t)(struct cons *arglist, symt_t *environment);

typedef struct obj {
  enum type tag;
  union {
    struct cons *cons;
    int mint;
    sym_t *sym;
    cfunc_t fun;
  };
} obj_t;

typedef struct cons {
  struct obj car;
  struct obj cdr;
} cons_t;


#include "hash.h"


obj_t *eval(obj_t *, symt_t *environment);


#endif // LISP_H
