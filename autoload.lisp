;; Defmacro and defun both come in ! and !less variants;
;;  the difference is that the ! version creates a constant
;;  that cannot be reassigned, probably only good for builtins.
(def defmacro!
    (mu (name arglist . body)
	(list 'def name
	      (apply list 'mu arglist body))))
(defmacro! defmacro (name arglist . body)
  (list 'set name
	(apply list 'mu arglist body)))

(defmacro! defun! (name arglist . body)
  (list 'def name (apply list 'lambda arglist body)))

(defmacro! defun (name arglist . body)
  (list 'def name (apply list 'lambda arglist body)))

(defmacro! if (cnd thn els)
  (list 'cond cnd thn t els))

(defmacro! when (cnd . thn)
  (list 'cond cnd (cons 'do thn)))

(defmacro! unless (cnd . thn)
  (list 'cond cnd nil t (cons 'do thn)))

(defun every-other (lst)
  (if (null? lst) nil
      (cons (car lst) (every-other (cdr (cdr lst))))))

(defmacro! let (bindings . body)
  (cons (list 'lambda (every-other bindings)
	      (cons 'do body))
	(every-other (cdr bindings))))


(defun map (f xs)
  (if (null? xs) nil
      (cons (f (car xs))
	    (map f (cdr xs)))))

(defun foldl (f acc xs)
  (if (null? xs)
      acc
      (foldl f (f (car xs) acc) (cdr xs))))

(defun foldr (f start xs)
  (if (null? xs)
      start
      (f (car xs)
	 (foldr f start (cdr xs)))))

(defun filter (f xs)
  (cond
    (null? xs) nil
    (f (car xs)) (cons (car xs)
		       (filter f (cdr xs)))
    t (filter f (cdr xs))))

(defun reverse (lst)
  (foldl cons nil lst))

(apply printnl '(Finished loading standard library.))
