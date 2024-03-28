code auto
data auto
interrupts 1
stack 1

; tiedoston tulostus
str puskuri 100
str annatiedosto "Sy”t„ tulostettavan tiedoston nimi."

mov ax $annatiedosto
printstr ax
mov ax $puskuri
mov bx 100
getstr ax bx

; avataan tiedosto lukutilassa
mov ax 2
mov bx 0
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
str tulostetaan "Avataan rinnakkaisportti tulostaaksemme."
mov ax $tulostetaan
printstr ax

; avataan rinnakkaisportti
str rinnakkaisportti "LPT1"
mov ax 2
mov bx 0x1000
mov dx $rinnakkaisportti
sys
word lpt_fp 0
mov $lpt_fp ax

mov bx -1
cmp ax bx
je virhe2

:silmukka
str luetaan "Luetaan tiedostoa."
mov ax $luetaan
printstr ax

; luetaan tiedostosta 100 tavua puskuriin
mov ax 0
mov bx $puskuri
mov cx 100
mov dx $fp
sys

mov bx -1
cmp ax bx
je virhe3

; otetaan luettujen tavujen m„„r„ talteen CX-rekisteriin
mov cx ax

; jos luettujen tavujen m„„r„ on 0, lopetetaan
jcz loppu

str kirjoitetaan "Kirjoitetaan rinnakkaisporttiin."
mov ax $kirjoitetaan
printstr ax

; kirjoitetaan luetut tavut rinnakkaisporttiin
mov ax 1
mov bx $puskuri
mov dx $lpt_fp
sys

mov bx -1
cmp ax bx
je virhe4

mov cx ax
jcz eipaperia

; hyp„t„„n takaisin silmukan alkuun
jmp silmukka

wait
:virhe1
str virhestr1 "Tapahtui virhe tiedostoa avatessa."
mov ax $virhestr1
printstr ax
jmp loppu
:virhe2
str virhestr2 "Tapahtui virhe rinnakkaisporttia avatessa."
mov ax $virhestr2
printstr ax
jmp loppu
:virhe3
str virhestr3 "Tapahtui virhe tiedostoa lukiessa."
mov ax $virhestr3
printstr ax
jmp loppu
:virhe4
str virhestr4 "Tapahtui virhe tulostamista yritt„ess„."
mov ax $virhestr4
printstr ax
jmp loppu
:eipaperia
str paperiloppu "Tulostimessa ei ole paperia."
mov ax $paperiloppu
printstr ax
:loppu
; suljetaan tiedostodeskriptorit
mov ax 3
mov bx $fp
sys
mov ax 3
mov bx $lpt_fp
sys
str valmis "Ohjelma on p„„ttynyt."
mov ax $valmis
printstr ax
wait