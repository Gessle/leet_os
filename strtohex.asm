str luku "1234"

jmp main


; ax = pointer to string containing the base-16 number
; returns the number as 16-bit unsigned int
:strtohex
push bx
push cx
push dx
push ex
push fx

mov bh '9'
mov bl '0'
mov ch 'F'
mov cl 'A'
mov fl 10

mov dx 0
mov el 4

:strtohex_loop

mov eh *ax
inc ax
cmp eh bl
jl strtohex_end
cmp eh bh
jg strtohex_highnums

sub eh bl
shl dx el
or dx eh
jmp strtohex_loop

:strtohex_highnums
cmp eh cl
jl strtohex_end
cmp eh ch
jg strtohex_end

sub eh cl
add eh fl
shl dx el
or dx eh
jmp strtohex_loop

:strtohex_end
mov ax dx

pop fx
pop ex
pop dx
pop cx
pop bx
ret

:main
mov ax $luku
call strtohex