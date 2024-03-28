code auto
data auto
stack 32
interrupts 128

;  UNIX-style command interpreter
;  version 0.6
;  SET command added
;

setint 18 .sig_handler $sig_handler
setint 2 .sigint_handler $sigint_handler

str cd_command "cd"
str set_command "set"
byte newline 0x0A

jmp main

:sigint_handler
; send sigint to subprocess
push ax
push bx
push cx

mov ax 0
mov bx $child_pid
cmp ax bx
je sigint_handler_end
mov ax 23
mov cx 2
sys
:sigint_handler_end

pop cx
pop bx
pop ax
iret

:sig_handler
mov $child_pid 0
iret

byte prompt_char '>'

str command 512
; read line from stdin
; returns line length
; exits program if EOF
:read_input_line
  push bx
  push cx
  push dx
  push ex
  push fx
  
  ; zero error code
  mov ax 39
  mov bx 0
  sys
  
  mov el 0xA  
  mov bx $command
  mov cx 1
  mov dx 0
  :read_input_line_loop
  mov ax 0
  sys
  push ax
  ; check error code
  mov ax 38
  sys
  mov fx 0x106
  cmp ax fx
  ; interrupted by a signal
  pop ax
  je read_input_line_end
  
  mov fx 0
  cmp ax fx
  je exit_program
  mov eh *bx
  cmp eh el
  je read_input_line_end
  inc bx    
  jmp read_input_line_loop
  
  :read_input_line_end
  dec bx
  mov eh *bx
  mov el 0x0D
  cmp eh el
  je read_input_dos_newline
  inc bx

  :read_input_dos_newline
  mov *bx 0
  mov ax $command
  sub bx ax
  mov ax bx

  :read_input_line_return
  pop fx
  pop ex
  pop dx
  pop cx
  pop bx
ret

:exit_program
  mov ax 12
  mov bx 0
  sys

; bx = program name + arguments
:exec_program
  mov cl 0
  mov al ' '

  :exec_program_loop2
  mov ch *bx
  cmp ch al
  jne exec_program_loop2_end
  inc bx
  jmp exec_program_loop2

  :exec_program_loop2_end
  push bx
  :exec_program_loop1
  inc bx
  mov ch *bx
  jcz exec_program_noargs
  cmp ch al
  jne exec_program_loop1
  
  mov *bx cl
  inc bx
  
  :exec_program_noargs  
  mov cx bx  
  pop bx
  mov ax 30
  sys
  
  ; if still alive...
  str unknowncommand "Unknown command.\n"
  mov ax 1
  mov bx $unknowncommand
  mov cx 17
  mov dx 1
  sys
  mov ax 12
  mov bx 1
  sys

; copy characters from bx to ax until whitespace
; return end of word
:copy_word
  push bx
  push cx
  push dx
  
  mov ch 0
  mov dl ' '
  :copy_word_loop
  mov cl *bx
  jcz copy_word_end
  cmp dl cl
  je copy_word_end
  mov *ax cl
  inc bx
  inc ax
  jmp copy_word_loop

  :copy_word_end
  mov cl 0
  mov *ax 0

  mov ax bx
  
  pop dx
  pop cx
  pop bx
  ret
  
