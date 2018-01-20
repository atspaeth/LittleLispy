#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include <setjmp.h>
#include "lisp.h"
#include "hash.h"
#include "builtins.h"

// nil will be redefined in init code, but some of that code depends
//  on nil having some (any) value; the mint 0 has been chosen arbitrarily
obj_t nil=(obj_t)0L, t;
symt_t *symtable;

obj_t 
car(obj_t cons) {
  if (gettype(cons) == TYPE_CONS)
    return as_cons(cons)->car;
  else return nil;
}

obj_t 
cdr(obj_t cons) {
  if (gettype(cons) == TYPE_CONS)
    return as_cons(cons)->cdr;
  else return nil;
}

obj_t 
sym_value(sym_t *sym) {
  obj_t val = sym->val;
  if (consp(val))
    return car(val);

  if (nullp(val) && !nullp(make_sym(sym)))
    error(E_UNDECLARED, make_sym(sym));

  return val;
}

obj_t
name_value(key_t name) {
  return sym_value(intern_name(name));
}

obj_t 
cons(obj_t car, obj_t cdr) {
  cons_t *cons = malloc(sizeof(cons_t));
  if (!cons) die();

  cons->car = car;
  cons->cdr = cdr;
  return make_cons(cons);
}

sym_t *
bind_sym(sym_t *sym, obj_t val) {

  // prevent assigning to nil
  if (nullp(make_sym(sym))) return as_sym(nil);

  // prevent assigning to constants
  if (consp(sym->val) || nullp(sym->val))
    sym->val = cons(val, sym->val);

  return sym;
}

sym_t *
unbind_sym(sym_t *sym) {
  if (consp(sym->val))
    sym->val = cdr(sym->val);
  return sym;
}

sym_t *
intern_name(key_t name) {
  sym_t **place = symt_find_ll(symtable, name);
  if (!*place) symt_add_at(place, name, nil);
  return *place;
}

sym_t *
bind_name(key_t name, obj_t val) {
  return bind_sym(intern_name(name), val);
}

sym_t *
make_const(key_t name, obj_t val) {
  sym_t *ret = intern_name(name);
  ret->val = val;
  return ret;
}

sym_t *
make_self_evaluating(key_t name) {
  sym_t *ret = intern_name(name);
  ret->val = make_sym(ret);
  return ret;
}

void
unbind_list(obj_t names) {
  while (consp(names)) {
    if (symp(car(names)))
      unbind_sym(as_sym(car(names)));
    names = cdr(names);
  }
  if (!nullp(names))
    unbind_sym(as_sym(names));
}

obj_t
eval_list(obj_t list) {
  if (!consp(list))
    return nil;
  else
    return cons(eval(car(list)), eval_list(cdr(list)));
}

void
bind_list(obj_t names, obj_t args, bool eval_first) {
  size_t argcount = 0;
  while (consp(names) && consp(args)) {
    obj_t name = car(names);
    obj_t arg = car(args);
    if (symp(name))
      bind_sym(as_sym(name),
	       eval_first? eval(arg) : arg);
    names = cdr(names);
    args = cdr(args);
    argcount ++;
  }

  if (consp(args) && symp(names) && !nullp(names)) {
    bind_sym(as_sym(names),
	     eval_first? eval_list(args) : args);

  }

  if (consp(args) || consp(names)) {
    while (consp(args)) {
	argcount ++;
	args = cdr(args);
    }

    if (consp(names)) {
	unbind_list(names);
	error(E_WRONG_ARGCOUNT, make_mint(argcount));
    }
  }
}

obj_t
interpret_function(cons_t *lam, obj_t args, bool eval_first) {
  obj_t parlist = lam->car;
  obj_t body = lam->cdr;

  bind_list(parlist, args, eval_first);

  obj_t ret = nil;
  do {
    ret = eval(car(body));
    body = cdr(body);
  } while (consp(body));

  unbind_list(parlist);

  return ret;
}

obj_t
funcall(obj_t it, obj_t args) {
  if (!funcp(it)) error(E_NO_FUNCTION, it);

  func_t *f = as_func(it);
  
  switch (getftype(f)) {
  case FTYPE_COMPILED: {
    return as_compiled(f)(eval_list(args));
  } case FTYPE_INTERP:
    return interpret_function(as_interp(f), args, true);
  case FTYPE_SPECIAL:
    return as_compiled(f)(args);
  case FTYPE_MACRO:
    return eval(interpret_function(as_interp(f), args, false));
  }
}

