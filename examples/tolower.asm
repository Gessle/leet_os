data auto
code auto
interrupts 1
stack 10

jmp main

; write a string to stdout (file descriptor 1)
; arguments: AX == pointer to str
; returns: non-negative if success, -1 if error
:put
  push bx
  push cx
  push dx

  mov bx ax

  :put_strlen_start
  push bx

  ; get length of the string to cx
  mov ax 0
  mov cx 0
  :put_strlen_loop
  mov ah *bx
  cmp ah al
  je put_strlen_end
  inc cx
  inc bx
  jmp put_strlen_loop
  :put_strlen_end  
  pop bx

  jcz put_end
  ; write
  mov ax 1
  mov dx 1
  sys

  ; compare str length and count of written bytes
  cmp ax cx
  je put_end
  ; if not equal, increment bx by the write count
  add bx ax
  ; and continue writing
  jmp put_strlen_start


  :put_end

  pop dx
  pop cx
  pop bx
ret

:main

str buff 60
:loop
; read stdin
mov ax 0
mov bx $buff
mov cx 59
mov dx 0
sys

mov cx ax
jcz end
push bx
add bx cx

;null-terminate the string that was read
mov *bx 0

pop bx

mov ah 0x20
mov cl 0x41
mov ch 0x5A
mov dx 0
:lowercase
mov al *bx
cmp al dl
je endlowercase
cmp al cl
jl skiplowercase
cmp al ch
jg skiplowercase
add al ah
mov *bx al
:skiplowercase
inc bx
jmp lowercase
:endlowercase

; write to stdout
mov ax $buff
call put
jmp loop

:end

mov ax 12
mov bx 0
sys
