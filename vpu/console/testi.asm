;kirjoitetaanpas konmentti
;koodia:
str hello "hello world"
word numero 0xFFFF
mov cx $numero
mov ax $hello
printstr ax
printuint cx