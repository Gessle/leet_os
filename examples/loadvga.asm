stack 1
data auto
code auto
interrupts 1
; load VGA driver module

str driver "drivers\\dbuffvga.exe"
str name "vga"

; load module
mov ax 28
mov bx $driver
mov cx $name

sys

;wait

str loaded "BUFFVGA module loaded."
mov ax $loaded
printstr ax

mov ax 12
mov bx 0
sys
