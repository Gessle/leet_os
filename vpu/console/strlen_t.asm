jmp main

:strlen
; save registers bx and cx
push bx
push cx

; zero the counter
mov cx 0

; null byte for comparing
mov bh 0

:loopstrlen
; byte from *ax to bl
mov bl *ax
; compare to null byte
cmp bh bl
; if equals, goto end
je endstrlen
; else increment ax and cx and repeat
inc ax
inc cx
jmp loopstrlen 

:endstrlen
; move length from counter to accumulator
mov ax cx

pop cx
pop bx

ret

:main
str string "hello world"
mov ax $string
call strlen
printuint ax