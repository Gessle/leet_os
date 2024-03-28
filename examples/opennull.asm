; load NULL device
str device "DEV:\\null"

mov ax 2
mov bx 0
mov dx $device
sys

; read 100 bytes
mov dx ax
mov bx 0
mov ax 0
mov cx 200
sys
