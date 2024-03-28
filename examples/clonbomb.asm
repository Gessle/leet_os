stack 512
data auto
code auto
interrupts 3

; clone bomb

jmp main

:child_died
str kuoli "<is„nnyys prosessi> Lapsiprosessi kuoli"
str paluuarvolla "<is„nnyys prosessi> paluuarvo:"
push ax
mov ax $kuoli
printstr ax
mov ax $paluuarvolla
printstr ax
mov ax $child_retval
mov bx *ax
printsint bx
pop ax 
wait
iret



:main
setint 2 .child_died $child_died

:loop
word child_retval
mov ax 16
mov bx .loop
mov cx &loop
mov dx 2
mov ex &child_retval
sys
mov bx 0xFFFF
cmp ax bx
jne noerror
str error "Error"
mov ax $error
printstr ax
wait
:noerror
jmp loop
