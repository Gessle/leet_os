data auto
code auto
interrupts 1
stack 10

;more

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


; ngets (read max n bytes long string from fd 0)
; ax = pointer to string
; returns: pointer to string
:ngets
  push bx
  push cx
  push dx
  push ex
  push fx
  push gx
  
  mov gx ax

  mov ex 0
  mov fx bx
  dec fx

  mov bx ax

  :ngets_loop
  mov cx 1
  mov dx 0
  mov ax 0
  sys
  cmp ax cx
;  jne ngets_loop
  jne ngets_returnnull
  mov dh *bx   
  add ex ax
  add bx ax

  ; check if newline
  mov dl '\n' 
  cmp dh dl
  je ngets_end
  mov dl '\r'
  cmp dh dl
  je ngets_end

  ; check count of bytes read
  cmp ex fx
  je ngets_end

  jmp ngets_loop

  :ngets_end
  ; terminate string with null byte  
  mov *bx 0
  mov ax gx
  jmp ngets_returnstr

  :ngets_returnnull
  mov ax 0
  :ngets_returnstr

  pop gx
  pop fx
  pop ex
  pop dx
  pop cx
  pop bx
ret

:main

mov fx 10
byte unused
str buff 60

:loop
; read line from stdin
mov ax $buff
mov bx 59
call ngets
mov cx ax
jcz end
; write line to stdout
mov ax $buff
call put

inc ex
cmp ex fx
je sleep

jmp loop

:sleep

; make backup dup of stdin
word stdin_backup
mov ax 31
mov bx 0
sys
mov $stdin_backup ax

; reopen console as stdin
str console "CON"
word console_fp
mov ax 2
mov bx 0
mov cx 0
mov dx $console
mov ex 0
sys
mov $console_fp ax

; make dup of console_fp to stdin
mov ax 32
mov bx $console_fp
mov cx 0
sys

; close console_fp
mov ax 3
mov bx $console_fp
sys

; read from console
str string 2
mov ax 0
mov bx $string
mov cx 2
mov dx 0
sys

; make dup of stdin_backup to stdin
mov ax 32
mov bx $stdin_backup
mov cx 0
sys

; close stdin_backup
mov ax 3
mov bx $stdin_backup
sys

mov ex 0
jmp loop

:end
mov ax 12
mov bx 0
sys
