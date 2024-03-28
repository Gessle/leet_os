; spawn test

str path "alarm.app"
str pid 4
str args 20

mov ax 15
mov bx $pid
mov cx $path
mov dx $args
sys

mov ax *bx
printuint ax