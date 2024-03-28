stack 16
code auto
data auto
interrupts 2

;sleep test
str sleeping "Sleeping..."
str wokeup "Good morning!"
mov ax 18
sys

mov ex dx
mov dx cx
mov cx bx
mov bx ax

mov ax $sleeping
printstr ax
mov ax 2
add ex ax
mov ax 22
sys

mov ax $wokeup
printstr ax
wait