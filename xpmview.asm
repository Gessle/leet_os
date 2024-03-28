data auto
code auto
stack 1
interrupts 1

;xpm 2 viewer

; create new window
mov ax 0
mov bx 400
mov cx 200
str title "XPM2 viewer"
mov dx $title
scr
word window_desc
mov $window_desc ax

; get argument 1
str filename 100

mov ax 14
mov bx 1
mov cx 0
mov dx $filename
mov ex 0
:argloop
sys
mov *dx al
cmp ax ex
je argloop_end
inc cx
inc dx
mov ax 14
jmp argloop
:argloop_end

; load image
mov ax 21
mov bx 0
mov cx 0
mov dx $filename
mov ex $window_desc
mov fx 0
mov gx 0
scr

; update window
mov ax 3
scr

wait