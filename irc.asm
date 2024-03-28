stack 64
interrupts 1
data auto
code auto

;jmp connect

str devnamef "DEV:\\sock\0\0\0\0"
str devname "sock0"
;str args_connect "\x1\x0B\x1Airc.freenode.net"
str args_connect 100
str args_service "\0"
str args_socketstatus "\x5"
word sock_desc
str out_buff 513
str in_buff 513
word child_retval 0
str port 8
str ping "PING"
str nickmsg "NICK "
str usermsg "USER "
str quitcmd "/quit"
str quitmsg "QUIT\r\n"
str nick 36
str newline "\r\n"
word nicklen
;str user " ! leet : leet.computer\r\n"
str user " ! elite : using.leetos\r\n"
str privmsg "PRIVMSG "
str channel 36
word chanlen
str join "JOIN "
str command_end " :"
str shout "!"

; get argument count
mov ax 13
sys
mov bx 3
cmp ax bx
jl noargs

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
;printuint ax
cmp ax dx
jne escape_loop

jmp connect_loop

:escape_loop
mov $sock_desc ax

printuint ax
str connected "Connected"
mov ax $connected
;printstr ax

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
;printstr dx
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
jne set_nick
str error "Could not create clone"
mov ax $error
printstr ax
jmp exit

:set_nick
str select_channel "Select channel\r\n"
mov ax 1
mov bx $select_channel
mov cx 16
mov dx 1
sys

:getchan
mov ax $channel
mov bx 36
call ngets
mov cx ax
jcz getchan

;dec ax
;add bx ax
xor dl dl
mov bx $channel
add bx ax
mov *bx dl
mov $chanlen ax

str select_nick "Select nickname\r\n"
;mov ax $select_nick
;printstr ax
mov ax 1
mov bx $select_nick
mov cx 17
mov dx 1
sys

:getnick
mov ax $nick
mov bx 36
call ngets
mov cx ax
jcz getnick

;dec ax
mov $nicklen ax

mov ax 1
mov bx $nickmsg
mov cx 5
mov dx $sock_desc
sys
mov ax 1
mov bx $nick
mov cx $nicklen
sys
mov ax 1
mov bx $newline
mov cx 2
sys

mov ax 1
mov bx $usermsg
mov cx 5
mov dx $sock_desc
sys
mov ax 1
mov bx $nick
mov cx $nicklen
sys
mov ax 1
mov bx $user
mov cx 25
sys

; wait a while before joining channel
mov ax 18
sys
mov ex dx
mov dx cx
mov cx bx
mov bx ax
mov ax 5
add ex ax

mov ax 22
sys

mov ax 1
mov bx $join
mov cx 5
mov dx $sock_desc
sys
mov ax 1
mov bx $channel
mov cx $chanlen
sys
mov ax 1
mov bx $newline
mov cx 2
sys

jmp user_loop

:listen_thread
mov bx $devnamef
mov ax 5
add bx ax
:wait_socket_ready
mov ax 29
mov cx $args_socketstatus
sys
mov cx ax
jcz wait_socket_ready
inc cx
jcz exit

mov ex 0
:listen_loop
; read socket
mov ax 0
; append the read contents to in_buff
mov bx $in_buff
add bx ex
mov cx 1
mov dx $sock_desc
sys
inc ex

; test if the last character is a unix-style newline
mov al *bx
mov ah 0x0A
cmp al ah
; if not, continue reading
jne listen_loop

mov al 0
inc bx
mov *bx al

mov cx ex

; check if ping
mov ax $in_buff
mov bx $ping
call strstr
mov bx $in_buff
cmp ax bx
jne test_privmsg

; answer to ping
inc ax
mov bl 'O'
mov *ax bl
mov ax 1
mov bx $in_buff
mov dx $sock_desc
sys
jmp listen_thread

:test_privmsg
mov ax $in_buff
mov bx $privmsg
call strstr
mov cx ax
jcz listen_write

mov ax $in_buff
mov bx $channel
call strstr
mov cx ax
jcz listen_write

mov ax $in_buff
mov bx $shout
call strstr
mov cx ax
jcz listen_write
;printuint cx
mov dx $in_buff
;printuint dx
sub cx dx
;printuint cx
mov ax 1
mov bx $in_buff
mov dx 1
sys

mov ax $in_buff
mov bx $command_end
call strstr
mov bx ax
mov ax $in_buff
mov cx bx
sub cx ax
sub ex cx
mov cx ex
mov ax 1
mov dx 1
sys


jmp listen_thread

; write to stdout
:listen_write
mov ax 1
mov cx ex
mov bx $in_buff
mov dx 1
sys
jmp listen_thread

:user_loop
word user_msg_len
; read user input
;mov ax 0
;mov bx $out_buff
;mov cx 512
;mov dx 0
;sys
:getusermsg
mov ax $out_buff
mov bx 512
call ngets
mov cx ax
jcz getusermsg

mov $user_msg_len ax

push ax

mov bx $out_buff
mov ax $quitcmd
call strcmp
je quit

pop ax

; write to socket
mov ax 1
mov bx $privmsg
mov cx 8
mov dx $sock_desc
sys
mov ax 1
mov bx $channel
mov cx $chanlen
sys
mov ax 1
mov bx $command_end
mov cx 2
sys

mov cx $user_msg_len
mov ax 1
mov bx $out_buff
sys

mov cx 2
mov ax 1
mov bx $newline
sys
jmp user_loop

; ax == str1, bx == str2
:strcmp
  push bx
  push cx
  push dx
  xor dl dl

  :strcmp_loop
  mov cl *ax
  mov ch *bx
  jcz strcmp_end

  cmp cl dl
  je strcmp_end
  cmp ch dl
  je strcmp_end

  inc ax
  inc bx

  cmp cl ch
  je strcmp_loop

  :strcmp_end
  mov ax cx

  pop dx
  pop cx
  pop bx
ret

; AX == pointer to str
; Returns length of the string to AX
:strlen
push cx
push bx
mov bx ax
xor ax ax
xor cx cx
:strlen_loop
mov cl *bx
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

; ax == ptr to string
; bx == max length
; returns count of read bytes
:ngets
  push cx
  push dx
  push ex
  push fx
  push bx
  push gx

  mov ex 0
  mov gx ex
  mov fx bx

  ; read one byte
  mov bx ax
  mov dx 0
  :ngets_loop
  mov ax 0
  mov cx 1
  sys

  ; if read returns -1, exit
  inc ax
  cmp ax ex
  je quit
  dec ax

  ; if newline, return
  mov cl *bx
  add bx ax
  mov ch 0x0A
  cmp cl ch
  je ngets_end
  mov ch 0x0D
  cmp cl ch
  je ngets_end

  ; else increment gx
  add gx ax

  cmp fx gx
  je ngets_end

  jmp ngets_loop

  :ngets_end
  mov *bx 0
  mov ax gx
  pop gx
  pop bx
  pop fx
  pop ex
  pop dx
  pop cx
ret

:quit
mov ax $quitmsg
call strlen
mov cx ax
mov ax 1
mov bx $quitmsg
mov dx $sock_desc
sys
jmp exit

:noargs
str usage "Usage: >IRC [address] [port]\r\nExample: IRC sininenankka.dy.fi 6667\r\n"
mov ax $usage
call strlen
mov cx ax
mov bx $usage
mov ax 1
mov dx 1
sys
:exit
mov ax 12
mov bx 0
sys
