str filename "testi.txt"
str file_struct 8
str entertext "Enter some text"
str writing "Writing text to file testi.txt"
str file_content 100

; open file $filename
mov dx $filename
mov bx 1
mov ax 2
mov ex $file_struct
sys

mov dx ax

mov fx $entertext
printstr fx

; get str of length 100 to $file_content
mov bx $file_content
mov cx 100
getstr bx cx

mov fx $writing
printstr fx

;write 50 bytes of $file_content to file descriptor
;that is returned to ax by syscall open()
mov bx $file_content
mov cx 50
;mov dx ax
mov ax 1
sys

;close the file descriptor
mov bx dx
mov ax 3
sys
