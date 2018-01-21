(set defmacro (mu (name arglist . body)
  (list 'set name (list 'mu arglist (cons 'do body)))))

(defmacro defun (name arglist . body)
  (list 'set name (list 'lambda arglist (cons 'do body))))

(defun every-other (lst)
  (if (null? lst) nil
      (cons (car lst) (every-other (cdr (cdr lst))))))

(defmacro let (bindings . body)
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

(defun reverse (lst)
  (foldl cons nil lst))
