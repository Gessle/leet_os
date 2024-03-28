;stack 16
;code auto
;data auto
;interrupts 5

:newclone
str uusisaie "Tehd„„np„ uusi ohjelma."
mov ax $uusisaie
;printstr ax

word pid 0
str ohjelma "args.app"
str args ""

mov ax 15
mov bx &pid
mov cx $ohjelma
mov dx $args
sys

mov ax 23
mov bx $pid
mov cx 9
sys

jmp newclone