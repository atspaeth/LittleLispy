# LittleLispy
A one-or-two-weekend academic exercise toy lisp interpreter.

This will be a Lisp-1 with dynamic scope like in days of old,
mainly for reasons of implementation simplicity.

There are only four datatypes: cons, mint ("Machine INTeger",
i.e fixnum), compiled function (only builtins for now), and symbol.

The available builtin functions will be the following:
* nil and t are symbols that evaluate to themselves; nil refers 
  to either the empty list or logical falsehood, while t refers
  to logical truth
* (cons? x) returns t if x is a cons cell, otherwise nil
* (sym? x) returns t if x is a symbol, otherwise nil
* (mint? x) returns t if x is a mint, otherwise nil
* (fun? x) returns t if x is a compiled function or a lambda 
  expression, otherwise nil
* (is? x y) returns whether x and y are identical, i.e. the same 
  symbol, number, function pointer, or cons pointer.
* (error ...) signals an error and prints its argument list
* (list ...) returns its argument list
* (quote ...) returns its argument list unevaluated
* (lambda ...) returns itself, potentially after checking
  its own validity as a lambda expression
* (cons a b) returns a new cons cell with car a and cdr b
* (car x) returns nil if x isn't a cons, otherwise x's car
* (cdr x) returns nil if x isn't a cons, otherwise x's cdr
* (rplaca x y) replaces x's car with y
* (rplacd x y) replaces x's cdr with y
* (set sym val) overwrites sym's current binding with val
