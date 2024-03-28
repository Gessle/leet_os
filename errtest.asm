; error code test

str errorcode "Error code:"
str file "testi.txt"
word fd

; open file
mov ax 2
mov bx 0
mov cx 0
mov dx $file
mov ex 0
sys

mov $fd ax

printuint ax
str fileopened "Opened file."
mov ax $fileopened
printstr ax

mov ax 38
sys
mov bx $errorcode
printstr bx
printuint ax
