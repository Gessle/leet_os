data auto
code auto
stack 1
interrupts 1

; load TCPIP module

str driver "drivers\\tcpip.exe"
str name "sock"

; load module
mov ax 28
mov bx $driver
mov cx $name

sys

cmp ax 0
jne error

str loaded "TCPIP module loaded."
mov ax $loaded
printstr ax

:end
mov ax 12
mov bx 0
sys

:error
str errorstr "Error: Could not load TCPIP module"
mov ax $errorstr
printstr ax

jmp end