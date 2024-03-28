str title "Ikkunan otsikko"


; luodaan ikkuna
mov ax 0
mov bx 400
mov cx 400
mov dx $title
scr


; piirretaan ikkunaan pikselit
mov ex ax
mov ax 2

mov fx 200
mov bx 20
mov cx 20

:drawpix
scr

inc bx
cmp bx fx
je end

jmp drawpix

:end


mov ax 5
scr