.8086

.MODEL small

PUBLIC set_mode_

_TEXT               SEGMENT BYTE PUBLIC USE16 'CODE'
                ASSUME CS:_TEXT


set_mode_ proc far
  push ax
  mov ah, 0x00
  mov al, 0x12
  int 0x10
  pop ax
  retf
set_mode_ endp

_TEXT ENDS

END

