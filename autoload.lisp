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
  `(def ,name (lambda ,arglist . ,body)))

(defmacro! if (cnd thn els)
  `(cond ,cnd ,thn t ,els))

(defmacro! when (cnd . thn)
  `(cond ,cnd (do . ,thn)))

(defmacro! unless (cnd . thn)
  `(cond ,cnd nil t (do . ,thn)))

(defmacro! let (bindings . body)
  ((lambda (every-other)
     `((lambda ,(every-other bindings)
	 (do . ,body))
       ,@(every-other (cdr bindings))))
   (lambda (lst)
     (if (null? lst) nil
	 (cons (car lst) (every-other (cdr (cdr lst))))))))

(defun! atom? (x)
  (null? (cons? x)))

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

(let (words '(Finished loading standard library))
  (printnl . words))
