stack 32
code auto
data auto
interrupts 134

; notepad program

str titlefilename 80
str title " - Notepad"
word titleptr
str filename 80
;str filename "testi.txt"

word window_width 400
word window_height 200

setint 133 .winresize $winresize

; hide console
mov ax 23
mov bx 0
mov cx 128
sys

;jmp start
; get first argument (filename)
mov ax 13
sys
mov bx 1
cmp ax bx
jle noargs

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

; copy filename to title
mov ax $title
sub ax cx
mov $titleptr ax
mov bx $filename
mov ch 0
:title_loop
mov cl *bx
cmp ch cl
je titleloop_end
mov *ax cl
inc ax
inc bx
jmp title_loop
:titleloop_end


:start
; set sigusr1 handler
setint 16 .key_handler $key_handler

word window_desc

; create window
mov ax 0
mov bx $window_width
mov cx $window_height
mov dx $titleptr
mov ex 16
mov fx 0
mov gx 0
mov hx 0
scr

mov $window_desc ax

; open file
mov ax 2
mov bx 0
mov cx 0
mov dx $filename
mov ex 0
sys

mov gx 0xFFFF
cmp ax -1
je empty_file
word file_desc
mov $file_desc ax
:empty_file

; allocate as much memory as possible
mov cx 0xE000
mov ex 0
:mmap_loop
dec cx
mov ax 9
mov bx 0
mov fx 0
sys
cmp ax -1
je mmap_loop

mov fx cx
push ax

; read file
mov dx $file_desc
pop ds
xor ax ax
xor bx bx
sys

cmp cx ax
je toobigfile

; convert cp/m-style newlines to posix-style
mov cx ax
:conv_loop
mov ax ds
mov dl 0x0D
memchr
jcz conv_loop_end

push cx
push bx
push ax

; ax:bx points to the 0x0D character
mov ex bx
mov dx ax
inc bx
memcpy

pop ax
pop bx
pop cx
jmp conv_loop
:conv_loop_end

;mov dx $file_desc
;pop ds
;mov bx 0
;cmp dx bx
;je new_file2
;mov cx 1
;mov eh 0x0D
;:read_loop
;mov ax 0
;sys
;; convert cp/m-style newlines to posix-style
;mov el *bx
;cmp eh el
;je read_loop
;inc bx
;cmp bx fx
;je toobigfile
;cmp ax cx
;je read_loop

; close file
mov ax 3
mov bx dx
sys

:new_file2

; create textbox
word textbox_desc
push ds
mov ds 0
dec fx
mov ax 29
mov bx 1
mov cx 20
mov dx 0
mov ex $window_desc
mov gx 380
mov hx 160
pop ds
scr
push ds
mov ds 0
;printuint ax
mov $textbox_desc bx

; create scrollbars
mov hx bx
mov ax 11
mov bx 382
mov cx 20
mov fx 134
mov gx 20
scr
word scroll1_desc
mov $scroll1_desc 1

mov ax 11
mov bx 1
mov cx 182
mov fx 350
mov gx 0xFFFF
mov dx 1
scr
word scroll2_desc
mov $scroll2_desc 2

; create buttons
str savelabel "\x11Save file"
mov ax 8
mov bx 1
mov cx 1
mov dx $savelabel
mov fx 0
mov gx 100
mov hx 18
scr
word save_keycode
mov $save_keycode ax
;printuint ax

str monospace "\x11Monospace font"
mov ax 8
mov bx 101
;mov cx 1
mov dx $monospace
;mov fx 0
mov gx 100
mov hx 18
scr
word mono_keycode
mov $mono_keycode ax

str sans "\x11Normal font"
mov ax 8
mov bx 202
;mov cx 1
mov dx $sans
;mov fx 0
mov gx 100
mov hx 18
scr
word sans_keycode
mov $sans_keycode ax
pop ds

; draw window
mov ax 3
scr

:do_nothing
wait
jmp do_nothing

