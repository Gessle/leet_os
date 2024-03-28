interrupts 1
data auto
code auto
stack 16

byte char 0

:start
mov ax 1
mov bx &char
mov cx 1
mov dx 1
mov fx 256
:loop
sys
mov el $char
inc ex
mov $char el
dec fx
cmp fx gx
jne loop

mov fx 10
mov cx 1
:loop2
mov ex 0x0D
mov $char ex
sys
mov ex 0x0A
mov $char ex
sys
dec fx
cmp fx gx
jne loop2
jmp start