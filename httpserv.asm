stack 16
interrupts 32
data 1 512
code auto

; lEEt/OS HTTP server v. 1.1
;
; changelog
; 1.1: The parent process should not be closing the socket fd...

str devnamef "DEV:\\sock\0\0\0\0"
str devname "sock0"
str args_connect "\x3\0\0"
str index "index.htm"
str notfounddoc "404.htm"
str defaultresponse "HTTP/1.1 200 OK\r\n"
str cacheresponse "Cache-Control: no-store\r\n\r\n"
str htdocdir "htdocs\\"
word sock_desc

; change directory to htdocs
mov bx $htdocdir
mov ax 4
sys

mov ax $args_connect
inc ax
; port to listen
mov *ax 80

str sending_command "Sending command to driver."
mov ax $sending_command
;printstr ax

:open_port
mov ax 29
mov bx $devname
mov cx $args_connect
sys
;printuint ax
mov hx -1
cmp ax hx
jne port_opened_ok

; print error number
mov ax 38
sys
printuint ax

jmp open_port

:port_opened_ok
mov $sock_desc ax

str port_opened "Port opened."
mov ax $port_opened
printstr ax

mov ax $sock_desc
mov bx $devnamef
mov cx 9
add bx cx

call hextostr

str listening "Listening connections..."
mov ax $listening
printstr ax

mov ax 2
mov bx 0
mov cx 0
mov dx $devnamef
mov ex 0
;printstr dx
sys

word accepted_socket

mov $sock_desc ax
:accept_loop
mov ax 0
mov cx 2
mov bx &accepted_socket
mov dx $sock_desc
sys

mov ax $accepted_socket
mov bx $devnamef
mov cx 9
add bx cx

call hextostr

mov dx $devnamef
printstr dx

; open the accepted socket as file
mov ax 2
mov bx 0
mov cx 0
mov dx $devnamef
mov ex 0
sys
printsint ax
cmp ax hx
; if file could not be opened
je accept_loop

; else give the file descriptor to clone
mov fx ax

; make a clone
word clone_return
mov ax 16
mov bx .serve_client
mov cx &serve_client
mov dx 0
mov ex &clone_return
sys

; create timeout thread
mov fx ax
mov ax 16
mov bx .timeout_thread
mov cx &timeout_thread
mov dx 0
mov ex &clone_return
sys

; close socket fd in parent
;mov ax 3
;mov bx fx
;sys

jmp accept_loop

:serve_client
;set TERM signal handler
setint 15 .timeout_handler $timeout_handler
;move socket descriptor to fx
mov fx ax
;printuint ax

; allocate new segment for the buffer
mov ax 9
mov bx 0
mov cx 512
mov dx 0
mov ex 0
sys
mov hx -1
cmp ax hx
je close_socket

; set data segment
dseg ax

; read from socket to new data segment
:read_from_socket
mov dx fx
mov ax 0
mov bx 0
mov cx 511
sys
printsint ax
cmp ax hx
je serv_end

; find whitespace
mov dx 0
mov ah ' '
:whitespace_loop1
mov al *dx
cmp ah al
je whitespace_loop1_end
cmp dx cx
jge serv_end
inc dx
jmp whitespace_loop1
:whitespace_loop1_end

;find first character after whitespace
:whitespace_loop2
mov al *dx
cmp ah al
jne whitespace_loop2_end
cmp dx cx
jge serv_end
inc dx
jmp whitespace_loop2
:whitespace_loop2_end

; save position of requested path
push dx

; find first character that is less or equal than whitespace
; or a question mark
mov el '?'
:whitespace_loop3
mov al *dx
cmp al ah
jle whitespace_loop3_end
cmp al el
je whitespace_loop3_end
cmp dx cx
jge serv_end
inc dx
jmp whitespace_loop3
:whitespace_loop3_end

;null-terminate path
mov al 0
mov *dx al

; open file
mov ax 2
mov ex 0
mov bx 0
mov cx 0
;mov dx fx
pop dx
call fix_httpget_path
printstr dx
sys

printsint ax

cmp ax hx
jne file_found

mov ax 2
push ds
dseg bx
mov dx $notfounddoc
sys
printstr dx
pop ds

printsint ax

:file_found

; copy file descriptor to ex
mov ex ax

; send http header
; get length of header
push ds
dseg bx
mov ax $defaultresponse
mov bx ax
call strlen
;length to cx
mov cx ax
mov ax 1
mov dx fx
sys
; send cache-control
mov ax $cacheresponse
mov bx ax
call strlen
mov cx ax
mov ax 1
sys
pop ds


