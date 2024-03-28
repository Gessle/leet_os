data auto
code auto
interrupts 2
stack 16

; set hardware int 1 to vpu int 1
jmp main

:intr_handler
str hello "Hello world"
mov ax $hello
printstr ax

;unset interrupt
mov ax 37
mov bx 1
sys
iret

:main
setint 1 .intr_handler $intr_handler

mov ax 36
mov bx 0x09
mov cx 1
sys        

:end
wait
jmp end