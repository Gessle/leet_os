data auto
code auto
stack 16
interrupts 1

jmp main

; strstr
; arguments: AX == pointer to str,
;            BX == pointer to ndl
; returns: NULL if ndl not found in str,
;          pointer to ndl in str if found
:strstr
  push bx
  push cx
  push dx
  push ex

  mov dx ax
  mov ex bx
  mov cx 0

  :strstr_search_begin
  mov ch *ax
  jcz strstr_search_end
  mov cl *bx
  cmp ch cl
  je strstr_search_continue
  jmp strstr_search_nomatch

  :strstr_search_continue
  inc ax
  inc bx
  mov cx 0
  mov cl *bx
  jcz strstr_search_found
  mov ch *ax
  cmp ch cl
  je strstr_search_continue
  jcz strstr_search_found
  mov cl 0
  jcz strstr_search_end

  :strstr_search_nomatch
  inc dx
  mov ax dx
  mov bx ex
  mov cx 0
  jmp strstr_search_begin

  :strstr_search_end
  mov ax 0
  jmp strstr_end

  :strstr_search_found
  mov ax dx

  :strstr_end

  pop ex
  pop dx
  pop cx
  pop bx
ret

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
; get argument count
mov ax 13
sys
mov cx ax
dec cx
jcz error

str needle 100
mov dx $needle
mov cx 0
:argv1loop
mov ax 14
mov bx 1
sys
mov *dx al
cmp ah al
je start_search
inc cx
inc dx
jmp argv1loop

str haystack 1024

:start_search
; read 1023 bytes from stdin
;mov ax 0
;mov bx $haystack
;mov cx 1023
;mov dx 0
;sys
mov ax $haystack
mov bx 1023
call ngets

; if read 0 bytes, exit
mov cx ax
jcz end

; append null byte to data
;add bx ax
;mov *bx 0

; search needle from haystack
mov ax $haystack
mov bx $needle
call strstr

mov cx ax
jcz start_search

; if needle found, write line to stdout
;mov bx $haystack
;printstr bx
mov ax $haystack
call puts


jmp start_search

:error

:end
mov ax 12
mov bx 0
sys

