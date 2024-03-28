stack 1
code auto
data auto
interrupts 1

str modeprog "MODE"
str modeargs "COM1 BAUD=4800 PARITY=n DATA=8 STOP=2"
str modepid 2

; tehd„„n sarjaportin asetukset
mov ax 15
mov bx $modepid
mov cx $modeprog
mov dx $modeargs
sys

mov ax 12
mov bx 0
sys
