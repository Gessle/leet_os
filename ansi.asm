;str up "\x0AHello world\x1B[1A"
;str up "\x0AHello World\x1B[0;10Hfdsfds"
str up "\x0AHello World\x1B[s\x1B[0Jhfdsffgfd\x1B[u"

mov ax 1
mov bx $up
mov cx 1
mov dx 1

:loop
mov eh *bx
cmp eh el
je end
sys
inc bx
jmp loop
:end
mov ax 27
mov bx 1
sys
jmp end
