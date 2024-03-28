code auto
data auto
interrupts 3
stack 16
; mutex test

jmp main

:child_died
str kuoli "<is„nnyys prosessi> Lapsiprosessi kuoli"
str paluuarvolla "<is„nnyys prosessi> paluuarvo:"
push ax
push bx
mov ax $kuoli
printstr ax
mov ax $paluuarvolla
printstr ax
mov ax $child_retval
mov bx *ax
printsint bx
pop bx
pop ax 
;wait
iret

; proseduuri jota klooni suorittaa
:clone_proc
mov ax 23
mov bx 0
mov cx 130
sys

str hello "<child process> hello wordrl"
mov ax $hello
mov cx 10
:clone_loop
dec cx
printstr ax
printuint cx
jcz suicide
jmp clone_loop
:suicide
str goodbye "<child process> goodbye cruel world"
mov ax $goodbye
printstr ax
; exit with value 5332
mov ax 12
mov bx 5332
sys


:main
str child_retval 2
; tehd„„n uusi s„ie. kuollessaan s„ie trigger”i is„nt„pros
;essissa keskeytyksen 2
mov ax 16
mov bx .clone_proc
mov cx &clone_proc
mov dx 2
mov ex $child_retval
sys
printuint ax
mov ax 16
sys
printuint ax
mov ax 16
sys
printuint ax
mov bx 0xFFFF
cmp ax bx
jne noerror
str error "Error"
mov ax $error
printstr ax
wait
:noerror
setint 2 .child_died $child_died
:mainloop
; tehd„„n mutex-lukko
mov ax 23
mov bx 0
mov cx 130
sys

str child_alive "<is„nnyys prosessi> viel„ n„ytt„„ el„v„n"
mov ax $child_retval
mov bx *ax
cmp bx gx
jne end
mov ax $child_alive
printstr ax
jmp mainloop
:end
wait

