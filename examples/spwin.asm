stack 4096
code 1 1024
data 1 512
interrupts 5

str muuttunut "Tekstiboksin sis„lt” muuttunut, uusi sis„lt”
:"
str hello 21
str uusiteksti "spr”l”l””"
str title "Uusi ikkuna"
str teksti "Alapuolella on nuolilla liikuteltava nelikulmio
:"
str vasemmalle "Siirret„„n neli”t„ vasemmalle..."
str oikealle "Siirret„„n neli”t„ oikealle..."
str nappiteksti "Painonappi"
str painettu "Painonappia painettu"
word painonappi_keycode 0
word nelio_x 20
word nappikoodi 0
word tekstilaatikko 0

setint 1 .nappis_handler $nappis_handler

jmp main

:nappis_handler
push ax
push bx
in 1
mov bx -1
cmp ax bx
je naphandl_end
printuint ax
mov $nappikoodi ax
mov bx 0
cmp ax bx
jne naphandl_end
in 1
mov bx -1
cmp ax bx
je naphandl_end
mov bx 0
cmp ax bx
je naphandl_end
printuint ax
mov $nappikoodi ax
:naphandl_end
pop bx
pop ax
iret

:piirrateksti
;mov ex dx
mov dx $teksti
mov bx 20
mov cx 0
mov ax 4
scr
;mov dx ex
ret

:piirraympyra
push ax
push bx
push cx
push fx
push dx

mov ax 13
mov bx 200
mov cx 150
mov fx 35
mov dx 0x00
scr

pop dx
pop fx
pop cx
pop bx
pop ax
ret

; dx = ikkunadeskriptori
; bx = x
; cx = y
:piirranelio
push ax
push fx
push gx
push dx
push bx
push cx

mov ax 15
mov bx $nelio_x
mov cx 50
mov fx 100
mov gx 50
mov dx 0
scr

pop cx
pop bx
pop dx
pop gx
pop fx
pop ax
ret


:inkrementoi_x
push ax

mov ax $nelio_x
inc ax
mov $nelio_x ax

pop ax
ret

:dekrementoi_x
push ax

mov ax $nelio_x
dec ax
mov $nelio_x ax

pop ax
ret

:piirraviiva
push ax
push bx
push cx
push dx

mov ax 14
mov bx 200
mov cx 150
mov dx 2
mov fx 300
mov gx 185
scr

pop dx
pop cx
pop bx
pop ax
ret

:main

; asetetaan nappaimistohandleri
;setint 1 .nappis_handler $nappis_handler

; luodaan ikkuna
mov ax 1
mov bx 0x13
;mov cx 200
mov dx $title
mov ex 1
mov fx 1
scr

; kopioidaan ikkunan ID rekisteriin ex
mov ex ax

; piirretaan teksti
call piirrateksti


; luodaan buttoni
mov ax 8
mov bx 10
mov cx 50
mov gx 80
mov hx 50
mov dx $nappiteksti
scr
mov $painonappi_keycode ax
;mov dx ex

; tehdaan tekstilaatikko
mov ax 9
mov bx 10
mov cx 110
mov dx $hello
mov gx 100
mov fx 20
scr
mov $tekstilaatikko ax

mov ax 19
mov dx $uusiteksti
scr

:loop
call piirranelio

; kutsutaan ympyranpiirto
call piirraympyra

; kutsutaan viivanpiirto
call piirraviiva

; paivitetaan ikkuna
mov ax 2
scr

; nollataan ikkuna
mov ax 3
scr

; siirretaan nelion vasenta reunaa
mov bx $nappikoodi
mov cx 75
cmp bx cx
je dekrementoi
mov cx 77
cmp bx cx
je inkrementoi
;; tarkistetaan onko painonappia painettu
mov cx $painonappi_keycode
cmp bx cx
je nappia_painettu
; tarkistetaan onko tekstilaatikon sisalto muuttunut
mov cx $tekstilaatikko
cmp bx cx
je sisalto_muuttunut

jmp loop

:nappia_painettu
mov $nappikoodi 0
mov ax $painettu
printstr ax
jmp loop

:sisalto_muuttunut
mov $nappikoodi 0
mov ax $muuttunut
printstr ax
mov ax $hello
printstr ax
jmp loop

:inkrementoi
mov $nappikoodi 0
mov ax $oikealle
printstr ax
call inkrementoi_x
jmp loop

:dekrementoi
mov $nappikoodi 0
mov ax $vasemmalle
printstr ax
call dekrementoi_x
jmp loop
