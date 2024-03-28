data auto
code auto
stack 1
interrupts 1

; load NULL module

str driver "drivers\\null.exe"
str name "null"

; load module
mov ax 28
mov bx $driver
mov cx $name

sys

str loaded "NULL module loaded."
mov ax $loaded
printstr ax

mov ax 12
mov bx 0
sys

