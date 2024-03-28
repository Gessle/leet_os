;mmap test
word dynamic_segment 5

; open file
str filename "testi.txt"
str file_struct 8
mov dx $filename
mov ax 2
mov bx 0
mov ex $file_struct
sys

; print fd
printuint ax

; allocate 50 bytes and map file contents there
; file descriptor to ex
mov ex ax
; syscall 9
mov ax 9
; try to allocate segment $dynamic_segment
mov bx $dynamic_segment
; length 50 bytes
mov cx 50
; from file beginning
mov fx 0
sys

dseg ax
mov ax 0
printstr ax

; free memory
mov bx 5
mov ax 11
sys