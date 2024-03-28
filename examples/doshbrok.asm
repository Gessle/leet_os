;code auto
;data auto
;stack 32
;interrupts 2

; unix-style command interpreter

setint 1 .cli_loop $cli_loop

jmp cli_loop

:debug
;push ax
;str hello "sprölölölö"
;mov ax $hello
;printstr ax
;pop ax
;ret

; puts (write string to fd 1)
; AX == pointer to string
; returns: count of bytes outputted

:puts
  push bx
  push cx
  push dx

  ; calculate string length
  mov bx ax
  mov dh 0
  mov cx 0
  :puts_strlen_loop
  mov dl *bx
  cmp dh dl
  je puts_write
  inc cx
  inc bx
  jmp puts_strlen_loop

  :puts_write
  mov bx ax

  mov ax 1
  mov dx 1
  sys

  str newline "\n"
  mov bx $newline
  mov cx 1
  mov ax 1
  sys

  pop dx
  pop cx
  pop bx
ret


; skip whitespaces
; ax == pointer to string
; returns: pointer to first position
; after whitespaces, if any

:skip_whitespaces
  push bx
  mov bl ' '

  :skip_whitespaces_loop
  mov bh *ax
  cmp bh bl
  jne skip_whitespaces_end
  inc ax
  jmp skip_whitespaces_loop

  :skip_whitespaces_end
  pop bx
ret

; split string into words
; ax = pointer to string
; bx = pointer to separated word
; cl = separator
; return: ax = pointer to next word

:split_str
  push bx
  push cx
  push dx

  mov dl cl
  mov cx 0

  :split_str_loop
  mov cl *ax
  mov *bx cl
  cmp dl cl
  je split_str_end
  jcz split_str_end
  inc ax
  inc bx
  jmp split_str_loop
  :split_str_end

  mov *bx 0
  inc ax

  pop bx
  pop dx
  pop cx
ret




str user_input 256

; fix this: VPU cannot MOV to this!
word command


; get command from user
:cli_loop
mov ax $user_input
mov bx 255
getstr ax bx

mov $command ax

; make fork
word return_value 0
mov ax 17
mov dx 1
mov ex &return_value
sys

mov cx ax
; if this process == parent, wait
jcz end
; else parse command
:parse_command

word pipe_receiver
str program_string 256

mov ax $command
mov bx $program_string
mov cx '|'
call split_str
call skip_whitespaces
mov $pipe_receiver ax

mov ch 0
mov cl *ax
; if no pipe, prepare for executing program
jcz exec_program
; else make pipe

mov ax 33
sys
mov ex bx
mov fx cx

; and another fork that parses next command

mov ax 17
mov dx 1
mov ex &return_value
sys

mov cx ax
; if this process == parent, close pipe's read head
jcz pipe_child
mov bx ex
mov ax 3
sys
; and connect write head to stdout
mov ax 32
mov bx fx
mov cx 1
sys
; and go to executing program
jmp exec_program

; else close pipe's write head
:pipe_child
mov bx fx
mov ax 3
sys
; and connect read head to stdin
mov ax 32
mov bx ex
mov cx 0
sys
; and continue parsing command
mov $command $pipe_receiver
jmp parse_command

:exec_program
str program_name 256
mov cx '<'
mov ax $program_string
mov bx $program_name
call split_str
call skip_whitespaces
; if *ax!=0 user wants to redirect a file to stdin
mov ch 0
mov cl *ax
jcz no_stdin_redir

; open file for reading input
mov dx ax
mov ax 2
mov bx 0
mov cx 0
sys
; dup file to stdin
mov cx 0
mov bx ax
mov ax 32
sys

:no_stdin_redir

mov cx '>'
mov ax $program_string
mov bx $program_name
call split_str
call skip_whitespaces
; if *ax!=0 user wants to redirect output to file
mov ch 0
mov cl *ax
jcz no_stdout_redir

; open file for writing output
mov dx ax
mov ax 2
mov bx 1
mov cx 0
sys
;dup file to stdout
mov cx 1
mov bx ax
mov ax 32
sys

:no_stdout_redir

str executable 13
; get program arguments
mov cx ' '
mov ax $program_name
mov bx $executable
call split_str
call skip_whitespaces

; exec program
mov bx $executable
mov cx ax
mov ax 30
sys
; if still alive, exit with return status 1
mov ax 12
mov bx 1
sys

:end
jmp end
