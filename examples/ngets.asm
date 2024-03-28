jmp main

;ngets
; AX == pointer to str
; BX == count of bytes to read
; returns pointer to str or NULL on error
; reads from file descriptor 0
:gets
push bx
push cx
push dx

mov cx bx
mov bx ax
mov dx 0
mov ax 0
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
mov bx 50
call gets
printstr ax