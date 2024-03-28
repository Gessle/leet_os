; unix-style command interpreter

setint 1 .cli_loop $cli_loop

jmp cli_loop


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
jne skip_whitespaces_loop

pop bx
ret

; split string into words
; ax = pointer to string
; bl = separator
; return: ax = pointer to next word
;         bx = pointer to separated word

str split_str_word 100
:split_str
push cx
push dx

mov dl bl
mov cx 0

:split_str_loop
mov bx $split_str_word
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
mov bx $split_str_word

pop dx
pop cx
ret



str command 256
word args
word program_name
str unknowncommand "Unknown command."

:exec_program
str executing "Executing command"
mov ax $executing
printstr ax
mov ax 30
mov bx $program_name
mov cx $args
sys
mov bx ax
mov ax 12
sys



:cli_loop
mov ax $return_value
mov bx 0
cmp ax bx
je getcommand
mov ax $unknowncommand
printstr ax
:getcommand
mov ax $command
mov bx 255
getstr ax bx

;split command into program path and arguments
mov bl ' '
call split_str
mov $program_name bx
mov $args ax

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
mov bx $program_name
jmp exec_program


:end
jmp end
