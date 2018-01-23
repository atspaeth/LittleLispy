#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include "lisp.h"
#include "hash.h"

#define INITIAL_STORE_SIZE 512
#define FREELIST_ENTRY_BITS (CHAR_BIT * sizeof(*free_list))
size_t store_size = 0;
cons_t *free_store = NULL;
size_t *free_list = NULL;


extern symt_t *symtable;


bool
already_used(size_t idx) {
  size_t entry = idx / FREELIST_ENTRY_BITS;
  size_t bit = idx % FREELIST_ENTRY_BITS;
  return (free_list[entry] & (1L << bit)) != 0;
}

void
mark_used(size_t idx) {
  size_t entry = idx / FREELIST_ENTRY_BITS;
  size_t bit = idx % FREELIST_ENTRY_BITS;
  free_list[entry] |= 1L << bit;
}

void
allocate_store(size_t ncells) {
  free_store = realloc(free_store, sizeof(cons_t) * ncells);
  free_list = realloc(free_list, ncells / CHAR_BIT);
  bzero(free_list + store_size,
	(ncells - store_size) / CHAR_BIT);
  store_size = ncells;
}


cons_t *
find_next_free_cons() {
  
  if (!free_store) {
    allocate_store(INITIAL_STORE_SIZE);
    mark_used(0);
    return &free_store[0];
  }

  for (size_t entry = 0; entry < store_size / FREELIST_ENTRY_BITS; entry++) {
    if (~entry) {
      for (size_t bit = 0; bit < FREELIST_ENTRY_BITS; bit++) {
	if ((free_list[entry] & (1L<<bit)) == 0) {
	  free_list[entry] |= (1L << bit);
	  return &free_store[entry*FREELIST_ENTRY_BITS + bit];
	}
      }
    }
  }
  return NULL;
}

void
mark_list(obj_t list) {
  while (consp(list)) {
    if (consp(car(list)))
      mark_list(car(list));

    size_t idx = as_cons(list) - free_store;
    if (already_used(idx)) return;
    mark_used(idx);

    list = cdr(list);
  }
}

void
mark_sym_and_children(sym_t *sym) {
  if (!sym) return;
  
  mark_list(sym->val);

  mark_sym_and_children(sym->next);
}

cons_t *
garbage_collect_and_find() {
  size_t *old_list = free_list;
  free_list = malloc(store_size / CHAR_BIT);
  bzero(free_list, store_size / CHAR_BIT);

  for (size_t n = 0; n < symtable->nitems; n++) {
    mark_sym_and_children(symtable->table[n]);
  }

  for (size_t entry = 0; entry < store_size / FREELIST_ENTRY_BITS; entry++) {
    if (old_list[entry] != free_list[entry])
      for (size_t bit = 0; bit < FREELIST_ENTRY_BITS; bit++) {
	bool old = old_list[entry] & (1L<<bit);
	bool new = free_list[entry] & (1L<<bit);
	if (old && !new)
	  printf("Freed cell %lu!\n", entry*FREELIST_ENTRY_BITS + bit);
	else if (new && !old)
	  printf("Wait, wha? %lu\n", entry*FREELIST_ENTRY_BITS + bit);
      }
  }

  free(old_list);
  return NULL;
}

cons_t *
expand_store_and_find() {

  size_t old_store_size = store_size;
  allocate_store(old_store_size * 2);

  if (!free_store) die();

  mark_used(old_store_size);
  return &free_store[old_store_size];
}


obj_t
cons(obj_t car, obj_t cdr) {

  cons_t *cons = find_next_free_cons();

  if (!cons) {
    cons = garbage_collect_and_find();

    if (!cons) {
      cons = expand_store_and_find();
    }
  }

  cons->car = car;
  cons->cdr = cdr;
  return make_cons(cons);
}
