mov ax 0
mov bx 200
mov cx 200
str title "Ikkuna"
mov dx $title
scr


mov ex ax
mov ax 28
mov bx 20
mov cx 20
str label "1 Radiobutton"
mov dx $label
scr

mov gx bx
mov bl '2'
mov *dx bl
mov ax 28
mov bx 20
mov cx 40
scr

mov gx bx
mov bl '3'
mov *dx bl
mov ax 28
mov bx 20
mov cx 60
scr
