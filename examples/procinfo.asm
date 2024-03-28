;stack 16
;data auto
;code auto
;interrupts 1

;list process info

str processname 100
str pid "PID:"
str priority "Priority:"
mov ex $pid
mov fx $priority

mov bx 0
:loop
mov ax 24
mov cx $processname
sys
cmp ax dx
je end
printstr ex
printuint ax
printstr cx
printstr fx
push bx
mov bx ax
mov ax 25
sys
printuint ax
pop bx
inc bx
jmp loop
:end

mov ax 12
mov bx 0
sys
