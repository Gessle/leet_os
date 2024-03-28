;stack 16
;code auto
;data auto
;interrupts 5

:newclone
str uusisaie "Tehd„„np„ uusi ohjelma."
mov ax $uusisaie
;printstr ax

word pid 0
str ohjelma "exittest.app"
str args ""

mov ax 15
mov bx &pid
mov cx $ohjelma
mov dx $args
sys
jmp newclone