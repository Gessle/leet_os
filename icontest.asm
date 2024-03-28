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
str title "Ikoni-ikkuna"
mov dx $title
mov ex 1
mov fx 1
mov gx 0
scr

str icon "icons\\computer.ico"
str teksti "Ikonitesti"
mov ex ax
mov ax 12
mov bx 10
mov cx 10
mov fx $icon
mov dx $teksti
scr

:loop
jmp loop