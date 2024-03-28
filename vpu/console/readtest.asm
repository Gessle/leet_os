str filename "testi.txt"
str message "File contents:"
str file_struct 8
str file_content 100

mov ax $message
printstr ax

; open file $filename in mode 0 (read)
mov dx $filename
mov bx 0
mov ax 2
mov ex $file_struct
sys

; read 50 chars to $file_content from file descriptor returned
mov bx $file_content
mov cx 50
mov dx ax
mov ax 0
sys

; print content
printstr bx