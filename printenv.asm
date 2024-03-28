mov bx 0
:loop
mov ax 48
mov dx 0
mov cx 200
sys
cmp ax -1
je end
printstr dx
inc bx
jmp loop
:end
