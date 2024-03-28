byte char 0

mov ax 1
mov bx &char
mov cx 1
mov dx 1
:loop
sys
mov el $char
inc ex
mov $char el
jmp loop