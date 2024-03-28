;escape test
str hello "Hello \"friends\""
str hello2 "Hello \' \0 world"
str hello3 "hello \x31"

byte array { 'b' 'a' 's' 'g' 'a' 0 }

mov ax $hello
mov bx $hello2
mov cx $hello3
mov dx &array

printstr ax
printstr bx
printstr cx
printstr dx