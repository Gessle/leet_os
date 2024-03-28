;pipe test

;make pipe
mov ax 33
sys
word read_end 0
word write_end 0
mov $read_end bx
mov $write_end cx

; write to write end
str string "T„m„ viesti kirjoitetaan putkeen."
mov ax 1
mov bx $string
mov cx 33
mov dx $write_end
sys

; read from read end
str readstring 100
mov ax 0
mov bx $readstring
mov cx 100
mov dx $read_end
sys

mov ax $readstring
printstr ax

; close write end
mov ax 3
mov bx $write_end
sys

; try to read from read end
mov ax 0
mov bx $readstring
mov cx 33
mov dx $read_end
sys
