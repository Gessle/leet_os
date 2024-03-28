stack 16
interrupts 32
data auto
code auto

str devnamef "DEV:\\sock\0\0\0\0"
str devname "sock0"
;str args_connect "\x1\x0B\x1Airc.freenode.net"
str args_connect 100
str args_service "\0"
str args_socketstatus "\x5"
word sock_desc
str out_buff 80
str in_buff 80
word child_retval 0
str port 8

setint 18 .program_end $program_end
setint 2 .sigint_handler $sigint_handler

; get argument count
mov ax 13
sys
mov bx 1
cmp ax bx
je noargs

mov dx $args_connect
mov *dx 1
inc dx
inc dx
inc dx

; get address
mov bx 1
mov cx 0
:get_arg_loop1
mov ax 14
sys
mov *dx al
inc cx
inc dx
cmp ah al
jne get_arg_loop1

; get port
mov dx $port
mov bx 2
mov cx 0
:get_arg_loop2
mov ax 14
sys
mov *dx al
inc cx
inc dx
cmp ah al
jne get_arg_loop2

; convert port to binary number
mov ax $port
call atoi
mov bx $args_connect
inc bx
mov *bx ax

mov dx -1
:connect_loop
mov ax 29
mov bx $devname
mov cx $args_connect
sys
printuint ax
cmp ax dx
jne escape_loop

jmp connect_loop

:escape_loop
mov $sock_desc ax

printuint ax
str connected "TCP SYN sent."
mov ax $connected
printstr ax

mov cx $sock_desc
mov bx $devnamef
mov ax 9
add bx ax
call hextostr

:service_loop
; open socket as a file
mov bx 0
mov cx 0
mov dx $devnamef
mov ax $sock_desc
mov ax 2
printstr dx
sys
mov $sock_desc ax

; make a clone to handle listening
mov ax 16
mov bx .listener
mov cx &listener
mov dx 18
mov ex &child_retval
sys
mov ex -1
cmp ax ex
jne user_loop
str error "Could not create clone"
mov ax $error
printstr ax


:listener
mov bx $devnamef
mov ax 5
add bx ax
:wait_ready_loop
; check socket status
mov ax 29
mov cx $args_socketstatus
sys

; if status == 0, loop
mov cx ax
jcz wait_ready_loop

; if status == -1, exit
inc cx
jcz program_end

str socket_is_ready "Connected."
mov bx $socket_is_ready
printstr bx

:listen_loop
; read socket
mov ax 0
mov bx $in_buff
mov cx 80
mov dx $sock_desc
sys

mov bx -1
cmp ax bx
je program_end

; write to stdout
mov cx ax
mov ax 1
mov bx $in_buff
mov dx 1
sys

; flush stdout
mov ax 27
mov bx 1
sys
jmp listen_loop

:user_loop
; read user input
mov ax 0
mov bx $out_buff
mov cx 80
mov dx 0
sys

; write to socket
; convert to CP/M-style newlines
push ax
mov bx $out_buff
add bx ax
dec bx

mov ah 0x0A
mov al *bx
cmp ah al
jne send_unfinished_row

mov *bx 0x0D
inc bx
mov *bx 0x0A

pop ax
mov cx ax
inc cx
mov ax 1
mov bx $out_buff
mov dx $sock_desc
sys
jmp user_loop

:send_unfinished_row
pop ax
mov cx ax
mov ax 1
mov bx $out_buff
mov dx $sock_desc
sys
jmp user_loop

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

; cx = number
; bx = pointer to buffer
; writes number in cx to buffer in [bx] as base-16
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


:noargs
str usage "Usage: >TELNET [address] [port]"
mov ax $usage
printstr ax
:program_end
mov ax 12
mov bx 0
sys

:sigint_handler
; write byte 0x03 to socket
push ax
push bx
push cx
push dx

byte break 0x03

mov ax 1
mov bx &break
mov cx 1
mov dx $sock_desc
sys

pop dx
pop cx
pop bx
pop ax
iret
