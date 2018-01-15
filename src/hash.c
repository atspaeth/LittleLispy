#include "lisp.h"
#include "hash.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

hash_t
hash(const char *str) {
  if (!str) return 0;

  hash_t hash = 5381;
  hash_t c;
  while ((c = *str++))
    hash = hash*33 + c;
  return hash;
}


void
symt_print_stats(struct symt *d) {
  
  size_t nempty = 0;
  size_t ncolls = 0;
  for (size_t i = 0; i < d->nitems; i++) {
    struct symtentry *it = d->table[i];
    if (!it) nempty++;
    else if (it->next) {
      printf("%s collides with ", it->key);
      struct symtentry *in = it;
      while ((in = in->next)) {
	if (!in->next) {
	  if (in == it->next) printf("%s.\n", in->key);
	  else printf("and %s.\n", in->key);
	} else printf("%s ", in->key);
	ncolls++;
      }
    }
  }
  printf("There were %zu collisions and %zu empties.\n", ncolls, nempty);
}



struct symt *
symt_create(size_t nitems) {
  assert((nitems & (nitems - 1)) == 0);

  struct symt *ret = malloc(sizeof(struct symt)
			    + sizeof(sym_t*[nitems]));
  if (!ret) die();

  ret->nitems = nitems;
  memset(&ret->table, 0, sizeof(sym_t*[nitems]));
  return ret;
}


bool
symt_match(key_t k, hash_t h, sym_t *d) {
  if (!h) h = hash(k);
  return d && h == d->hash && !strcmp(k, d->key);
}

struct symtentry **
symt_find_ll(struct symt *d, key_t k) {
  assert(d);

  hash_t h = hash(k);
  size_t idx = h & (d->nitems - 1);

  struct symtentry **ret = &d->table[idx];
  while (*ret && !symt_match(k, h, *ret))
    ret = &(*ret)->next;

  return ret;
}

sym_t *
symt_add_at(sym_t **place, key_t k, obj_t val) {
  assert(place);

  sym_t *ret = malloc(sizeof(*ret));
  if (!ret) die();

  hash_t h = hash(k);
  ret->hash = h;
  ret->key = k;
  ret->val = val;
  ret->next = *place;
  return *place = ret;
}

sym_t *
symt_push(struct symt *d, key_t k, obj_t val) {
  sym_t **it = symt_find_ll(d,k);
  return symt_add_at(it, k, val);
}

obj_t
symt_rplac(struct symt *d, key_t k, obj_t val) {
  sym_t **it = symt_find_ll(d,k);
  if (*it) {
    obj_t ret = (*it)->val;
    (*it)->val = val;
    return ret;
  } else {
    symt_add_at(it, k, val);
    return nil;
  }
}

sym_t *
symt_find(struct symt *d, key_t k) {
  return *symt_find_ll(d,k);
}

sym_t *
symt_pop(struct symt *d, key_t k) {
  sym_t **it = symt_find_ll(d, k);

  sym_t *ret = *it;
  if (ret)
    *it = ret->next;
  return ret;
}

