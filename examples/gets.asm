jmp main

;gets
; AX == pointer to str
; returns pointer to str or NULL on error
; reads from file descriptor 0
:gets
push bx
push cx
push dx

mov bx ax
mov ax 0
mov cx 0xFFFF
mov dx 0
sys
cmp ax cx
je gets_returnnull
mov ax bx
jmp gets_end
:gets_returnnull:
mov ax 0
:gets_end
pop dx
pop cx
pop bx
ret

:main
str input 50

mov ax $input
call gets
printstr ax