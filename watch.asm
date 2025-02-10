stack 16
interrupts 20
data auto
code auto

str progname 13
str progargs 256
word interval

setint 18 .child_died $child_died

; get argument count
mov ax 13
sys
cmp ax 3
jl help

; get interval
mov bx 1
xor cx cx
mov dx $progname
:getintervalloop
mov ax 14
sys
mov *dx al
cmp al 0
je getprogname
inc dx
inc cx
jmp getintervalloop

:getprogname

mov ax $progname
call atoi
mov $interval ax

; get program name
mov bx 2
xor cx cx
mov dx $progname
:getprognameloop
mov ax 14
sys
mov *dx al
cmp al 0
je getargs
inc dx
inc cx
jmp getprognameloop

:getargs
mov ax 13
sys
cmp ax 4
jl noargs

mov ex ax

mov bx 3
mov dx $progargs
:nextarg
xor cx cx
:getargsloop
mov ax 14
sys
mov *dx al
cmp al 0
je getnextarg
inc dx
inc cx
jmp getargsloop
:getnextarg
inc bx
cmp bx ex
je args_ready
mov al ' '
mov *dx al
inc dx
jmp nextarg
:args_ready

:noargs
; make a fork
word clone_retval
mov ax 17
mov dx 18
mov ex &clone_retval
sys

; if this process is the parent, wait for signal
cmp ax 0
jne wait_chld

; else run the program with EXEC syscall
mov ax 30
mov bx $progname
mov cx $progargs
sys

; if still alive, print error and exit
str error "Error"
mov ax $error
printstr ax
jmp exit

:wait_chld
; pause
mov ax 18
sys
mov ex dx
mov dx cx
mov cx bx
mov bx ax
mov ax $interval
add ex ax
jfns 0x80 no_carry
inc dx
:no_carry
wait
mov ax 22
sys

jmp noargs

:help
str help1 "Usage: $ watch [interval] [program]"
mov ax $help1
printstr ax

:exit
mov ax 12
xor bx bx
sys

:child_died
iret

; AX == pointer to str
; Returns length of the string to AX
:strlen
push cx
push bx
mov bx ax
xor ax ax
xor cx cx
:strlen_loop
mov cl *bx
jcz strlen_end
inc ax
inc bx
jmp strlen_loop
:strlen_end
pop bx
pop cx
ret

; AX == pointer to string
; Returns int (unsigned) value to AX
:atoi
push bx
push cx
push dx
push ex
push fx
push gx
push hx
mov hx 1

mov dh 0x2D
mov dl *ax
cmp dh dl
jne atoi_start

inc ax
mov hx -1

:atoi_start
mov bx ax
call strlen
mov cx ax
mov ax 0
mov dh 0x30

:atoi_loop
dec cx
mov dl *bx
inc bx
sub dl dh
jcz atoi_end

mov ex cx
mov gx 10
mov fx 1
:atoi_mul
mul fx gx
dec cx
jcz atoi_stopmul
jmp atoi_mul
:atoi_stopmul
mul fx dl
add ax fx
mov cx ex

jmp atoi_loop
:atoi_end
add ax dl

mul ax hx

pop hx
pop gx
pop fx
pop ex
pop dx
pop cx
pop bx
ret