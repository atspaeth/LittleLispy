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
  obj_t *val;

  struct symtentry* next;
};

struct symt {
  size_t nitems;
  struct symtentry *table[0];
};



// Return a new symtable with the given number of slots.
// We will return NULL if the number of slots isn't a power of two! 
struct symt *symt_create(size_t nitems);

// Return the place where the symtable stores a given key.
// Intended for low-level implementation.
sym_t **symt_find_ll(struct symt*, key_t);

// Return a key from the symtable.
sym_t *symt_find(struct symt*, key_t);

// Add a symbol to the symtable, 
sym_t *symt_push(struct symt*, key_t, obj_t*);

// Add a key-value pair to the symtable, replacing any existing binding.
obj_t *symt_rplac(struct symt*, key_t, obj_t*);

// Remove and return the first binding of the key from the symtable.
sym_t *symt_pop(struct symt*, key_t);



#endif // HASH_H
