; test program for scrollbars
setint 1 .int1_handler $int1_handler
jmp main

:int1_handler
in 1
printuint ax
in 2
printuint ax
iret

:main
mov ax 0
mov bx 400
mov cx 400
str title "Vierityspalkki-ikkuna"
mov dx $title
mov ex 1
mov fx 1
mov gx 0
scr

mov ex ax
mov ax 11
mov bx 10
mov cx 10
mov dx 2
mov fx 200
mov gx 1000
mov hx 2
scr

:loop
inc ax
out 2
push ax
mov ax 3
scr
pop ax
jmp loop