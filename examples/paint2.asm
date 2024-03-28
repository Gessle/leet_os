stack 16
interrupts 5
data auto
code auto

; This is a code example for virtual machine console

str title "Uusi ikkuna"

setint 2 .hiiri_handler $hiiri_handler

jmp main

:hiiri_handler
push ax
push bx
push cx
push dx
push ex
in 2
mov bx ax
in 3
mov cx ax
dec cx
mov dx 0
mov ax 5
mov ex $ikkuna_desc
scr
mov ax $title
printstr ax
pop ex
pop dx
pop cx
pop bx
pop ax
iret

:main

; luodaan ikkuna
mov ax 0
mov bx 400
mov cx 200
mov dx $title
mov ex 0
mov fx 0
mov gx 2
mov hx 2
scr

word ikkuna_desc
mov $ikkuna_desc ax

:end
wait
jmp end