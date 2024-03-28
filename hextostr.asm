jmp main

; cx = number
; bx = pointer to buffer
; writes number in ax to buffer in [bx] as base-16
:hextostr
push ax
push bx
push cx
push dx
push ex

mov ex bx
mov dh 4
mov dl 0xF
mov al 9
mov ah '0'
mov bh '7'

:hextostr_loop
jcz hextostr_end
mov bl ch
shr bl dh
shl cx dh
and bl dl

cmp bl al
jg hextostr_highnums

add bl ah
mov *ex bl
inc ex
jmp hextostr_loop

:hextostr_highnums
add bl bh
mov *ex bl
inc ex
jmp hextostr_loop

:hextostr_end
pop ex
pop dx
pop cx
pop bx
pop ax
ret

:main
mov cx 0xF5E8
call hextostr
