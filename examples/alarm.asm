stack 128
code auto
data auto
interrupts 2

jmp main

; AX == number to process
; BX == pointer to output string

:printf_u_appnum
  push dx
  mov dx 0x30
  add cx dx
  mov *bx cl
  inc bx
  pop dx
  ret

:printf_u
  push ax
  push cx
  push dx

  mov cx ax
  mov dx 10000
  div cx dx

  call printf_u_appnum
  mov ax dx

  mov cx ax
  mov dx 1000
  div cx dx

  call printf_u_appnum
  mov ax dx

  mov cx ax
  mov dx 100
  div cx dx

  call printf_u_appnum
  mov ax dx

  mov cx ax
  mov dx 10
  div cx dx

  call printf_u_appnum
  mov ax dx

  mov cx ax
  call printf_u_appnum


  pop dx
  pop cx
  pop ax
  ret


; AX == pointer to str
; Returns length of the string to AX
:strlen
push cx
push bx
mov bx ax
mov ax 0
:strlen_loop
mov cx *bx
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

signed word seconds_left -1

:start_counting
  push ax

  mov ax $text_buff
  call atoi
  mov $seconds_left ax

  call init_wait_seconds

  pop ax
  ret

:keycode_handler
  push ax
  push bx

  in 0
  mov bx $button_keycode
  cmp ax bx
  jne keycode_handler_end

  call start_counting

  :keycode_handler_end
  pop bx
  pop ax
  iret

; returns seconds since last call
:wait_second
push bx
push cx
push dx
push ex
push fx

word seconds 0

:wait_second_loop
mov ax 18
sys
mov cx $seconds
cmp cx dx
je wait_second_loop

mov $seconds dx

sub dx cx
mov ax dx

pop fx
pop ex
pop dx
pop cx
pop bx
ret

:init_wait_seconds
push ax
push bx
push cx
push dx
push ex
push fx

mov ax 18
sys
mov $seconds dx

pop fx
pop ex
pop dx
pop cx
pop bx
pop ax
ret

:main

; create new window
str window_title "Alarm clock"
mov ax 0
mov bx 200
mov cx 70
mov dx $window_title
mov ex 1
mov fx 0
scr
word window_desc 0
mov $window_desc ax

setint 1 .keycode_handler $keycode_handler 

; create textbox
str text_buff 10
mov ax 9
mov bx 10
mov cx 10
mov dx $text_buff
mov ex $window_desc
mov fx 9
mov gx 180
scr
word textbox_desc 0
mov $textbox_desc bx

; create button
str button_text "Start"
mov ax 8
mov bx 10
mov cx 40
mov dx $button_text

mov fx 0
mov gx 180
mov hx 20
scr
word button_keycode 0
mov $button_keycode ax

; update window
mov ax 3
scr

:mainloop
mov ax -1
mov bx $seconds_left
cmp ax bx
jne countdown
jmp mainloop

:countdown
call wait_second
mov cx $seconds_left
cmp cx ax
jle alarm

sub cx ax
mov $seconds_left cx

mov bx $text_buff
mov ax cx
call printf_u

mov cl 0
mov *bx cl

mov dx $text_buff
mov bx $textbox_desc
mov ax 19
scr
mov ax 3
scr

jmp countdown

:alarm
mov ax -1
mov $seconds_left ax
mov cx 5
mov ax 19
:alarmloop
mov bx 600
sys
call wait_second
mov ax 19
mov bx 500
sys
call wait_second
mov ax 19
dec cx
jcz end_alarm
jmp alarmloop
:end_alarm
mov ax 20
sys
jmp mainloop
