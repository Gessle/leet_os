

:open_port
str devname "sock0"
str devnamef "DEV:\\sock\0\0\0\0"
str args_openport "\x4\0\0"
word socket_desc

mov ax $args_openport
inc ax
; open port 25
mov *ax 25

; send command to driver
mov ax 29
mov bx $devname
mov cx $args_openport
sys

printsint ax

;open file
mov cx ax
mov bx $devnamef
mov ax 9
add bx ax
call hextostr

mov ax 2
mov bx 0
mov cx 0
mov dx $devnamef
printstr dx
sys

printsint ax

; read file
:accept_loop
mov dx ax
mov ax 0
mov cx 1
mov bx &socket_desc
sys

;inc bx
;mov ax 0
;sys

mov ax $socket_desc
printuint ax

jmp accept_loop

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
