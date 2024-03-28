code 1 256
data 1 256
stack 16
interrupts 1

; soundcheck
mov ax 19
mov dx 1
mov ex 12000
:begin
mov bx 0
:loop
add bx dx
;dec cx
sys
;jcz end
cmp bx ex
jge begin
;mov ax 21
;sys
;mov ax 19
jmp loop
:end
mov ax 20
sys
