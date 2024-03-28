str title "Ikkunan otsikko"
str teksti "Alapuolella on nelikulmio"


; luodaan ikkuna
mov ax 0
mov bx 600
mov cx 400
mov dx $title
scr

mov gx 0
mov hx 0

mov bx 40

; piirretaan ikkunaan nelioe
mov ex ax
mov ax 2

mov cx 200
:nelioe
mov fx bx
add fx cx
mov cx 40

:drawpix
scr

inc bx
cmp bx fx
je endrow

jmp drawpix

:endrow
inc cx
cmp cx fx
je end
mov bx 20
jmp drawpix

:end

;kirjoitetaan ikkunaan teksti
mov ax 7
mov bx 20
mov cx 0
mov dx $teksti
scr

mov ax 5
scr

cmp gx hx
je incbx

:decbx
mov gx 1
dec bx
cmp bx hx
jne nelioe


:incbx
mov gx 0
inc bx
cmp bx cx
je decbx

jmp nelioe
