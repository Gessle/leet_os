str error "Error"
str works "Conversion works"

mov al -100

sconvw bx al
sconvb ah bx

cmp ah al
je success

mov cx $error
jmp loppu

:success
mov cx $works

:loppu
printsint al
printsint ah
printstr cx