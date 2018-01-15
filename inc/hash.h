#ifndef HASH_H
#define HASH_H

#include <stdlib.h>
#include <stdint.h>


typedef uint64_t hash_t;
typedef const char *key_t;

hash_t hash(const char *str);



struct symtentry {
  hash_t hash;
  key_t key;
  obj_t val;

  struct symtentry* next;
};

typedef struct symt {
  size_t nitems;
  struct symtentry *table[0];
} symt_t;



// Return a new symtable with the given number of slots.
// We will return NULL if the number of slots isn't a power of two! 
symt_t *symt_create(size_t nitems);

sym_t **symt_find_ll(symt_t*, key_t);
sym_t *symt_add_at(sym_t**, key_t, obj_t);

// Return the entry of a key in a symtable.
sym_t *symt_find(symt_t*, key_t);

// Add a symbol to the symtable, 
sym_t *symt_push(symt_t*, key_t, obj_t);

// Add a key-value pair to the symtable, returning the existing binding
//  if any, otherwise nil.
obj_t symt_rplac(symt_t*, key_t, obj_t);

// Remove and return the entry of a key in the symtable
sym_t *symt_pop(symt_t*, key_t);



#endif // HASH_H
