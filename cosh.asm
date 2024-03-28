code auto
data auto
stack 32
interrupts 128

; ;;;;;;;;;;;;;; REDIRECT STDIO TO COM1 ;;;;;;;;;;;;

; 8 databitti„, 1 stopbitti„, pariton pariteetti
word sarjaportti_tila 0b00001011
; 4800 baud
byte baudit 7
; xon/xoff vuonohjaus
byte vuonohjaus 0b01

mov al 6
mov ah $vuonohjaus
shl ah al
mov al $baudit
or ah al
mov al $sarjaportti_tila

mov $sarjaportti_tila ax

; avataan sarjaportti
str sarjaportti "COM1"
mov ax 2
mov bx $sarjaportti_tila
mov dx $sarjaportti
sys
word com_fp 0
mov $com_fp ax

mov ax 32
mov bx $com_fp
mov cx 0
sys
mov ax 32
mov bx $com_fp
mov cx 1
sys


; ;;;;;;;;;;;;;;; STDIO REDIRECTED ;;;;;;;;;


; unix-style command interpreter
; v. 0.3

setint 18 .cli_return $cli_return

; command used to change working directory
str cd_command "cd"

jmp cli_loop

:debug
;push ax
;str hello "sprölölölö"
;mov ax $hello
;printstr ax
;pop ax
;ret

; write a string to stdout (file descriptor 1)
; arguments: AX == pointer to str
; returns: non-negative if success, -1 if error
;
; this procedure checks the number of written bytes
:puts
  push bx
  push cx
  push dx

  mov bx ax

  :puts_strlen_start
  push bx

  ; get length of the string to cx
  mov ax 0
  mov cx 0
  :puts_strlen_loop
  mov ah *bx
  cmp ah al
  je puts_strlen_end
  inc cx
  inc bx
  jmp puts_strlen_loop
  :puts_strlen_end  
  pop bx

  jcz puts_end
  ; write
  mov ax 1
  mov dx 1
  sys

  ; compare str length and count of written bytes
  cmp ax cx
  je puts_end
  ; if not equal, increment bx by the write count
  add bx ax
  ; and continue writing
  jmp puts_strlen_start


  :puts_end

  ; print newline
  str puts_newline "\n"
  mov bx $puts_newline
  mov ax 1
  mov dx 1
  mov cx 1
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
  je split_str_end1
  jcz split_str_end2
  inc ax
  inc bx
  jmp split_str_loop
  :split_str_end1
  inc ax
  :split_str_end2
  mov *bx 0

  pop bx
  pop dx
  pop cx
ret

; ngets (read max n bytes long string from fd 0)
; ax = pointer to string
; returns: pointer to string
:ngets
  push bx
  push cx
  push dx
  push ex
  push fx
  push gx

  mov gx ax

  mov ex 0
  mov fx bx
  dec fx

  mov bx ax

  :ngets_loop
  mov cx 1
  mov dx 0
  mov ax 0
  sys
  cmp ax cx
  jne ngets_returnnull
  mov dh *bx   
  add ex ax
  add bx ax

  ; check if newline
  mov dl '\n' 
  cmp dh dl
  je ngets_end
  mov dl '\r'
  cmp dh dl
  je ngets_end

  ; check count of bytes read
  cmp ex fx
  je ngets_end

  jmp ngets_loop

  :ngets_end
  ; terminate string with null byte  
  mov *bx 0
  mov ax gx
  jmp ngets_returnstr

  :ngets_returnnull
  mov ax 0
  :ngets_returnstr

  pop gx
  pop fx
  pop ex
  pop dx
  pop cx
  pop bx
ret


; ax == pointer to str
; bl == char to find
; returns: pointer to first occurence, NULL if not any
:strchr
  push cx

  mov cx 0

  :strchr_loop
  mov cl *ax  
  cmp cl bl
  je strchr_end
  jcz strchr_end2
  inc ax  
  jmp strchr_loop
  :strchr_end2
  mov ax 0

  :strchr_end
  pop cx
ret

