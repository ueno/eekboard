;; mim2remap.el -- generate eekxml remap file from m17n MIM file
;; Copyright (C) 2011 Daiki Ueno <ueno@unixuser.org>
;; Copyright (C) 2011 Red Hat, Inc.

;; This program is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.

;; This program is distributed in the hope that it will be useful, but
;; WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;; General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see
;; <http://www.gnu.org/licenses/>.

;;; Commentary:

;; emacs -Q -batch -l mim2remap.el -f batch-mim2remap \
;;	/usr/share/m17n/te-inscript.mim trans

;;; Code:

(require 'json)

(defconst mim2remap--char-to-keyname-alist
  '((32 . "space")
    (33 . "exclam")
    (34 . "quotedbl")
    (35 . "numbersign")
    (36 . "dollar")
    (37 . "percent")
    (38 . "ampersand")
    (39 . "apostrophe")
    (40 . "parenleft")
    (41 . "parenright")
    (42 . "asterisk")
    (43 . "plus")
    (44 . "comma")
    (45 . "minus")
    (46 . "period")
    (47 . "slash")
    (48 . "0")
    (49 . "1")
    (50 . "2")
    (51 . "3")
    (52 . "4")
    (53 . "5")
    (54 . "6")
    (55 . "7")
    (56 . "8")
    (57 . "9")
    (58 . "colon")
    (59 . "semicolon")
    (60 . "less")
    (61 . "equal")
    (62 . "greater")
    (63 . "question")
    (64 . "at")
    (65 . "A")
    (66 . "B")
    (67 . "C")
    (68 . "D")
    (69 . "E")
    (70 . "F")
    (71 . "G")
    (72 . "H")
    (73 . "I")
    (74 . "J")
    (75 . "K")
    (76 . "L")
    (77 . "M")
    (78 . "N")
    (79 . "O")
    (80 . "P")
    (81 . "Q")
    (82 . "R")
    (83 . "S")
    (84 . "T")
    (85 . "U")
    (86 . "V")
    (87 . "W")
    (88 . "X")
    (89 . "Y")
    (90 . "Z")
    (91 . "bracketleft")
    (92 . "backslash")
    (93 . "bracketright")
    (94 . "asciicircum")
    (95 . "underscore")
    (96 . "grave")
    (97 . "a")
    (98 . "b")
    (99 . "c")
    (100 . "d")
    (101 . "e")
    (102 . "f")
    (103 . "g")
    (104 . "h")
    (105 . "i")
    (106 . "j")
    (107 . "k")
    (108 . "l")
    (109 . "m")
    (110 . "n")
    (111 . "o")
    (112 . "p")
    (113 . "q")
    (114 . "r")
    (115 . "s")
    (116 . "t")
    (117 . "u")
    (118 . "v")
    (119 . "w")
    (120 . "x")
    (121 . "y")
    (122 . "z")
    (123 . "braceleft")
    (124 . "bar")
    (125 . "braceright")
    (126 . "asciitilde")))

(defun mim2remap--char-to-keyname (char)
  (let ((entry (assq char mim2remap--char-to-keyname-alist)))
    (if entry
	(cdr entry))))

(defun mim2remap (file map)
  (with-current-buffer (find-file-noselect file)
    (let (sexp)
      (goto-char (point-min))
      (while (and (null sexp) (not (eobp)))
	(setq sexp (read (current-buffer)))
	(if (eq (car sexp) 'map)
	    (setq sexp (cdr sexp))
	  (setq sexp nil)))
      (if sexp
	  (setq sexp (assq map sexp)))
      (unless sexp
	(error "No map named %S in %s" map file))
      (json-encode
       (mapcar
	(lambda (entry)
	  (let ((from (car entry))
		(to (car (cdr entry))))
	    (cons (if (listp from)
		      (car from)
		    (or (mim2remap--char-to-keyname (aref from 0))
			(error "No keyname for %c" (aref from 0))))
		  (if (characterp to)
		      (list (cons :text (char-to-string to)))
		    (list (cons :text to))))))
	(cdr sexp))))))

(defun batch-mim2remap ()
  (interactive)
  (let ((file (car command-line-args-left))
	(map (car (cdr command-line-args-left))))
    (princ (mim2remap file (intern map)))
    (setq command-line-args-left (nthcdr 2 command-line-args-left))))

;;; mim2remap.el ends here