; bx = command
word redirect_file_mode 1
str output_file_name 13
str input_file_name 13
:redirect_stdio_file
  push ax
  push bx
  push cx
  push dx
  push ex
  push fx
  
  :redirect_stdio_init
  mov cx 0
  mov al '>'
  mov fl '<'
  mov ah ' '
  :redirect_stdio_loop1
  inc bx    
  mov cl *bx
  jcz redirect_stdio_end
  cmp cl al
  je redirect_stdout
  cmp cl fl
  je redirect_stdin
  jmp redirect_stdio_loop1  

  :redirect_stdout

  ; null-terminate command
  mov *bx ch
  
  :redirect_stdout_loop2
  inc bx
  mov cl *bx
  cmp cl al
  je redirect_stdout_append
  cmp cl ah
  je redirect_stdout_loop2
  jmp redirect_stdout_open_file
  
  :redirect_stdout_append
  mov $redirect_file_mode 2
  jmp redirect_stdout_loop2
  
  :redirect_stdout_open_file
  ; now bx == output file name
  mov ax $output_file_name
  call copy_word
  push ax
  mov ax 2
  mov dx $output_file_name
  mov bx $redirect_file_mode
  mov cx 0
  mov ex 0
  sys
  
  ; redirect stdout to the new file descriptor
  mov bx ax
  mov ax 32
  mov cx 1
  sys
  ; close the file descriptor
  mov ax 3
  sys  
  pop bx
  jmp redirect_stdio_init
  
  :redirect_stdin
  ; null-terminate command
  mov *bx ch  
  :redirect_stdin_loop2
  inc bx
  mov cl *bx
  cmp cl ah
  je redirect_stdin_loop2

  ; now bx == input file name
  mov ax $input_file_name
  call copy_word
  push ax
  mov ax 2
  mov dx $input_file_name
  mov bx 0
  mov cx 0
  mov ex 0
  sys
  ; redirect stdin from the new file descriptor
  mov bx ax
  mov ax 32
  mov cx 0
  sys
  ; close the file descriptor
  mov ax 3
  sys
  pop bx
  jmp redirect_stdio_init  
  
  :redirect_stdio_end
  pop fx
  pop ex
  pop dx
  pop cx
  pop bx
  pop ax
  ret

:use_spawn
  mov al 0
  mov ah ' '
  mov *bx al
  mov bx $command
  mov dx 0
  mov cx 0
  
  :use_spawn_loop1
  inc bx
  mov cl *bx
  jcz use_spawn_noargs
  cmp cl ah
  je use_spawn_args
  jmp use_spawn_loop1
  
  :use_spawn_args
  mov *bx al
  inc bx
  mov dx bx  
  
  :use_spawn_noargs
  mov ax 15
  mov bx &child_pid
  mov cx $command
  sys

  pop ax  
  jmp get_command  
  
:remove_last_char
mov al 0
mov *bx al
jmp do_fork

:change_dir
add bx 3
mov ax 4
sys
mov bx 0
cmp ax bx
jne get_command
str directorychgd "Directory changed.\n"
mov bx $directorychgd
mov ax 1
mov dx 1
mov cx 19
sys
jmp get_command

:wait_for_sigchld
mov $child_pid ax
; if last char was &, get new command
mov al $cmd_last_char
mov ah '&'
cmp al ah
je get_command
:wait_for_sigchld_2
wait
mov ax $child_pid
mov bx 0
cmp ax bx
jne wait_for_sigchld_2

:main

:get_command

; print prompt and flush stdout
str prompt "READY.\n"
mov bx $prompt
mov ax 1
mov dx 1
mov cx 7
sys

; print working directory
str cwd 256
mov ax 45
mov bx $cwd
mov cx 256
sys
mov cx ax
mov ax 1
mov dx 1
sys

mov ax 1
mov bx &prompt_char
mov cx 1
mov dx 1
sys
mov ax 27
mov bx 1
sys

call read_input_line
mov cx 1

; if empty command, repeat
cmp ax cx
jle get_command

word command_len
mov $command_len ax

; if cd command
;mov cl *bx
;mov ch 'c'
;cmp cl ch
;jne get_last_char
;inc bx
;mov ch 'd'
;mov cl *bx
;cmp cl ch
push ax
mov ax $cd_command
mov bx $command
call first_wd_eq
cmp ax 0
je change_dir
;dec bx
; if set command
mov ax $set_command
call first_wd_eq
cmp ax 0
je parse_set
pop ax

:get_last_char
; get last char
byte cmd_last_char
mov bx $command
add bx ax
dec bx
mov $cmd_last_char *bx

