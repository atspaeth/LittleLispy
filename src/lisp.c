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
obj_t quote, quasiquote, unquote, unquote_splice;
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

sym_t *
bind_sym(sym_t *sym, obj_t val) {

  // prevent assigning to nil or constants
  if (nullp(make_sym(sym)) || !listp(sym->val))
    error(E_REDEFINE, make_sym(sym));

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
    return eval(list);
  else
    return cons(eval(car(list)), eval_list(cdr(list)));
}

obj_t binderrobj;

// generates a binding list to be walked by bind_list
error_t
generate_binding_list(obj_t names, obj_t args, obj_t *out) {
  switch (gettype(names)) {
  case TYPE_MINT:
  case TYPE_FUNC:
    binderrobj = names;
    return E_INVALID_NAME;
  case TYPE_SYM:
    if (nullp(names)) {
      return nullp(args) ? E_ALL_OKAY : E_FAILED_BIND;}
    *out = cons(cons(names, args), *out);
    return E_ALL_OKAY;
  case TYPE_CONS: {
    if (!consp(args))
      return E_FAILED_BIND;
    error_t ret = generate_binding_list(car(names), car(args), out);
    return ret? ret: generate_binding_list(cdr(names), cdr(args), out);
  }}
}

void
bind_list(obj_t names, obj_t args) {
  obj_t bindings = nil;
  error_t err = generate_binding_list(names, args, &bindings);

  if (err == E_FAILED_BIND) binderrobj = cons(names, args);
  if (err) error(err, binderrobj);

  while (!nullp(bindings)) {
    obj_t name = car(car(bindings));
    obj_t val = cdr(car(bindings));

    bind_sym(as_sym(name), val);

    bindings = cdr(bindings);
  }
}


obj_t
interpret_function(cons_t *lam, obj_t args) {
  obj_t parlist = lam->car;
  obj_t body = lam->cdr;

  bind_list(parlist, args);

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
    return interpret_function(as_interp(f), eval_list(args));
  case FTYPE_SPECIAL:
    return as_compiled(f)(args);
  case FTYPE_MACRO:
    return eval(interpret_function(as_interp(f), args));
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

bool stringp(obj_t str) {
  while (consp(str)) {
    if (!mintp(car(str)) || !isprint(as_mint(car(str))))
      return false;
    str = cdr(str);
  }
  if (!nullp(str)) return false;
  return true;
}

obj_t
printy(obj_t arg) {
  switch (gettype(arg)) {
  case TYPE_MINT:
    printf("%ld", as_mint(arg)); break;
  case TYPE_SYM:
    printf("%s", as_sym(arg)->key); break;
  case TYPE_FUNC:
    switch(getftype(as_func(arg))) {
    case FTYPE_COMPILED:
      printf("<C function>"); break;
    case FTYPE_INTERP:
      printf("<function>"); break;
    case FTYPE_MACRO:
      printf("<macro>"); break;
    case FTYPE_SPECIAL:
      printf("<special form>"); break;
    }
    break;
  case TYPE_CONS:
    if (stringp(arg)) {
      putchar('"');
      while (consp(arg)) {
	if (mintp(car(arg)) && isprint(as_mint(car(arg))))
	  putchar(as_mint(car(arg)));
	arg = cdr(arg);
      }
      putchar('"');
    } else {
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
    break;
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
  return c=='(' || c==')' || isspace(c);
}

obj_t
read_mint(char *str) {
  char *endptr = str;
  mint_t x = strtol(str, &endptr, 0);
  if (!*endptr) return make_mint(x);
  else return nil;
}

obj_t
read_string(FILE *in) {
  obj_t ret = cons(quote, nil);
  obj_t ptr = ret;
  int c;
  bool backslashed = false;
  while ((c = fgetc(in)) != EOF) {
    if (!backslashed && c == '"') break;

    if (c == '\\')
      backslashed = true;
    else
      backslashed = false;

    as_cons(ptr)->cdr = cons(make_mint(c), nil);
    ptr = cdr(ptr);
  }
  return ret;
}

obj_t
read(FILE *in) {
  int c;

  while (isspace(c = fgetc(in)));

  switch (c) {
  case EOF:
    error(E_END_OF_FILE, nil);
  case ')':
    error(E_READ_ERROR, nil);
  case '(':
    return read_cons(in);
  case '\'':
    return cons(quote, read(in));
  case '`':
    return cons(quasiquote, read(in));
  case ',':
    if ((c = fgetc(in)) == '@') 
      return cons(unquote_splice, read(in));
    else {
      ungetc(c, in);
      return cons(unquote, read(in));
    }
  case '"':
    return read_string(in);
  case ';':
    while ((c = fgetc(in)) != '\n');
  default: ;
    // fall through to finish reading
  }

  ungetc(c, in);
  char *tok = gets_until(in, is_terminating);
  obj_t it = read_mint(tok);
  if (!nullp(it)) return it;
  else return make_sym(intern_name(tok));
}


jmp_buf errhandler;
obj_t errobj;
bool did_autoload = false;

int main() {
  symtable = symt_create(128);
  nil = make_sym(make_self_evaluating("nil"));
  t = make_sym(make_self_evaluating("t"));
  quote = make_sym(intern_name("quote"));
  quasiquote = make_sym(intern_name("quasiquote"));
  unquote = make_sym(intern_name("unquote"));
  unquote_splice = make_sym(intern_name("unquote-splice"));

  setup_builtins();

  FILE *autoload = fopen("autoload.lisp", "r");
  if (!autoload) did_autoload = true;

  int ecode = setjmp(errhandler);
  if (ecode == 0 || ecode == E_TRY_AGAIN) {
    while(!did_autoload) {
      eval(read(autoload));
    }
    while(1) {
      printf("> ");
      printy(eval(read(stdin)));
      putchar('\n');
    }
  } else switch(ecode) {
    case E_READ_ERROR:
      puts("* READ ERROR");
      longjmp(errhandler, E_TRY_AGAIN);
    case E_END_OF_FILE:
      if (did_autoload) 
	exit(0);
      else {
	did_autoload = true;
	longjmp(errhandler, E_TRY_AGAIN);
      }
    case E_NO_FUNCTION:
      printf("* NOT A FUNCTION: ");
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
    case E_FAILED_BIND:
      printf("* BINDING FAILED: ");
      printy(car(errobj));
      printf(" <-> ");
      printy(cdr(errobj));
      putchar('\n');
      longjmp(errhandler, E_TRY_AGAIN);
    case E_INVALID_NAME:
      printf("* NAME NOT A SYMBOL: ");
      printy(errobj);
      putchar('\n');
      longjmp(errhandler, E_TRY_AGAIN);
    case E_REDEFINE:
      printf("* ATTEMPT TO REDEFINE: ");
      printy(errobj);
      putchar('\n');
      longjmp(errhandler, E_TRY_AGAIN);
    case E_RUNTIMEY:
      printf("* RUNTIME ERROR: ");
      while (consp(errobj)) {
	printy(car(errobj));
	putchar(' ');
	errobj = cdr(errobj);
      }
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