:is_cd_command
push bx
push cx

mov bx $cd_command

:is_cd_command_loop
mov ch 0
mov cl *bx
jcz is_cd_command_true

mov ch *ax

cmp ch cl
jne is_cd_command_false

inc ax
inc bx
jmp is_cd_command_loop

:is_cd_command_true

mov ch *ax
mov cl ' '
cmp ch cl
jne is_cd_command_false

inc ax
jmp is_cd_command_end

:is_cd_command_false
mov ax 0
:is_cd_command_end

pop cx
pop bx
ret

:change_dir
str dirchgd "Directory changed."
mov bx ax
mov ax 4
sys
mov ah 0
cmp ah al
jne change_dir_end
mov ax $dirchgd
call puts
:change_dir_end
ret


str user_input 256
word command

:cli_return
iret

:wait_for_signal
wait

; get command from user
:cli_loop
mov ax $user_input
mov cx 256

:zero_command_loop
mov *ax 0
dec cx
jcz end_zero_command_loop
inc ax
jmp zero_command_loop
:end_zero_command_loop

mov ax $user_input
mov bx 255
;getstr ax bx
call ngets
; if 0 bytes read from stdin, exit
mov cx ax
jcz exit
; remove newline
:command_removenl_loop
mov bl '\n'
mov bh '\r'
mov al *cx
cmp al bl
je command_removenl
cmp al bh
je command_removenl
inc cx
jmp command_removenl_loop
:command_removenl
mov *cx 0
mov ax $user_input
cmp ax cx
je cli_loop

call is_cd_command
mov ah 0
cmp ah al
je no_dir_change
call change_dir
jmp cli_loop
:no_dir_change

;mov $command ax
mov $command $user_input
; make fork
word return_value 0
mov ax 17
mov dx 18
mov ex &return_value
sys

mov cx ax
; if this process == parent, wait
jcz parse_command
jmp wait_for_signal

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
mov fx bx
mov gx cx

; and another fork that parses next command

mov ax 17
mov dx 1
mov ex &return_value
sys

mov cx ax
; if this process == parent, close pipe's read head
jcz pipe_child
mov bx fx
mov ax 3
sys
; and connect write head to stdout
mov ax 32
mov bx gx
mov cx 1
sys
; and go to executing program
jmp exec_program

; else close pipe's write head
:pipe_child
mov bx gx
mov ax 3
sys
; and connect read head to stdin
mov ax 32
mov bx fx
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

str file_name 13
mov bx $file_name
mov cl ' '
call split_str
;mov dx bx

;str redirectingfrom "Redirecting stdin from:"
;mov ax $redirectingfrom
;printstr ax
;mov ax $file_name
;printstr ax

; open file for reading input
mov dx $file_name
mov ax 2
mov bx 0
mov cx 0
mov ex 0
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

mov bx $file_name
mov cl ' '
call split_str
;mov dx bx

;str redirectingto "Redirecting stdout to:"
;mov ax $redirectingto
;printstr ax
;mov ax $file_name
;printstr ax


; open file for writing output
mov dx $file_name
mov ax 2
mov bx 1
mov cx 0
mov ex 0
sys
;dup file to stdout
mov cx 1
mov bx ax
mov ax 32
sys

:no_stdout_redir


word args
str executable 13
; get program arguments
mov cx ' '
mov ax $program_string
mov bx $executable
call split_str
call skip_whitespaces
; ax is now args with stdio redirections
; let's get rid of stdio redirections

mov $args ax
mov bl '>'
call strchr
mov cx ax
jcz no_args_stdout
mov *ax 0
:no_args_stdout

mov ax $args
mov bl '<'
call strchr
mov cx ax
jcz run_program
mov *ax 0


:run_program
; exec program
mov bx $executable
mov cx $args
mov ax 30

sys
; if still alive, exit with return status 1
str unknowncmd "Unknown command."
mov ax $unknowncmd
;printstr ax
call puts
:exit
mov ax 12
mov bx 1
sys

;:end
;wait
;jmp cli_loop

