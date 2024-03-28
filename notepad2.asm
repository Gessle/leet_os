;stack 32
;code auto
;data auto
;interrupts 17

; notepad program

str title "Notepad"
;str filename 80
str filename "testi.txt"

jmp start
; get first argument (filename)
mov ax 13
sys
mov bx 1
cmp ax bx
jle exit

; copy argument 1 to $filename
mov bx 1
mov cx -1
mov dx $filename
:arg_loop1
inc cx
mov ax 14
sys
mov *dx al
inc dx
cmp ah al
jne arg_loop1

:start
; set sigusr1 handler
setint 16 .key_handler $key_handler

; create window
mov ax 0
mov bx 400
mov cx 200
mov dx $title
mov ex 16
mov fx 0
mov gx 0
mov hx 0
scr

push ax

; open file
mov ax 2
mov bx 0
mov cx 0
mov dx $filename
mov ex 0
sys

mov gx 0xFFFF
cmp ax gx
je exit

; allocate as much memory as possible and mmap the file
mov cx 0xE000
mov ex ax
:mmap_loop
dec cx
mov ax 9
mov bx 0
mov fx 0
sys
cmp ax gx
je mmap_loop

dseg ax

; close file
mov ax 3
mov bx ex
sys

; create textbox
mov fx cx
dec fx
mov ax 29
mov bx 1
mov cx 20
mov dx 0
pop ex
mov gx 380
mov hx 160
scr
printuint ax

; create scrollbars
mov hx bx
mov ax 11
mov bx 381
mov cx 20
mov fx 132
mov gx 20
scr

mov ax 11
mov bx 1
mov cx 170
mov fx 348
mov gx 20
mov dx 1
scr

; create save button
str savelabel "Save file"
mov ax 8
mov bx 1
mov cx 1
mov dx $savelabel
mov fx 0
mov gx 100
mov hx 18
push ds
mov ds 0
scr
word save_keycode
mov $save_keycode ax
pop ds
printuint ax

:do_nothing
wait
jmp do_nothing

:exit
mov ax 12
mov bx 0
sys

:save_file
; open file for writing
mov ax 2
mov bx 1
mov cx 0
mov dx $filename
mov ex 0
push ds
mov ds 0
sys
pop ds

; save contents
mov dx ax
mov cx 0
mov bx 0
mov ah 0
:save_loop
mov al *cx
cmp ah al
je save_write
inc cx
jmp save_loop
:save_write
mov ax 1
printuint cx
sys
cmp ax cx
jne error

:save_close
; close file
mov ax 3
mov bx dx
sys

str savestr "File saved."
mov ax $savestr
push ds
mov ds 0
printstr ax
pop ds

ret

:key_handler
in 0
printuint ax
push ds
mov ds 0
mov bx $save_keycode
pop ds
cmp ax bx
jne key_loop
call save_file
:key_loop
in 1
mov bx -1
cmp ax bx
jne key_loop
:key_handler_end
iret

:error
str errorstr "Error"
mov ax $errorstr
push ds
mov ds 0
printstr ax
pop ds
