#include "hash.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

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
dict_print_stats(struct dict *d) {
  if (!d) return;
  
  size_t nempty = 0;
  size_t ncolls = 0;
  for (size_t i = 0; i < d->nitems; i++) {
    struct dictentry *it = d->table[i];
    if (!it) nempty++;
    else if (it->next) {
      printf("%s collides with ", it->key);
      struct dictentry *in = it;
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



struct dict *
dict_create(size_t nitems) {
  if (nitems & (nitems - 1)) return NULL;
  struct dict*ret = malloc(sizeof(struct dict)
			   + sizeof(struct dictentry*[nitems]));
  if (ret) {
    ret->nitems = nitems;
    memset(&ret->table, 0, sizeof(struct dictentry*[nitems]));
  }
  return ret;
}


bool
dict_match(key_t k, hash_t h, struct dictentry *d) {
  return d && h == d->hash && !strcmp(k, d->key);
}

struct dictentry **
dict_find_ll(struct dict *d, key_t k) {
  if (!d) return NULL;

  hash_t h = hash(k);
  size_t idx = h & (d->nitems - 1);

  struct dictentry **ret = &d->table[idx];
  while (*ret && !dict_match(k, h, *ret))
    ret = &(*ret)->next;

  return ret;
}

struct dictentry *
dict_add_at(struct dictentry **place, key_t k, void *val) {
  if (!place) return NULL;

  struct dictentry *ret = malloc(sizeof(*ret));
  if (ret) {
    hash_t h = hash(k);
    ret->hash = h;
    ret->key = k;
    ret->val = val;
    ret->next = *place;
  }
  return *place = ret;
}

struct dictentry *
dict_push(struct dict *d, key_t k, void *val) {
  struct dictentry **it = dict_find_ll(d,k);
  return dict_add_at(it, k, val);
}

struct dictentry *
dict_find(struct dict *d, key_t k) {
  struct dictentry **ret = dict_find_ll(d,k);
  if (!ret) return NULL;
  return *ret;
}

struct dictentry *
dict_pop(struct dict *d, key_t k) {
  struct dictentry **it = dict_find_ll(d, k);
  if (!it) return NULL;

  struct dictentry *ret = *it;
  if (ret)
    *it = ret->next;
  return ret;
}