; read from file and write to socket
:serve_loop
mov dx ex
mov ax 0
mov bx 0
mov cx 511
sys
cmp ax hx
je close_file

;printuint ax

mov cx ax
jcz close_file
mov ax 1
mov dx fx
sys

cmp ax hx
je close_file

;printuint ax

; test that everything was sent
:send_loop
cmp ax cx
; if was, read more data from the file
je serve_loop

;else loop until everything is sent
add bx ax
sub cx ax
mov ax 1
sys
cmp ax hx
je close_file
jmp send_loop

:close_file
mov bx ex
mov ax 3
sys

printsint ax

:serv_end
; free memory
mov ax 0
mov bx ds
dseg ax
mov ax 11
sys

:close_socket
; close socket
mov ax 3
mov bx fx
sys

;printuint ax

; exit
:exit
mov ax 12
mov bx 0
sys

:timeout_handler
; if file is open
mov hx 0
mov bx ex
cmp bx hx
je timeout_file_not_open
mov ax 3
sys
printsint ax
:timeout_file_not_open
; close socket
mov bx fx
mov ax 3
sys
printsint ax
jmp exit

:timeout_thread
; AX = server thread
mov hx ax
; get time
mov ax 18
sys
zflags
; increment time by 60 sec and sleep
mov ex 60
add dx ex
; if carry is not set, jump to timeout_pause
jfns 0x80 timeout_pause
; else increment cx
inc cx
:timeout_pause
mov ex dx
mov dx cx
mov cx bx
mov bx ax
mov ax 22
sys
;send TERM signal to server thread
mov ax 23
mov bx hx
mov cx 15
sys

jmp exit

; ax = number
; bx = pointer to buffer
; writes number in ax to buffer in [bx] as base-16
:hextostr
push ax
push bx
push cx
push dx
push ex
push fx

mov dh 'A'
mov dl '0'
mov eh 9
mov el 4

mov cx el

:hextostr_loop

mov fl ah
shr fl el
shl ax el

cmp fl eh
jg hextostr_highnums

add fl dl
jmp hextostr_addchar

:hextostr_highnums
sub fl eh
dec fl
add fl dh

:hextostr_addchar
mov *bx fl
inc bx

dec cx
jcz hextostr_end
jmp hextostr_loop

:hextostr_end
pop fx
pop ex
pop dx
pop cx
pop bx
pop ax
ret

; dx == pointer to path
:fix_httpget_path
push ax
push bx
push cx

mov cx 0

; replace /:s with \:s
push dx
mov ah '/'
:fix_httpget_path_loop1
mov cl *dx
jcz fix_httpget_path_loop1_end
cmp ah cl
jne fix_httpget_path_loop1_1
mov cl '\\'
mov *dx cl
:fix_httpget_path_loop1_1
inc dx
jmp fix_httpget_path_loop1
:fix_httpget_path_loop1_end

; increment pointer while path begins with \
pop dx
;printstr dx
mov ah '\\'
:fix_httpget_path_loop2
mov cl *dx
cmp ah cl
jne fix_httpget_path_loop2_end
jcz fix_httpget_path_loop2_end
inc dx
jmp fix_httpget_path_loop2
:fix_httpget_path_loop2_end

; if path contains drive letter, return empty string
push dx
mov ah ':'
inc dx
mov al *dx
cmp ah al
je fix_httpget_path_return0
pop dx
;printstr dx

; if path contains .., return index document
push dx
mov ah '.'
:fix_httpget_path_loop3
mov cl *dx
jcz fix_httpget_path_loop3_end
cmp ah cl
jne fix_httpget_path_loop_2
inc dx
mov al *dx
cmp ah al
je fix_httpget_path_return0
:fix_httpget_path_loop_2
inc dx
jmp fix_httpget_path_loop3
:fix_httpget_path_loop3_end
pop dx
;printstr dx

; if the path is now empty, return index
mov cl *dx
jcz fix_httpget_path_return1

pop cx
pop bx
pop ax
ret

:fix_httpget_path_return0
pop dx
:fix_httpget_path_return1
push dx
mov ax $index
:copyindexloop
push ds
mov ds 0
mov cl *ax
pop ds
mov *dx cl
jcz copyindexloop_end
inc ax
inc dx
jmp copyindexloop
:copyindexloop_end
pop dx

pop cx
pop bx
pop ax
ret

; AX == pointer to str
; Returns length of the string to AX
:strlen
push cx
push bx
mov bx ax
mov ax 0
mov cx 0
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

