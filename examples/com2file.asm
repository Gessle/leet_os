code auto
data auto
interrupts 1
stack 1

; 8 databitti„, 1 stopbitti„, pariton pariteetti
word sarjaportti_tila 0b00001011
; 4800 baud
byte baudit 7
; xon/xoff vuonohjaus
byte vuonohjaus 0b01

mov al 6
mov ah $vuonohjaus
shl ah al
mov al $baudit
or ah al
mov al $sarjaportti_tila
mov $sarjaportti_tila ax

; tiedoston vastaanotto
str puskuri 100
str annatiedosto "Sy”t„ tallennettavan tiedoston nimi."

mov ax $annatiedosto
printstr ax
mov ax $puskuri
mov bx 100
getstr ax bx

; avataan tiedosto kirjoitustilassa
mov ax 2
mov bx 1
mov dx $puskuri
mov ex 0
sys

word fp 0
mov $fp ax

mov bx -1
cmp ax bx
je virhe1

str tiedostoavattu "Tiedosto avattu onnistuneesti."
mov ax $tiedostoavattu
printstr ax
str avsarjaportti "Avataan sarjaportti vastaanottoon."
mov ax $avsarjaportti
printstr ax

; avataan sarjaportti
str sarjaportti "COM1"
mov ax 2
mov bx $sarjaportti_tila
mov dx $sarjaportti
sys
word com_fp 0
mov $com_fp ax

mov bx -1
cmp ax bx
je virhe2

str luetaan "Luetaan sarjaporttia."
mov ax $luetaan
printstr ax

:silmukka
; luetaan sarjaportista 100 tavua puskuriin
mov ax 0
mov bx $puskuri
mov cx 100
mov dx $com_fp
sys

mov bx -1
cmp ax bx
je virhe3

; otetaan luettujen tavujen m„„r„ talteen CX-rekisteriin
mov cx ax

; luettujen tavujen yhteism„„r„st„ pidet„„n kirjaa
; EX-rekisteriss„
add ex cx
;str tavujaluettu "Tavuja luettu yhteens„:"
;mov ax $tavujaluettu
;printstr ax

cmp ex fx
je nollaluettu
printuint ex

:nollaluettu

; jos luettujen tavujen m„„r„ on 0, hyp„t„„n alkuun
jcz silmukka

str kirjoitetaan "Kirjoitetaan tiedostoon."
mov ax $kirjoitetaan
printstr ax

; kirjoitetaan luetut tavut tiedostoon
mov ax 1
mov bx $puskuri
mov dx $fp
sys

; hyp„t„„n takaisin silmukan alkuun
jmp silmukka

wait
:virhe1
str virhestr1 "Tapahtui virhe tiedostoa avatessa."
mov ax $virhestr1
printstr ax
jmp loppu
:virhe2
str virhestr2 "Tapahtui virhe sarjaporttia avatessa."
mov ax $virhestr2
printstr ax
jmp loppu
:virhe3
str virhestr3 "Tapahtui virhe sarjaporttia lukiessa."
mov ax $virhestr3
printstr ax
jmp loppu
:loppu
; suljetaan tiedostodeskriptorit
mov ax 3
mov bx $fp
sys
mov ax 3
mov bx $com_fp
sys
str valmis "Ohjelma on p„„ttynyt."
mov ax $valmis
printstr ax
wait