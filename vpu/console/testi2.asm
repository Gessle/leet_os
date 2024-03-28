;kirjoitetaanpas konmentti
;koodia:
str hello "hello world"
word numero 0xFF

jmp alku

:hellofunc
mov ax $hello
printstr ax
ret

:numfunc
printuint cx
dec cx
jcz loppu
ret

:alku
mov cx $numero

:loop
call hellofunc
call numfunc
jmp loop

:loppu