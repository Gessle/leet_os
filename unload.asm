stack 16
data auto
code auto
interrupts 1

mov ax 13
sys
dec ax
cmp ax bx
; no arguments
je end

str modname 6

mov cx 0
mov dx $modname
:readarg1_loop
mov ax 14
mov bx 1
sys
cmp ah al
je readarg_loop_end
mov *dx al
inc cx
inc dx
jmp readarg1_loop

:readarg_loop_end
mov ax 47
mov bx $modname
sys

xor bx bx
cmp ax bx
jne error

str unloaded "Module unloaded.\n"
mov ax 1
mov bx $unloaded
mov cx 17
mov dx 1
sys
jmp end

:error
mov bx 1
cmp ax bx
jne nosuchmodule

str unsupport "Module is now inactive, but still in memory.\n"
mov ax 1
mov bx $unsupport
mov cx 45
mov dx 1
sys
jmp end

:nosuchmodule
str nomodule "No such module.\n"
mov ax 1
mov bx $nomodule
mov cx 16
mov dx 1
sys

:end
mov ax 12
mov bx 1
sys
