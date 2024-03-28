; load RANDOM module

str driver "drivers\\random.exe"
str name "random"

; load module
mov ax 28
mov bx $driver
mov cx $name

sys

str loaded "RANDOM module loaded."
mov ax $loaded
printstr ax

mov ax 12
mov bx 0
sys
