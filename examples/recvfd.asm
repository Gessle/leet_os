interrupts 133
data auto
code auto
stack 16

; a program that receives a file descriptor

jmp main

:receive_fd
str received "Received FD:"
str buff 200
mov bx $received
printstr bx
in 0
printuint ax
; read from the file descriptor
mov dx ax
mov ax 0
mov bx $buff
mov cx 199
sys
; print contents
printstr bx

iret


:main
setint 132 .receive_fd $receive_fd
wait