;pipe test

;make pipe
mov ax 33
sys
word read_end 0
word write_end 0
mov $read_end bx
mov $write_end cx

; make fork
mov ax 17
mov dx 1
mov dx 1
mov ex 0
sys

mov cx ax
jcz read

; parent closes read end
mov bx $read_end
mov ax 3
sys

; parent writes to write end
str string "T„m„ viesti kirjoitetaan putkeen."
mov ax 1
mov bx $string
mov cx 33
mov dx $write_end
sys
jmp end
:read

; child closes write end
mov bx $write_end
mov ax 3
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
:end
; close read end
mov ax 3
mov bx $read_end
sys

; try to write to write end
mov ax 1
mov bx $string
mov cx 33
mov dx $write_end
sys
