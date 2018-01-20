#ifndef LISP_H
#define LISP_H

#include <stdint.h>
#include <stdbool.h>

enum type {
  TYPE_MINT=0,
  TYPE_CONS,
  TYPE_SYM,
  TYPE_FUNC,
};

// There are four basic datatypes; since I don't actually have any
//  32-bit platforms to run on, this could be increased to eight,
//  but I would need to choose at least three more first.
// CONS is the same as in any Lisp ever
// MINT is a machine integer, a 30-bit (because of tags) fixnum.
// SYM is a symbol type, pointing to a symtable entry.
// FUNC points to the function object defined below.
typedef struct cons cons_t; 
typedef intptr_t mint_t;
typedef struct symtentry sym_t;
typedef struct func func_t;

typedef union obj {
  intptr_t tag;
  uintptr_t _bits;
  cons_t *cons;
  mint_t mint;
  sym_t *sym;
  func_t *func;
} obj_t;

typedef struct cons {
  obj_t car;
  obj_t cdr;
} cons_t;



// There are four types of function-like objects, conveniently
//  corresponding to the two available tag bits.
// COMPILED is a regular compiled C function.
// INTERP is an interpreted function, such as the return value of
//   the (lambda) special form.
// MACRO means that the parameters should not be evaluated before
//   the call, but the return value should be.
// SPECIAL is a compiled function whose parameters aren't evaled.
enum ftype {
  FTYPE_COMPILED=0,
  FTYPE_INTERP,
  FTYPE_SPECIAL,
  FTYPE_MACRO,
};

typedef union funcptr {
  intptr_t tag;
  obj_t(*compiled)(obj_t);
  cons_t *interp;
} funcptr_t;

// A func is a structure despite currently only containing
//  one function pointer because TODO: add closures! :D
typedef struct func {
  funcptr_t f;
} func_t;


// Check the type of a func_t instance
static inline enum ftype getftype(func_t *func) {
  return func->f.tag & 0x3;}
static inline obj_t(*as_compiled(func_t *func))(obj_t) {
  return (obj_t(*)(obj_t))((intptr_t)func->f.compiled & ~0x3);}
static inline cons_t *as_interp(func_t *func) {
  return (cons_t*)((intptr_t)func->f.interp & ~0x3);}

// Create the union part of a func_t instance.
static inline funcptr_t make_compiled(obj_t(*comp)(obj_t)) {
  return (funcptr_t)((intptr_t)comp | FTYPE_COMPILED);}
static inline funcptr_t make_special(obj_t(*spec)(obj_t)) {
  return (funcptr_t)((intptr_t)spec | FTYPE_SPECIAL);}
static inline funcptr_t make_macro(cons_t *mac) {
  return (funcptr_t)((intptr_t)mac | FTYPE_MACRO);}
static inline funcptr_t make_interp(cons_t *interp) {
  return (funcptr_t)((intptr_t)interp | FTYPE_INTERP);}

// Check the type of an obj_t instance
static inline enum type gettype(obj_t obj) {
  return obj.tag & 0x3;}
static inline bool consp(obj_t obj) {
  return gettype(obj) == TYPE_CONS;}
static inline bool mintp(obj_t obj) {
  return gettype(obj) == TYPE_MINT;}
static inline bool funcp(obj_t obj) {
  return gettype(obj) == TYPE_FUNC;}
static inline bool symp(obj_t obj) {
  return gettype(obj) == TYPE_SYM;}

// Cast an obj_t to the appropriate type.
static inline cons_t *as_cons(obj_t obj) {
  return (struct cons*)((intptr_t)obj.cons & ~0x3);}
static inline sym_t *as_sym(obj_t obj) {
  return (sym_t*)((intptr_t)obj.sym & ~0x3);}
static inline func_t *as_func(obj_t obj) {
  return (func_t*)((intptr_t)obj.func & ~0x3);}
static inline long as_mint(obj_t obj) {
  return obj.mint >> 2;}

// Cast one of the four datatypes to obj_t.
static inline obj_t make_cons(struct cons *cons) {
  return (obj_t)(((intptr_t)cons) | TYPE_CONS);}
static inline obj_t make_sym(sym_t *sym) {
  return (obj_t)(((intptr_t)sym) | TYPE_SYM);}
static inline obj_t make_func(func_t *func) {
  return (obj_t)(((intptr_t)func) | TYPE_FUNC);}
static inline obj_t make_mint(long mint) {
  return (obj_t)((mint << 2) | TYPE_MINT);}

// Predicates.
extern obj_t nil;
static inline bool eqp(obj_t a, obj_t b) {
  return a._bits == b._bits;}
static inline bool nullp(obj_t a) {
  return a._bits == nil._bits;}

obj_t cons(obj_t car, obj_t cdr);
obj_t car(obj_t cons);
obj_t cdr(obj_t cons);
obj_t eval(obj_t);


// Manipulate the symbol table.
sym_t *bind_sym(sym_t *sym, obj_t val);
sym_t *unbind_sym(sym_t *sym);
sym_t *make_const(const char *name, obj_t val);
obj_t name_value(const char *name);
obj_t sym_value(sym_t *sym);
sym_t *intern_name(const char *name);



// General utility.
typedef enum {
  E_TRY_AGAIN=1,
  E_UNDECLARED,
  E_NO_FUNCTION,
  E_END_OF_FILE,
  E_READ_ERROR,
  E_WRONG_ARGCOUNT,
  E_INVALID_ARG,
  E_RUNTIMEY,
} error_t;

void error(error_t, obj_t);

void die(void);

#endif // LISP_H
