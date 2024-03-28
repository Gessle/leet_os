str buff 100

word com_fp
str com "COM1"

mov ax 2
mov bl 0b00001011
mov bh 0b01000111
mov dx $com
sys

mov dx ax

:loop
mov bx $buff
mov cx 100
getstr bx cx
mov ax 1
sys
jmp loop