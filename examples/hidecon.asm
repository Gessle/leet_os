stack 1
code auto
data auto
interrupts 1

; hide console window
mov ax 23
mov bx 0
mov cx 128
sys
wait
