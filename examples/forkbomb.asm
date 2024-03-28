stack 16
code auto
data auto
interrupts 5

; fork bomb
str clone_retval 2

:newclone
str uusisaie "Tehd„„np„ uusi s„ie."
mov ax $uusisaie
;printstr ax
mov ax 17
mov dx $clone_retval
sys


cmp ax cx
je imclone

str original "<orkkis> N„ytt„isin olevan is„nt„prosessi."
str pid "<orkkis> Kloonin pid:"

mov bx $original
;printstr bx
mov bx $pid
;printstr bx
;printuint ax

jmp newclone

:imclone
str clone "<klooni> N„ytt„isin olevan klooni."
mov ax $clone
;printstr ax
jmp newclone
