stack 16
interrupts 4
code auto
data auto

; fork test
str clone_retval 2

; how many clones
mov cx 5

:newclone

mov ax 17
mov dx $clone_retval
sys


cmp ax bx
je imclone

dec cx
jcz continue
jmp newclone
:continue

str original "<orkkis> N„ytt„isin olevan is„nnyysprosessi."

str pid "<orkkis> Kloonin pid:"

mov bx $original
printstr bx
mov bx $pid
printstr bx
printuint ax

wait

:imclone
str clone "<klooni> N„ytt„isin olevan klooni."
mov ax $clone
printstr ax
