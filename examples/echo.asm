stack 16
code auto
data auto
interrupts 1

jmp main

; write a string to stdout (file descriptor 1)
; arguments: AX == pointer to str
; returns: non-negative if success, -1 if error
;
; this procedure checks the number of written bytes
:puts
  push bx
  push cx
  push dx

  mov bx ax

  :puts_strlen_start
  push bx

  ; get length of the string to cx
  mov ax 0
  mov cx 0
  :puts_strlen_loop
  mov ah *bx
  cmp ah al
  je puts_strlen_end
  inc cx
  inc bx
  jmp puts_strlen_loop
  :puts_strlen_end  
  pop bx

  jcz puts_end
  ; write
  mov ax 1
  mov dx 1
  sys

  ; compare str length and count of written bytes
  cmp ax cx
  je puts_end
  ; if not equal, increment bx by the write count
  add bx ax
  ; and continue writing
  jmp puts_strlen_start


  :puts_end

  ; print newline
  str puts_newline "\n"
  mov bx $puts_newline
  mov ax 1
  mov dx 1
  mov cx 1
  sys

  pop dx
  pop cx
  pop bx
ret


:main

; get argument count to EX
mov ax 13
sys
mov ex ax

str buff 100
; get argument chars to $buff
mov dx $buff
mov bx 1
:loop
mov cx 0
:nextchar
mov ax 14
sys
mov *dx al
cmp al fl
je nextarg
inc dx
inc cx
jmp nextchar
:nextarg
inc bx
cmp bx ex
je end
mov *dx ' '
inc dx
jmp loop
:end

mov ax $buff
;printstr ax
call puts

mov ax 12
mov bx 0
sys
