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

:main
str numero 10
mov ax 1485
mov bx $numero
call printf_u