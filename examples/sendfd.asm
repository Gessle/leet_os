; send fd to process

; first spawn the new program
word pid
str executable "recvfd.app"
str args ""
mov ax 15
mov bx &pid
mov cx $executable
mov dx $args
sys

; pid of the new process is now in $pid
; so let's open a file

str filename "testi.txt"
word fd
mov ax 2
mov bx 0
mov cx 0
mov dx $filename
mov ex 0
sys
mov $fd ax

; send the file descriptor to the spawned process
:fd_send_loop
mov cx $fd
mov ax 35
mov bx $pid
sys
; if ax == -1, retry until it is 0
mov cx ax
jcz end
jmp fd_send_loop
:end
wait






