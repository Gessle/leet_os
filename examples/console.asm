code auto
data auto
stack 32
interrupts 2

; unix-style command interpreter

setint 1 .cli_loop $cli_loop

jmp cli_loop

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


:parse_command
  push bx
  push cx
  push dx
  push ex
  mov cl '>'
  mov ax $command
  mov bx $program_name
  call split_str
  ; if *AX != 0, open file for stdout redirection
  ; skip any whitespaces after possible '>'
  call skip_whitespaces
  ; save pointer to filename to $output_file
  mov $output_file ax
  :pc_no_stdout_redirection
  ; split command into program name and arguments
  mov ax $program_name
  mov cl ' '
  mov bx $program_path
  call split_str
  call skip_whitespaces
  mov $args ax
  :parsecommand_end
  pop ex
  pop dx
  pop cx
  pop bx
ret

str command 256
word args
str program_name 256
str program_path 256
str unknowncommand "Unknown command."
str executing "Executing command"
word output_file

:exec_program
  ; open file for output
  mov dx $output_file
  mov ch 0
  mov cl *dx  
  jcz exec_program_stdout  
  mov ax 2
  mov bx 1
  mov cx 0  
  sys  
  ; duplicate opened file to stdout
  mov bx ax  
  mov ax 32
  mov cx 1
  sys
  ; stdout should now be redirected to file.  
  :exec_program_stdout
  mov ax 30
  mov bx $program_path
  mov cx $args
  sys
  mov cx ax
  jcz exec_end
  mov ax $unknowncommand
  call puts
  :exec_end
  mov ax 12
sys



:cli_loop
mov ax $command
mov bx 255
getstr ax bx

mov ax $executing
call puts

call parse_command

; make fork
word return_value 0
mov ax 17
mov dx 1
mov ex &return_value
sys

mov bx 0
cmp ax bx
; if this process == parent, wait
jne end

; else execute command
;mov bx $program_name
mov ex 0
jmp exec_program


:end

:end2
jmp end2
wait
