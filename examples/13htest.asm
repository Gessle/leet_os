; This is a code example for virtual machine console

str title "Uusi ikkuna"
str teksti "Alapuolella on liikkuva nelikulmio:"
word nelio_x 20
byte inkrementoi 1

setint 1 .nappis_handler $nappis_handler

jmp main

:nappis_handler
push ax
in 1
printuint ax
pop ax
iret

:piirrateksti
mov ex dx
mov dx $teksti
mov bx 20
mov cx 0
mov ax 4
scr
mov dx ex
ret

:piirranelio
mov ax 5
mov bx $nelio_x
mov cx 10
mov fx bx
mov gx 20
add fx gx
:jatkarivia
scr

inc bx
cmp bx fx
je piirranelio_seuraavarivi
jmp jatkarivia

:piirranelio_seuraavarivi
inc cx
cmp cx gx
je piirranelio_loppu

mov bx $nelio_x
jmp jatkarivia

:piirranelio_loppu
ret


:inkrementoi_x
mov ax $nelio_x
inc ax
mov $nelio_x ax

mov bx 100
cmp ax bx
jne loppu_inkrementoi

mov bl 0
mov $inkrementoi bl

:loppu_inkrementoi
ret

:dekrementoi_x
mov ax $nelio_x
dec ax
mov $nelio_x ax

mov bx 0
cmp ax bx
jne loppu_inkrementoi

mov bl 1
mov $inkrementoi bl

ret

:main

; asetetaan nappaimistohandleri
;setint 1 .nappis_handler $nappis_handler

; luodaan ikkuna
mov ax 1
mov bx 0x13
;mov cx 200
mov dx $title
;mov ex 1
;mov fx 1
scr

; kopioidaan ikkunan ID rekisteriin dx
mov dx ax
; kutsutaan nelioen piirtofunktio
call piirrateksti
:loop
inc dx
call piirranelio

; paivitetaan ikkuna
mov ax 2
scr

; nollataan ikkuna
mov ax 3
scr

; siirretaan nelion vasenta reunaa
mov bh 0
mov bl $inkrementoi
cmp bh bl
je dekrementoi

call inkrementoi_x
jmp loop

:dekrementoi
call dekrementoi_x
jmp loop
