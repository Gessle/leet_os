data auto
code auto
stack 1
interrupts 1

; load RANDOM module

str driver "drivers\\random.exe"
str name "rand"

; load module
mov ax 28
mov bx $driver
mov cx $name

sys

mov dx -1
cmp ax dx
je error

str loaded "RANDOM module loaded."
mov ax $loaded
printstr ax

mov ax 12
mov bx 0
sys

:error
str error "Error"
mov ax $error
printstr ax

wait
