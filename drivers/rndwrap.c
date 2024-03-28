.8086

.MODEL compact


PUBLIC __PRO
PUBLIC __EPI

DGROUP          GROUP   _DATA
main_TEXT               SEGMENT BYTE PUBLIC USE16 'CODE'
                ASSUME CS:main_TEXT, DS:DGROUP, SS:DGROUP


__PRO proc far
  push bx
  push cx 
  

  mov bx, [ss:bp+2]     ;; si = caller's ip
  mov cx, [ss:bp+4]    ;; cx = caller's code segment
    
;  debug_callstack_push(far ptr cx:bx);
  call far ptr debug_callstack_push_
  
  pop cx
  pop bx
retf

__PRO endp

__EPI proc far

  call far ptr debug_callstack_pop_
retf

__EPI endp

main_TEXT ENDS

_DATA           SEGMENT WORD PUBLIC USE16 'DATA'

_DATA ENDS

END

