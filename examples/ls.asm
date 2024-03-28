code auto
data auto
stack 16
interrupts 1

; dir list example


str filename 13
;str stat_struct 20
word dir_desc 0

str dir ""

; open dir
mov ax 5
mov bx $dir
sys

cmp ax ex
jne end

mov $dir_desc ax
mov ex 0
mov cx $filename
:loop
;readdir
mov bx $dir_desc
mov ax 6
sys
cmp ax ex
jne close

;printstr cx
push ax
mov ax cx
call puts
mov cx ax
pop ax

jmp loop

;close dir
:close
mov ax 7
sys
:end

mov ax 12
mov bx 0
sys

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