obj_t
eval(obj_t it) {
  switch (gettype(it)) {
  case TYPE_SYM:
    return sym_value(as_sym(it));
  case TYPE_MINT:
  case TYPE_FUNC:
    return it;
  case TYPE_CONS:
    return funcall(eval(as_cons(it)->car), as_cons(it)->cdr);
  }
}

obj_t
printy(obj_t arg) {
  switch (gettype(arg)) {
  case TYPE_MINT:
    printf("%ld", as_mint(arg)); break;
  case TYPE_SYM:
    printf("%s", as_sym(arg)->key); break;
  case TYPE_FUNC:
    printf("<function>"); break;
  case TYPE_CONS:
    putchar('(');
    printy(car(arg));
    while (consp(arg = cdr(arg))) {
      putchar(' ');
      printy(car(arg));
    }
    if (!nullp(arg)) {
      printf(" . ");
      printy(arg);
    }
    putchar(')');
  }
  return nil;
}

obj_t read(FILE *in);

obj_t
read_cons(FILE *in) {
  int c;
  while (isspace(c = fgetc(in)));
  if (c == ')') return nil;
  if (c == '.') {
    obj_t ret = read(in);
    while (isspace(c = fgetc(in)));
    if (c != ')')
      error(E_READ_ERROR, nil);
    return ret;
  } else {
    ungetc(c, in);
    return cons(read(in), read_cons(in));
  }
}

char *
gets_until(FILE *in, bool(*pred)(char)) {
  int c;
  size_t size=8, len=0;
  char *buf = malloc(size);

  while ((c = fgetc(in)) != EOF && !pred(buf[len] = c))
    if (++len == size) {
      buf = realloc(buf, size *= 2);
      if (!buf) return NULL;
    }
  buf[len] = 0;
  if (c != EOF) ungetc(c, in);

  return buf;
}


bool
is_terminating(char c) {
  return c=='(' || c==')' || c==',' || isspace(c);
}

obj_t
read_mint(char *str) {
  char *endptr = str;
  mint_t x = strtol(str, &endptr, 0);
  if (!*endptr) return make_mint(x);
  else return nil;
}

obj_t
read(FILE *in) {
  int c;

  while (isspace(c = fgetc(in)));

  if (c == EOF) error(E_END_OF_FILE, nil);
  if (c == ')') error(E_READ_ERROR, nil);
  if (c == '(') return read_cons(in);
  if (c == '\'')
    return cons(make_sym(intern_name("quote")), cons(read(in), nil));

  ungetc(c, in);
  char *tok = gets_until(in, is_terminating);
  obj_t it = read_mint(tok);
  if (!nullp(it)) return it;
  else return make_sym(intern_name(tok));
}


jmp_buf errhandler;
obj_t errobj;

int main() {
  symtable = symt_create(128);
  nil = make_sym(make_self_evaluating("nil"));
  t = make_sym(make_self_evaluating("t"));

  setup_builtins();

  int ecode = setjmp(errhandler);
  if (ecode == 0 || ecode == E_TRY_AGAIN) {
    while(1) {
      printy(eval(read(stdin)));
      putchar('\n');
    }
  } else switch(ecode) {
    case E_READ_ERROR:
      puts("* READ ERROR");
      longjmp(errhandler, E_TRY_AGAIN);
    case E_END_OF_FILE:
      exit(0);
    case E_NO_FUNCTION:
      printf("* UNDEFINED FUNCTION: ");
      printy(errobj);
      putchar('\n');
      longjmp(errhandler, E_TRY_AGAIN);
    case E_UNDECLARED:
      printf("* UNDECLARED VARIABLE: ");
      printy(errobj);
      putchar('\n');
      longjmp(errhandler, E_TRY_AGAIN);
    case E_INVALID_ARG:
      printf("* INVALID ARGUMENT: ");
      printy(errobj);
      putchar('\n');
      longjmp(errhandler, E_TRY_AGAIN);
    case E_WRONG_ARGCOUNT:
      printf("* WRONG NUMBER OF ARGUMENTS: ");
      printy(errobj);
      putchar('\n');
      longjmp(errhandler, E_TRY_AGAIN);
    case E_RUNTIMEY:
      printf("* RUNTIME ERROR: ");
      printy(errobj);
      putchar('\n');
      longjmp(errhandler, E_TRY_AGAIN);
    default:
      puts("* BUG! ILLEGAL ERROR");
      exit(1);
    }

  exit(1);
}

void
error(error_t ecode, obj_t eobj) {
  errobj = eobj;
  longjmp(errhandler, ecode);
}

void
die() {
  perror(NULL);
  abort();
}

