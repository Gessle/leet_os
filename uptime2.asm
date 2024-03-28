stack 32
data auto
code auto
interrupts 1

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

; get time
mov ax 18
sys

mov ax dx
mov dx cx
mov bx $starttime_high
mov cx $starttime_low
call b32_sub
mov $uptime_low ax
mov $uptime_high dx

str time "0    days, 0  hours, 0  minutes, 0  seconds\n" 

; divide uptime in seconds by 86400 (secs per day)
mov bx 0x0001
mov cx 0x5180
call b32_div
; now ax = days
; and bx:cx = uptime % secs per day
mov gx ax

mov ax cx
mov dx bx
xor bx bx
mov cx 60
; divide by 60
call b32_div
; now ax = minutes and (bx:)cx = seconds
mov fx cx

xor dx dx
mov cx 60
; divide minutes by 60
call b32_div
; now ax = hours, cx = minutes
mov dx ax
mov ex cx


;mov dx cx
;mov fx 60
;mov ex fx
;div dx fx
;div dx ex


mov bx $time
push bx
mov ax gx
call printf_u
pop bx
add bx 11
push bx
mov ax dx
call printf_u
pop bx
add bx 10
push bx
mov ax ex
call printf_u
pop bx
add bx 12
mov ax fx
call printf_u

mov ax 1
mov bx $time
mov dx 1
mov cx 44
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

; dx:ax = dx:ax - bx:cx
:b32_sub
  push bx
  sub ax cx
  jfns 0x80 b32_sub_nc
  inc bx
  :b32_sub_nc
  sub dx bx
  pop bx
  ret

; ax = dx:ax / bx:cx
; bx:cx = mod
:b32_div
; todo: replace this with binary division...
  push ex
  xor ex ex

  :b32_div_loop
  push ax
  push dx

  call b32_sub
  jfs 0x20 b32_div_end
  inc ex
  add sp 2
  jmp b32_div_loop

  :b32_div_end
  pop bx
  pop cx
  mov ax ex

  pop ex
  ret



