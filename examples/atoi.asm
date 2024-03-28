jmp main

; AX == pointer to str
; Returns length of the string to AX
:strlen
push cx
push bx
mov bx ax
mov ax 0
:strlen_loop
mov cx *bx
jcz strlen_end
inc ax
inc bx
jmp strlen_loop
:strlen_end
pop bx
pop cx
ret

; AX == pointer to string
; Returns int (unsigned) value to AX
:atoi
push bx
push cx
push dx
push ex
push fx
push gx
push hx
mov hx 1

mov dh 0x2D
mov dl *ax
cmp dh dl
jne atoi_start

inc ax
mov hx -1

:atoi_start
mov bx ax
call strlen
mov cx ax
mov ax 0
mov dh 0x30

:atoi_loop
dec cx
mov dl *bx
inc bx
sub dl dh
jcz atoi_end

mov ex cx
mov gx 10
mov fx 1
:atoi_mul
mul fx gx
dec cx
jcz atoi_stopmul
jmp atoi_mul
:atoi_stopmul
mul fx dl
add ax fx
mov cx ex

jmp atoi_loop
:atoi_end
add ax dl

mul ax hx

pop hx
pop gx
pop fx
pop ex
pop dx
pop cx
pop bx
ret

:main
str luku "-15121"
mov ax $luku
call atoi
printsint ax