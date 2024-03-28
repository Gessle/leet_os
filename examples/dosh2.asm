code auto
data auto
stack 32
interrupts 2

; unix-style command interpreter

setint 1 .cli_return $cli_return

jmp cli_loop

:debug
;push ax
;str hello "sprölölölö"
;mov ax $hello
;printstr ax
;pop ax
;ret

; ngets (read max n bytes long string from fd 0)
; ax = pointer to string
; returns: pointer to string
:ngets
  push ax
  push bx
  push cx
  push dx
  push ex
  push fx
  
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
  jne ngets_loop
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
  
  pop fx
  pop ex
  pop dx
  pop cx
  pop bx
  pop ax
ret

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


str user_input 256
word command

:cli_return
iret

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

; remove newlines from user input
mov bx '\n'
call strchr
mov *ax 0
mov ax $user_input
mov bx '\r'
call strchr
mov *ax 0
mov ax $user_input


mov $command ax
; make fork
word return_value 0
mov ax 17
mov dx 1
mov ex &return_value
sys

mov cx ax
; if this process == parent, wait
jcz parse_command
jmp end

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
printstr ax
;call puts
mov ax 12
mov bx 1
sys

:end
wait
jmp cli_loop
