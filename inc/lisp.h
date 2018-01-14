#ifndef LISP_H
#define LISP_H

#include <stdint.h>

enum type {
  TYPE_MINT,
  TYPE_CONS,
  TYPE_SYM,
  TYPE_CFUNC,
  NUM_TYPES
};

typedef struct symt symt_t;

typedef struct cons cons_t; 
typedef intptr_t mint_t;
typedef struct symtentry sym_t;
typedef struct obj *(*cfunc_t)(cons_t *arglist, symt_t *environment);


typedef union {
  enum type tag;
  cons_t *cons;
  mint_t mint;
  sym_t *sym;
  cfunc_t fun;
} obj_t;


typedef struct cons {
  obj_t car;
  obj_t cdr;
} cons_t;


// Check the type of an obj_t instance
static inline enum type gettype(obj_t obj) {
  return obj.tag & 0x3;}

// Cast an obj_t to the appropriate type.
static inline cons_t* as_cons(obj_t obj) {
  return (struct cons*)(((intptr_t)obj.cons) & ~0x3);}
static inline sym_t * as_sym(obj_t obj) {
  return (sym_t *)(((intptr_t)obj.cons) & ~0x3);}
static inline cfunc_t as_cfunc(obj_t obj) {
  return (cfunc_t)(((intptr_t)obj.cons) & ~0x3);}
static inline long as_mint(obj_t obj) {
  return ((intptr_t)obj.mint) >> 2;}

// Cast one of the four datatypes to obj_t.
static inline obj_t make_cons(struct cons *cons) {
  return (obj_t)(((intptr_t)cons) | TYPE_CONS);}
static inline obj_t make_sym(sym_t *sym) {
  return (obj_t)(((intptr_t)sym) | TYPE_SYM);}
static inline obj_t make_cfunc(struct cons *cons) {
  return (obj_t)(((intptr_t)cons) | TYPE_CFUNC);}
static inline obj_t make_mint(long mint) {
  return (obj_t)((mint << 2) | TYPE_MINT);}


// The absolute basics...
extern obj_t nil;
extern obj_t t;
obj_t cons(obj_t car, obj_t cdr);
obj_t car(obj_t cons);
obj_t cdr(obj_t cons);
obj_t eval(obj_t, symt_t *environment);



// General utility.
void die(void);

#endif // LISP_H
