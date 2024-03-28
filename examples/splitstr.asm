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
inc ax
inc bx
cmp dl cl
je split_str_end
jcz split_str_end
jmp split_str_loop
:split_str_end

pop dx
pop cx
