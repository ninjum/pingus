#!/usr/bin/guile \
--debug -e main -s
!#

(define (get-keyword-value args keyword default)
  (let ((kv (memq keyword args)))
    (if (and kv (>= (length kv) 2))
	(cadr kv)
	default)))

;; Return a list containing all the objects which match 'pred?'
(define (filter pred? objects)
  (let loop ((objs objects)
	     (result '()))
    (cond ((null? objs) (reverse! result))
	  ((pred? (car objs)) (loop (cdr objs) (cons (car objs) result)))
	  (else (loop (cdr objs) result)))))

;; Return a list of the strings which match the regex 'rx'
(define (grep rx strings)
  (let ((r (make-regexp rx)))
    (filter (lambda (x) (regexp-exec r x)) strings)))

(define (print . str)
  (for-each (lambda (x) (display x)) str)
  (force-output))

(define (println-sep . str)
  (for-each (lambda (x) 
	      (display "\"")
	      (display x)
	      (display "\"")) str)
  (newline))

;; Print the given strings and a newline
(define (println . str)
  (apply print str) (newline))

(define (println-port port . strs)
  (for-each (lambda (el)
	      (display el port))
	    strs)
  (newline port))

(define (directory->list path)
  (let* ((dir (opendir path))
	 (lst '()))
    (let loop ((fobj (readdir dir)))
      (cond ((not (eof-object? fobj))
	     (set! lst (cons fobj lst))
	     (loop (readdir dir)))))
    (closedir dir)
    (reverse lst)))

;; End: Helper functions

(define (directory? file)
  (equal? (stat:type (stat file)) 'directory))

(define (indent level)
  (make-string (* 2 level) #\space))

(define (cut-extension filename)
  (substring filename 0 (- (string-length filename) 4)))

(define (dirtree2scr port level path)
  (let* ((lst    (sort  (directory->list path) string<=?))
	 (images (grep "\.png$\|\.jpg$" (filter (lambda (x) 
						  (not (directory? (string-append path "/" x))))
						lst)))
	 (directories (filter (lambda (el)
				(and (directory? (string-append path "/" el))
				     (not (equal? el "CVS"))
				     (not (equal? el "old"))
				     (not (equal? (string-ref el 0) #\.))))
			      lst)))
    (for-each (lambda (el)
		(println-port port 
                              (indent (1+ level))
                              "<sprite name=\"" (cut-extension el) "\">\n"
                              (indent (+ 2 level))
                              "<image file=\"../" path "/" el "\"/>\n"
                              (indent (1+ level))"</sprite>\n"
                              ))
	      images)
    (for-each (lambda (el)
		(println-port port (indent level) "<section name=\"" el "\">")
		(dirtree2scr port (1+ level) (string-append path "/" el))
		(println-port port (indent level) "</section>"))
	      directories)))

(define (create-scr-file filename directory)
  (let ((port (open-output-file filename)))
    (println-port port "<?xml version=\"1.0\"?>\n")
    (println-port port "<!-- Automatic generated by gen-scrfile.scm - don't edit by hand! -->\n")
    (println-port port"<resources>")
    (dirtree2scr port 1 directory)
    (println-port port"</resources>")
    (close port)))

(define (main args)
  (create-scr-file "data/groundpieces-ground.xml" "images/groundpieces/ground")
  (create-scr-file "data/groundpieces-solid.xml" "images/groundpieces/solid")
  (create-scr-file "data/groundpieces-remove.xml" "images/groundpieces/remove")
  (create-scr-file "data/groundpieces-transparent.xml" "images/groundpieces/transparent")
  (create-scr-file "data/groundpieces-bridge.xml" "images/groundpieces/bridge")
  (create-scr-file "data/hotspots.xml" "images/hotspots"))


;; EOF ;;
