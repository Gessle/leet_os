str filename "testi.txt"
str file_struct 8
str entertext "Enter some text"
str writing "Writing text to file testi.txt"
str file_content 100

mov fx $entertext
printstr fx

; get str of length 100 to $file_content
mov bx $file_content
mov cx 100
getstr bx cx

mov fx $writing
printstr fx

;write 50 bytes of $file_content to file descriptor 0
mov bx $file_content
mov cx 50
mov dx ax
mov ax 1
sys

