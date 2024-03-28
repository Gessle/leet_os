stack 1
interrupts 1
code auto
data auto

; avataan sarjaportti
str sarjaportti "COM1"
word com_desc 0
mov ax 2
mov dx $sarjaportti
; baud 4800
mov bh 7
mov bl 0x10000000
or bh bl
; pariteetti 0, data 8, stop 2
mov bl 0b00000111
sys
mov $com_desc ax

; avataan tiedosto kirjoitustilassa
str tiedosto "mario.exe"
word file_desc 0
mov ax 2
mov dx $tiedosto
mov bx 1
sys
mov $file_desc ax

str buff 100

; luetaan puskuriin 100 tavua sarjaportista
;mov bx $buff
:loop
mov ax 0
mov bx $buff
mov cx 100
mov dx $com_desc
sys

;printsint ax

mov cx ax
; jos luettu nolla tavua, palaa alkuun
jcz loop

printsint ax
printstr bx

; kirjoitetaan puskurin sis„lt” tiedostoon
mov ax 1
mov dx $file_desc
sys
mov ax 100

jmp loop

