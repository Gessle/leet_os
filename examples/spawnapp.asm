stack 1
data auto
code auto
interrupts 1

; spawn test

str path "edit"
str pid 4
str args 20

mov ax 15
mov bx $pid
mov cx $path
mov dx $args
sys
