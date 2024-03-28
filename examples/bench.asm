code auto
data auto
stack 1
interrupts 1

; benchmark

word time
; get time
mov ax 18
sys
mov $time dx

; read stdin in loop and print everything to stdout
str buff 100
mov bx $buff
:loop
mov ax 0
mov cx 100
mov dx 0
sys
mov cx ax
jcz end
mov ax 1
mov dx 1
sys
jmp loop

:end
; get time
mov ax 18
sys

mov ax $time
sub dx ax
printuint dx

mov ax 12
mov bx 0
sys
