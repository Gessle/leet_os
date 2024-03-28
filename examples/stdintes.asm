str filename "testi.txt"
str message "File contents:"
str file_struct 8
str file_content 100

mov ax $message
printstr ax

; read 50 chars to $file_content from file descriptor 1
mov bx $file_content
mov cx 50
mov dx 1
mov ax 0
sys

; print content
printstr bx