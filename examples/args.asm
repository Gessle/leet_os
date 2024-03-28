stack 16
code 1 512
data 1 512
interrupts 1

str count "Argument count:"
str fetching "Fetching argument:"
mov ax $count
printstr ax

; get argument count to BX
mov ax 13
sys
mov bx ax
printuint bx

str buff 100
; get argument char to $buff
:loop
mov ax $fetching
printstr ax
dec bx
printuint bx
mov dx $buff
mov cx 0
:nextchar
mov ax 14
sys
mov *dx al
cmp ax ex
je nextarg
inc dx
inc cx
jmp nextchar
:nextarg
mov dx $buff
printstr dx
cmp bx ex
je end
jmp loop
:end

mov ax 12
mov bx 0
sys
