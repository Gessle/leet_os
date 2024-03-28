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
mov ax 0
mov bx $haystack
mov cx 1023
mov dx 0
sys

; if read 0 bytes, exit
mov cx ax
jcz end

; search needle from haystack
mov ax $haystack
mov bx $needle
call strstr

mov cx ax
jcz start_search

mov bx $haystack
printstr bx


jmp start_search



:end
mov ax 12
mov bx 0
sys

