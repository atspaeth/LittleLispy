;; Defmacro and defun both come in ! and !less variants;
;;  the difference is that the ! version creates a constant
;;  that cannot be reassigned, probably only good for builtins.
(def defmacro!
    (mu (name arglist . body)
	`(def ,name (mu ,arglist . ,body))))

(defmacro! defmacro (name arglist . body)
  `(set ,name (mu ,arglist . ,body)))

(defmacro! defun! (name arglist . body)
  `(def ,name (lambda ,arglist . ,body)))

(defmacro! defun (name arglist . body)
  `(set ,name (lambda ,arglist . ,body)))

;; When your only conditional primitive is (cond)
(defmacro! if (cnd thn els)
  `(cond ,cnd ,thn t ,els))
(defmacro! when (cnd . thn)
  `(cond ,cnd (do . ,thn)))
(defmacro! unless (cnd . thn)
  `(cond ,cnd nil t (do . ,thn)))

;; Yes, this macro body includes a manually expanded version of itself
;;  to get the effect of flet within the definition of let... :D
(defmacro! let (bindings . body)
  ((lambda (every-other)
     `((lambda ,(every-other bindings)
	 (do . ,body))
       ,@(every-other (cdr bindings))))
   (lambda (lst)
     (if (null? lst) nil
	 (cons (car lst) (every-other (cdr (cdr lst))))))))

(defmacro! flet (bindings . body)
  (let (lambdaify (lambda (((name arglist . exprs) . rest))
		    `(,name
		      (lambda ,arglist . ,exprs)
		      ,@(if (null? rest) nil
			    (lambdaify rest)))))
    `(let ,(lambdaify bindings)
       . ,body)))

(defun! atom? (x)
  (null? (cons? x)))

;; Sequence stuff
(defun map (f xs)
  (if (null? xs) nil
      (cons (f (car xs))
	    (map f (cdr xs)))))

(defun foldl (f acc xs)
  (if (atom? xs)
      acc
      (foldl f (f (car xs) acc) (cdr xs))))

(defun foldr (f start xs)
  (if (atom? xs)
      start
      (f (car xs)
	 (foldr f start (cdr xs)))))

(defun filter (f xs)
  (cond
    (atom? xs) nil
    (f (car xs)) (cons (car xs)
		       (filter f (cdr xs)))
    t (filter f (cdr xs))))

(defun reverse (lst)
  (foldl cons nil lst))

(defun range (start end)
  (cond (= start end) (list end)
	(< start end) (cons start (range (+ start 1) end))
	(> start end) (cons start (range (- start 1) end))))


;; String stuff
(defun is-lower (char)
  (and (>= char (car "a")) (<= char (car "z"))))
(defun is-upper (char)
  (and (>= char (car "A")) (<= char (car "Z"))))
(defun is-digit (char)
  (and (>= char (car "0")) (<= char (car "9"))))
(defun to-upper (char)
  (if (is-lower char)
      (+ char (- (car "A") (car "a")))
      char))
(defun to-lower (char)
  (if (is-upper char)
      (+ char (- (car "a") (car "A")))
      char))

(let (words '(Finished loading standard library.))
  (printnl . words))
