interrupts 1
stack 16
data auto
code auto

;catcom (print serial port input to stdin)

; get the serial port interrupt to signal 0
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
mov bx $buff
:loop
str buff 100
mov ax 0
mov cx 100
mov dx $serial_desc
sys

; write to stdout
mov cx ax
mov dx 1
mov ax 1
sys
jmp loop