:exit
mov ax 12
mov bx 0
sys

:save_file
push ax
push bx
push cx
push dx
push ex
push ds

; open file for writing
mov ax 2
mov bx 1
mov cx 0
mov ds 0
mov dx $filename
mov ex 0
sys

pop ds

; save contents
mov dx ax
mov bx 0
mov eh 0x0A
:save_loop
mov cx 1
mov ax cx
mov el *bx
cmp eh el
jne save_nonewline
call save_addcr
jmp save_newline
:save_nonewline
cmp el ah
je save_close
sys
cmp ax cx
jne error
:save_newline
inc bx
jmp save_loop

:save_close
; close file
mov ax 3
mov bx dx
sys

str savestr "File saved."
mov bx $savestr
mov cx bx
mov dx 0
mov ds 0
mov ax 20
scr

pop ex
pop dx
pop cx
pop bx
pop ax
ret

str cr "\r\n"
:save_addcr
push bx
push ds
mov ax 1
mov ds 0
mov bx $cr
mov cx 2
sys
cmp ax cx
jne error
pop ds
pop bx
ret

:set_monospace
mov ax 32
mov bx $textbox_desc
mov cx 1
scr
ret

:set_sans
mov ax 32
mov bx $textbox_desc
mov cx 0
scr
ret

:key_handler
push ax
push bx
push ds

in 0
printuint ax
mov ds 0
mov bx 0
cmp ax bx
je key_loop

mov bx 's'
cmp ax bx
je call_save
mov bx 'S'
cmp ax bx
je call_save

mov bx 'm'
cmp ax bx
je call_mono
mov bx 'M'
cmp ax bx
je call_mono

mov bx 'n'
cmp ax bx
je call_sans
mov bx 'N'
cmp ax bx
je call_sans

mov bx $save_keycode
cmp ax bx
jne key_handler1
:call_save
pop ds
push ds
call save_file
jmp key_handler_end
:key_handler1
mov bx $mono_keycode
cmp ax bx
jne key_handler2
:call_mono
call set_monospace
jmp key_handler_end
:key_handler2
mov bx $sans_keycode
cmp ax bx
jne key_handler_end
:call_sans
call set_sans

jmp key_handler_end

:key_loop
in 0
mov bx -1
cmp ax bx
jne key_loop

:key_handler_end
pop ds
pop bx
pop ax
iret

:error
str errorstr "Error"
mov bx $errorstr
mov cx $errorstr
mov dx 0
mov ds 0
mov ax 20
scr
jmp exit

:noargs
str usage "Usage: $ NOTEPAD [file]"
mov ax $usage
printstr ax
jmp exit

:toobigfile
str toobig "Too big file"
mov ax 20
mov bx $error
mov cx $toobig
mov dx 0
mov ex 0
scr
jmp exit

:winresize
  push ax
  push bx
  push cx
  push dx
  push ex
  push ds
  mov ds 0

  mov cx -1

  mov ax 33
  scr
  cmp ax cx
  je exit

  mov $window_width ax
  mov $window_height bx

  mov ax 24
  mov ex $window_desc
  mov bx $scroll1_desc
  mov cx $window_width
  mov dx 18
  sub cx dx
  mov dx 20
  scr

  mov ax 23
  mov bx $scroll1_desc
  mov cx $window_height
  mov dx 66
  sub cx dx
  scr

  mov ax 24
  mov bx $scroll2_desc
  mov dx $window_height
  mov cx 18
  sub dx cx
  mov cx 1
  scr

  mov ax 23
  mov bx $scroll2_desc
  mov cx $window_width
  mov dx 46
  sub cx dx
  scr

  mov ax 23
  mov cx $window_width
  mov dx 20
  sub cx dx
  mov dx $window_height
  mov bx 40
  sub dx bx
  mov bx $textbox_desc
  scr

  mov ax 3
  scr

  in 0

  pop ds
  pop ex
  pop dx
  pop cx
  pop bx
  pop ax
iret