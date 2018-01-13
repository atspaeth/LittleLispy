#include "hash.h"
#include <stdio.h>
#include <string.h>


typedef struct dict *dict;

int main() {
  dict d = dict_create(128);
  dict_push(d, "test", (void*)42);

  struct dictentry *it = dict_find(d, "test");
  int ret = it? (int)it->val: 0;
  printf("Got %d%s\n", ret, ret==42? "! :D": "... :(");

  return 0;
}

