stack 64
interrupts 1
data auto
code auto

str devnamef "DEV:\\sock0"
str devname "sock0"
;str args_connect "\x1\x0B\x1Airc.freenode.net"
str args_connect 100
str args_service "\0"
word sock_desc
str out_buff 80
str in_buff 513
word child_retval 0
str port 8
str ping "PING"

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
str connected "Connected"
mov ax $connected
printstr ax

:service_loop
; open socket as a file
mov bx 0
mov cx 0
mov dx $devnamef
mov ex dx
mov fx 9
add ex fx
mov ax $sock_desc
mov fx 0x30
add ax fx
mov *ex al
mov ax 2
printstr dx
sys
mov $sock_desc ax

; make a clone to handle listening
mov ax 16
mov bx .listen_thread
mov cx &listen_thread
mov dx 1
mov ex &child_retval
sys
mov ex -1
cmp ax ex
jne user_loop
str error "Could not create clone"
mov ax $error
printstr ax

:listen_thread
mov ex 0
:listen_loop
; read socket
mov ax 0
; append the read contents to in_buff
mov bx $in_buff
add bx ex
mov cx 80
mov dx $sock_desc
sys

mov cx ax
mov ax $in_buff
mov bx $ping
call strstr

; write to stdout
:listen_write
mov ax 1
mov bx $in_buff
mov dx 1
sys
jmp listen_thread

:user_loop
; read user input
mov ax 0
mov bx $out_buff
mov cx 80
mov dx 0
sys

; write to socket
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

; strstr
; arguments: AX == pointer to str,
;            BX == pointer to ndl
; returns: NULL if ndl not found in str,
;          pointer to ndl in str if found
:strstr
  push bx
  push cx
  push dx
  push ex

  mov dx ax
  mov ex bx
  mov cx 0

  :strstr_search_begin
  mov ch *ax
  jcz strstr_search_end
  mov cl *bx
  cmp ch cl
  je strstr_search_continue
  jmp strstr_search_nomatch

  :strstr_search_continue
  inc ax
  inc bx
  mov cx 0
  mov cl *bx
  jcz strstr_search_found
  mov ch *ax
  cmp ch cl
  je strstr_search_continue
  jcz strstr_search_found
  mov cl 0
  jcz strstr_search_end

  :strstr_search_nomatch
  inc dx
  mov ax dx
  mov bx ex
  mov cx 0
  jmp strstr_search_begin

  :strstr_search_end
  mov ax 0
  jmp strstr_end

  :strstr_search_found
  mov ax dx

  :strstr_end

  pop ex
  pop dx
  pop cx
  pop bx
ret


:noargs
str usage "Usage: >TELNET [address] [port]"
mov ax $usage
printstr ax
mov ax 12
mov bx 0
sys
