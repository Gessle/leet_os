data auto
code auto
stack 8
interrupts 1
; cat

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

; get argument count
mov ax 13
sys
mov cx ax
dec cx
jcz error

str file 13
mov dx $file
mov cx 0
:argv1loop
mov ax 14
mov bx 1
sys
mov *dx al
cmp ah al
je openfile
inc cx
inc dx
jmp argv1loop

:openfile
mov fx -1
mov ax 2
mov dx $file
mov bx 0
mov cx 0
sys
cmp fx ax
je error2

str buff 60

mov dx ax
mov ex 0
:readloop
mov bx $buff
mov ax 0
mov cx 59 
sys
cmp ax ex
je end
cmp ax fx
je error2
push bx
add bx ax
mov *bx 0
pop bx
push ax
mov ax bx
call put
mov bx ax
pop ax

jmp readloop

:error
str nofile "No file given"
mov ax $nofile
printstr ax
jmp end
:error2
str readerror "Read error"
mov ax $readerror
printstr ax

:end

mov ax 12
mov bx 0
sys
