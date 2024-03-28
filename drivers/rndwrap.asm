; assembly wrappers for random generator functions

.8086

.MODEL small


PUBLIC random_write_
PUBLIC random_close_
EXTERN _random_seed:WORD


_TEXT               SEGMENT BYTE PUBLIC USE16 'CODE'
                ASSUME CS:_TEXT

; dx:bx = buffer
; cx = count
random_write_ proc far
  push cx
  push es
  push bx

  mov es, dx
  xor ah, ah

  random_write_loop:
  test cx, 0xFFFF
  jz random_loop_end
  mov al, byte ptr [es:bx]
  add cs:_random_seed, ax
  inc bx
  dec cx
  jmp random_write_loop

  random_loop_end:
 
  pop bx
  pop es
  pop ax
  ret
retf
random_write_ endp

random_close_ proc far
  xor ax, ax
retf
random_close_ endp

_TEXT ENDS

END

