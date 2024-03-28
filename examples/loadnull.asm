; load NULL module

str driver "drivers\\null.exe"
str name "null"

; load module
mov ax 28
mov bx $driver
mov cx $name

sys

;wait

str loaded "NULL module loaded. Now opening DEV:\\null"
mov ax $loaded
printstr ax

str devname "DEV:\\null"

mov ax 2
mov bx 5
mov dx $devname
sys

; read 100 bytes
mov dx ax
mov bx 0
mov ax 0
mov cx 200
sys