push ax

mov al $cmd_last_char
mov ah '&'
cmp al ah
je remove_last_char
mov ah '%'
cmp al ah
je use_spawn

:do_fork

; else fork
word fork_return
mov ax 17
mov ex &fork_return
mov dx 18
sys
mov bx 0
cmp ax bx
; if this process = parent, wait for child
word child_pid
;mov $child_pid ax
jne wait_for_sigchld
pop ax

:parse_command
mov bx $command
add bx ax
dec bx

; decrement bx until at beginning of the command or pipe
mov dx $command
mov cl '|'
:command_dec1_loop
dec bx
mov ch *bx
cmp bx dx
je run_program
cmp ch cl
je run_program_pipe
jmp command_dec1_loop

:run_program
add bx ax
mov cl 0
mov *bx cl
sub bx ax
call redirect_stdio_file
call exec_program

:run_program_pipe
push bx
; make pipe
word pipe_read
word pipe_write
mov ax 33
sys
mov $pipe_read bx
mov $pipe_write cx
;redirect stdin from pipe_read
mov cx 0
mov ax 32
sys
;make another fork
mov ax 17
mov ex &fork_return
mov dx 0
sys
cmp ax dx
; if this process = parent, parse command after pipe operator
jne pipe_parent 
; else redirect stdout to pipe_write
mov bx $pipe_write
mov cx 1
mov ax 32
sys
; and close pipe's read head
mov ax 3
mov bx $pipe_read
sys
; null-terminate command
pop bx
mov *bx dl
; and continue parsing command from the beginning
mov ax $command
sub bx ax
mov ax bx
mov $command_len ax
jmp parse_command

:pipe_parent
; close pipe's write head
mov ax 3
mov bx $pipe_write
sys

pop bx
inc bx
mov ax $command
mov cx $command_len
add ax cx
sub ax bx

jmp run_program

jmp main

; ax = compare string
; bx = command
:first_wd_eq
push bx
push cx

:first_wd_eq_loop
mov cl *ax
mov ch *bx
cmp cl 0
je first_wd_eq_cmpz
;je first_wd_eq_nodiff
cmp cl ch
jne first_wd_eq_diff
inc ax
inc bx
jmp first_wd_eq_loop
:first_wd_eq_cmpz
cmp ch ' '
jne first_wd_eq_cmpz2
:first_wd_eq_nodiff
xor ax ax
jmp first_wd_eq_end

:first_wd_eq_diff
mov ax -1
:first_wd_eq_end
pop cx
pop bx
ret
:first_wd_eq_cmpz2
cmp ch 0
je first_wd_eq_nodiff
jmp first_wd_eq_diff

:parse_set
add bx 3
mov al *bx
cmp al 0
je list_envvars

inc bx
call set_env

jmp get_command

:list_envvars
xor bx bx

:list_envvars_loop
mov ax 48
mov dx $command
mov cx 512
sys
cmp ax -1
je get_command
; write to stdout
mov ax dx
call strlen
mov cx ax
mov ax 1
push bx
mov bx dx
mov dx ax
sys
mov ax 1
mov bx &newline
mov cx 1
sys
pop bx
inc bx
jmp list_envvars_loop
 
:strlen
push bx
push cx
push dx
mov bx ax
push ax
mov ax ds
mov cx -1
xor dl dl
memchr
pop ax
dec bx ax
mov ax bx
pop dx
pop cx
pop bx
ret

:set_env
push ax
push cx
push dx

push bx

mov cx 508
mov dl '='
mov ax ds
memchr
jcz setenv_end

mov dl 0
mov *bx dl

mov cx bx
inc cx
pop bx
;cx == environment variable value
;bx == environment variable name
mov dx 1
mov al *cx
cmp al 0
jne setenv_callsys
inc dx
:setenv_callsys
mov ax 49
sys

:setenv_end
pop dx
pop cx
pop ax
ret
