Module: E:\OHJELM~1\FREEDO~1\303\VPU\vpudint.asm
GROUP: 'DGROUP' _DATA

Segment: vpudint_TEXT WORD USE16 00000000 bytes

Routine Size: 0 bytes,    Routine Base: vpudint_TEXT + 0000

No disassembly errors

Segment: _DATA WORD USE16 00000004 bytes
0000                          _int_to_vpu_next_handler:
0000    00 00 00 00                                     ....

Segment: main_TEXT BYTE USE16 0000001D bytes
0000                          dummy_interrupt_mask_handler_:
0000    50                        push        ax 
0001    B8 FF 00                  mov         ax,0x00ff 
0004    9A 00 00 00 00            call        int_to_vpu_ 
0009    B0 20                     mov         al,0x20 
000B    E6 20                     out         0x20,al 
000D    58                        pop         ax 
000E    CF                        iret        

Routine Size: 15 bytes,    Routine Base: main_TEXT + 0000

000F                          dummy_interrupt_mask_handler_cont_:

Routine Size: 0 bytes,    Routine Base: main_TEXT + 000F

000F                          dummy_interrupt_mask_handler_end_:
000F    50                        push        ax 
0010    B8 FF 00                  mov         ax,0x00ff 
0013    9A 00 00 00 00            call        int_to_vpu_ 
0018    58                        pop         ax 
0019    FF 2E 00 00               jmp         dword ptr DGROUP:_int_to_vpu_next_handler 
001D                          dummy_interrupt_mask_handler_cont_end:

Routine Size: 14 bytes,    Routine Base: main_TEXT + 000F

No disassembly errors

