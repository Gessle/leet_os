data auto
code auto
interrupts 2
stack 16

; redirect hardware interrupt 9 to process signal 1
jmp main

:intr_handler
str msg "Keyboard interrupt!"
mov ax $msg
printstr ax
iret

:main
setint 1 .intr_handler $intr_handler

; Syscall 36 (sethwint)
mov ax 36
mov bx 9
mov cx 1
mov dx 1
sys

mov bx -1
cmp ax bx
je error

:end
;wait
mov ax 23
mov bx 0
mov cx 23
sys
jmp end
:error
str error "Error happened. :("
mov ax $error
printstr ax
wait
