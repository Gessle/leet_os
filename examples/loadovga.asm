stack 1
data auto
code auto
interrupts 1
; load VGA driver module

str driver "drivers\\optvga.exe"
str name "vga"

; load module
mov ax 28
mov bx $driver
mov cx $name

sys

;wait

str loaded "Optimized VGA module loaded."
mov ax $loaded
printstr ax

mov ax 12
mov bx 0
sys
