;stack 32
;data auto
;code auto
;interrupts 1

word starttime_high
word starttime_low
word uptime_high
word uptime_low

mov ax 43
mov bx 1
sys
mov $starttime_high ax

mov ax 43
mov bx 2
sys
mov $starttime_low ax

:loop
; get time
mov ax 18
sys

mov bx $starttime_low
sub dx bx
mov $uptime_low dx

;mov ax $starttime_high
;jfns 0x01 nocarry
;inc cx
;:nocarry
;sub cx ax
;mov $uptime_high cx

;printuint dx

str time "0  hours, 0  minutes, 0  seconds\n" 

mov ex 60
div dx ex
mov fx ex
mov ex 60
div dx ex

mov bx $time
push bx
mov ax dx
call printf_u
pop bx
mov ax 10
add bx ax
push bx
mov ax ex
call printf_u
pop bx
mov ax 12
add bx ax
mov ax fx
call printf_u

;mov ax $time
;printstr ax
mov ax 1
mov bx $time
mov dx 1
mov cx 33
sys

mov ax 12
mov bx 0
sys

:printf_u_appnum
  push dx
  mov dx 0x30
  add cx dx
  mov *bx cl
  inc bx
  pop dx
  ret

; AX == number to process
; BX == pointer to output string

:printf_u
  push ax
  push cx
  push dx

  mov cx ax
  mov dx 10000
  div cx dx

  jcz printf_u_1000
  call printf_u_appnum
  mov ax dx

  :printf_u_1000
  mov cx ax
  mov dx 1000
  div cx dx

  jcz printf_u_100
  call printf_u_appnum
  mov ax dx

  :printf_u_100
  mov cx ax
  mov dx 100
  div cx dx

  jcz printf_u_10
  call printf_u_appnum
  mov ax dx

  :printf_u_10  
  mov cx ax
  mov dx 10
  div cx dx

  jcz printf_u_1
  call printf_u_appnum
  mov ax dx

  :printf_u_1
  mov cx ax
  call printf_u_appnum


  pop dx
  pop cx
  pop ax
  ret

