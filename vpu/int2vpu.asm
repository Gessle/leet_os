.8086

.MODEL large

EXTRN int_to_vpu_:BYTE

PUBLIC __PRO
PUBLIC __EPI

DGROUP          GROUP   _DATA
main_TEXT               SEGMENT BYTE PUBLIC USE16 'CODE'
                ASSUME CS:main_TEXT, DS:DGROUP, SS:DGROUP


dummy_interrupt_mask_handler_ proc far

push ax
mov ax, 0xFF
call far ptr int_to_vpu_
pop ax
iret

dummy_interrupt_mask_handler_ endp

dummy_interrupt_mask_handler_end_ proc far
dummy_interrupt_mask_handler_end_ endp

main_TEXT ENDS

_DATA           SEGMENT WORD PUBLIC USE16 'DATA'

_DATA ENDS

END

