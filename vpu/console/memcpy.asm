str string1 "memcpy "
str string2 "works!"
str dest 20

; copy 7 bytes
mov ex 7
; mov segment of string1 to ax
mov ax .string1
; mov offset of string1 to bx
mov bx $string1
;mov segment of dest to cx
mov cx .dest
; mov offset of dest to dx
mov dx $dest
; do memcpy
memcpy

; copy segment of string2 to ax
mov ax .string2
; copy offset of string2 to bx
mov bx $string2
; copy 6 bytes
mov ex 6
; do memcpy
memcpy

; print "memcpy works!"
mov fx $dest
printstr fx
