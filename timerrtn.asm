.8086

.MODEL large


PUBLIC timer_rtn_
PUBLIC _old_timer_rtn
;EXTERN _clock_ticks:WORD
PUBLIC _clock_ticks

_TEXT               SEGMENT BYTE PUBLIC USE16 'CODE'
                ASSUME CS:_TEXT                               

_clock_ticks: dw 0

timer_rtn_ proc far
  inc _clock_ticks
  
  db 0xEA ; jmp absolute 16:16
  _old_timer_rtn:
  dd 0 ; pointer to old timer routine
timer_rtn_ endp

_TEXT ENDS

END
