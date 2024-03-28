stack 1
interrupts 1
code auto
data auto

str data 100

mov ax $data
mov bx 99
add ax bx
mov bx *ax
inc ax
mov bx *ax
inc ax
mov bx *ax
wait
