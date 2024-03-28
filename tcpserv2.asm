stack 1
interrupts 1
data 1 150
code auto

; this program keeps TCP/IP services running
; this is interrupt-triggered ersion, only works if network card interrupt is
; set correctly in line 20

; hide console
mov ax 23
mov bx 0
mov cx 128
sys

; set network cards hardware interrupt to this program
mov ax 36
; *** CHANGE THIS IF NECESSARY! ***
; THIS IS THE INTERRUPT OF YOUR NETWORK CARD
mov bx 72
; sigcont
mov cx 25
mov dx 1
sys

; set priority to 7 (length of loop)
mov ax 26
mov bx 0
mov cx 7
sys


str devname "sock0"
str args_service "\0"


mov bx $devname
mov cx $args_service
xor dx dx
jmp service_loop

:service_loop_nosleep
nop
nop
:service_loop
mov ax 29
sys

cmp ax dx
jne service_loop_nosleep

mov ax 46
sys

jmp service_loop
