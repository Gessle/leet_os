str title "otsikko"
str text "hello friends"
str help "apustus laatikko"
:alku
mov ax 20
mov bx $title
mov cx $text
mov dx $help
mov eh 1
mov el 3
scr
cmp ax fx
jne alku