#ifndef HASH_H
#define HASH_H

#include <stdlib.h>
#include <stdint.h>


typedef uint64_t hash_t;
typedef const char *key_t;

hash_t hash(const char *str);



struct dictentry {
  hash_t hash;
  key_t key;
  void *val;

  struct dictentry* next;
};

struct dict {
  size_t nitems;
  struct dictentry *table[0];
};



// Return a new dictionary with the given number of slots.
// We will return NULL if the number of slots isn't a power of two! 
struct dict *dict_create(size_t nitems);

// Return a key from the dictionary.
struct dictentry *dict_find(struct dict*, key_t);

// Add a key-value pair to the dictionary, shadowing any existing binding.
struct dictentry *dict_push(struct dict*, key_t, void*);

// Add a key-value pair to the dictionary, replacing any existing binding.
struct dictentry *dict_rplac(struct dict*, key_t, void*);

// Remove and return the first binding of the key from the dictionary.
struct dictentry *dict_pop(struct dict*, key_t);



#endif // HASH_H
