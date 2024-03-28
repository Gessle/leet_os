interrupts 2
stack 16
data auto
code auto

;catcom (print serial port input to stdin)

setint 1 .int_handler $int_handler

; get the serial port interrupt to signal 1
mov ax 36
mov bx 0xC
mov cx 0
mov dx 0
sys

; open the serial port 1 for reading
str serialport "COM1"
word serial_desc
mov ax 2
; data 8, stop 1, odd, 115200 baud, rts/cts
mov bx 0b1000110000001011
mov cx 0
mov dx $serialport
mov ex 0
sys
mov $serial_desc ax

; read from serial port
:loop
str buff 100
mov bx $buff
mov ax 0
mov cx 100
mov dx $serial_desc
sys

; write to stdout
mov cx ax
jcz end
mov dx 1
mov ax 1
sys

mov ax 36
mov bx 0xC
mov cx 1
mov dx 1
sys
jmp loop

:int_handler
mov ax 37
mov bx 0xC
sys
iret

:end
mov ax 12
mov bx 0
sys