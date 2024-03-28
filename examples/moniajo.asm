stack 128
code 1 256
data 1 256
interrupts 1


; moniajotesti
str hello "hello friends"
mov ax $hello
dec cx
:loop
printstr ax
printuint cx
dec cx
jcz end
jmp loop
:end
