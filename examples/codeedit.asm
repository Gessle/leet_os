jmp main

:dseg
dseg 0
jmp end

:main
mov ax $dseg
inc ax
mov *ax 5
jmp dseg

:end