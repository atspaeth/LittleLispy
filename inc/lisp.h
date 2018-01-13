#ifndef LISP_H
#define LISP_H


enum type {
  TYPE_CONS,
  TYPE_MINT,
  TYPE_SYM,
  NUM_TYPES
};

struct obj {
  enum type tag;
  union {
    struct cons* cons;
    int mint;
    struct sym* sym;
  };
};

struct cons {
  struct obj car;
  struct obj cdr;
};


struct sym {
  char *name;
  struct obj *val;
};



#define NIL ((struct obj*)0) 

#endif // LISP_H
