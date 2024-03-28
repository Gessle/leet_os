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
str heinapaali "http://sininenankka.dy.fi"
str neula "ankka"

mov ax $heinapaali
mov bx $neula
call strstr
printuint ax
