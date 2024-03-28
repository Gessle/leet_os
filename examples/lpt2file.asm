; tiedoston sis„ll”n siirto sarjaporttiin

; kutsutaan sarjaportin asetukset tekev„ ohjelma
;str modeprog "com1mode.app"
;str args 1
;str pid 2

;mov ax 15
;mov bx $pid
;mov cx $modeprog
;mov dx $args
;sys

; avataan sarjaportti
str sarjaportti "LPT1"
word com_desc 0
mov ax 2
mov dx $sarjaportti
; baud 4800
mov bh 6
; pariteetti 0, data 8, stop 2
mov bl 0b00000111
sys
mov $com_desc ax

; avataan tiedosto kirjoitustilassa
str tiedosto "testi.txt"
word file_desc 0
mov ax 2
mov dx $tiedosto
mov bx 1
sys
mov $file_desc ax

str buff 100

byte waiting 1

; luetaan puskuriin 100 tavua sarjaportista
mov bx $buff
:wait
str odotetaan "Odotetaan toista p„„t„..."
mov ax $odotetaan
printstr ax
mov ax $waiting
mov cx ax
jcz end

:loop

mov ax 0
mov cx 100
mov dx $com_desc
sys

printsint ax
printstr bx

mov cx ax
; jos luettu nolla tavua, palaa alkuun
jcz wait
mov ax 0
mov $waiting ax

; kirjoitetaan puskurin sis„lt” tiedostoon
mov ax 1
mov dx $file_desc
sys
mov ax 100
; jos luettiin 100 tavua, luetaan lis„„
cmp ax cx
je loop

:end
wait