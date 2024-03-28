.8086

.MODEL large

EXTRN int_to_vpu_:BYTE
;EXTRN _int_to_vpu_next_handler:FAR PTR

PUBLIC dummy_interrupt_mask_handler_
PUBLIC dummy_interrupt_mask_handler_end_
PUBLIC dummy_interrupt_mask_handler_cont_
PUBLIC dummy_interrupt_mask_handler_cont_end_

;PUBLIC _int_to_vpu_next_handler

DGROUP          GROUP   _DATA
main_TEXT               SEGMENT BYTE PUBLIC USE16 'CODE'
;                ASSUME CS:main_TEXT, DS:DGROUP, SS:DGROUP
                ASSUME CS:main_TEXT


dummy_interrupt_mask_handler_ proc far

push ax
mov ax, 0xFF
cld
call far ptr int_to_vpu_

mov al, 0x20
out 0x20, al
pop ax
iret

dummy_interrupt_mask_handler_ endp

dummy_interrupt_mask_handler_end_ proc far
dummy_interrupt_mask_handler_end_ endp

;_int_to_vpu_next_handler_seg SEGMENT USE16 AT 0xFFFF
;_int_to_vpu_next_handler LABEL FAR
;_int_to_vpu_next_handler_seg ENDS

dummy_interrupt_mask_handler_cont_ proc far

push ax
mov ax, 0xFF
cld
call far ptr int_to_vpu_
pop ax

jmpf far ptr int_to_vpu_

dummy_label:

;jmp [_int_to_vpu_next_handler]

;_int_to_vpu_next_handler dd 0

dummy_interrupt_mask_handler_cont_ endp

dummy_interrupt_mask_handler_cont_end proc far
dummy_interrupt_mask_handler_cont_end endp

main_TEXT ENDS

_DATA           SEGMENT WORD PUBLIC USE16 'DATA'

_DATA ENDS

END

