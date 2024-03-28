Module: E:\OHJELM~1\FREEDO~1\309\vpu\vpuhwint.c
GROUP: 'DGROUP' CONST,CONST2,_DATA

Segment: vpuhwint_TEXT BYTE USE16 0000013E bytes
0000    69 6E 74 5F 74 6F 5F 76 70 75 0A                int_to_vpu.

Routine Size: 11 bytes,    Routine Base: vpuhwint_TEXT + 0000

000B                          int_to_vpu_:
000B    45                        inc         bp 
000C    55                        push        bp 
000D    89 E5                     mov         bp,sp 
000F    9A 00 00 00 00            call        __PRO 
0014    53                        push        bx 
0015    51                        push        cx 
0016    52                        push        dx 
0017    56                        push        si 
0018    57                        push        di 
0019    83 EC 06                  sub         sp,0x0006 
001C    88 46 F4                  mov         byte ptr -0xc[bp],al 
001F    B8 00 00                  mov         ax,DGROUP:CONST 
0022    8E D8                     mov         ds,ax 
0024    8B 0E 00 00               mov         cx,word ptr _vpu_hwint_count 
0028    B8 00 00                  mov         ax,seg _disable_vpu_interrupts 
002B    8E D8                     mov         ds,ax 
002D    83 3E 00 00 00            cmp         word ptr _disable_vpu_interrupts,0x0000 
0032    74 03                     je          L$1 
0034    E9 F7 00                  jmp         L$8 
0037                          L$1:
0037    49                        dec         cx 
0038    83 F9 FF                  cmp         cx,0xffff 
003B    74 19                     je          L$2 
003D    89 C8                     mov         ax,cx 
003F    BB 0C 00                  mov         bx,0x000c 
0042    F7 EB                     imul        bx 
0044    BA 00 00                  mov         dx,DGROUP:CONST 
0047    8E DA                     mov         ds,dx 
0049    C5 1E 00 00               lds         bx,dword ptr _vpu_hardware_int_table 
004D    01 C3                     add         bx,ax 
004F    8A 07                     mov         al,byte ptr [bx] 
0051    3A 46 F4                  cmp         al,byte ptr -0xc[bp] 
0054    75 E1                     jne         L$1 
0056                          L$2:
0056    89 C8                     mov         ax,cx 
0058    BB 0C 00                  mov         bx,0x000c 
005B    F7 EB                     imul        bx 
005D    BA 00 00                  mov         dx,DGROUP:CONST 
0060    8E DA                     mov         ds,dx 
0062    C5 36 00 00               lds         si,dword ptr _vpu_hardware_int_table 
0066    01 C6                     add         si,ax 
0068    8B 7C 04                  mov         di,word ptr 0x4[si] 
006B    8B 44 06                  mov         ax,word ptr 0x6[si] 
006E    89 46 F0                  mov         word ptr -0x10[bp],ax 
0071    89 FB                     mov         bx,di 
0073    89 46 F2                  mov         word ptr -0xe[bp],ax 
0076    8B 44 02                  mov         ax,word ptr 0x2[si] 
0079    8E 5E F0                  mov         ds,word ptr -0x10[bp] 
007C    89 FE                     mov         si,di 
007E    89 84 8C 00               mov         word ptr 0x8c[si],ax 
0082                          L$3:
0082    8E 5E F2                  mov         ds,word ptr -0xe[bp] 
0085    8B 97 4C 01               mov         dx,word ptr 0x14c[bx] 
0089    8B 87 4E 01               mov         ax,word ptr 0x14e[bx] 
008D    85 C0                     test        ax,ax 
008F    75 04                     jne         L$4 
0091    85 D2                     test        dx,dx 
0093    74 3B                     je          L$6 
0095                          L$4:
0095    89 D6                     mov         si,dx 
0097    89 D3                     mov         bx,dx 
0099    89 46 F2                  mov         word ptr -0xe[bp],ax 
009C    8E D8                     mov         ds,ax 
009E    8B 8C 56 01               mov         cx,word ptr 0x156[si] 
00A2                          L$5:
00A2    49                        dec         cx 
00A3    83 F9 FF                  cmp         cx,0xffff 
00A6    74 DA                     je          L$3 
00A8    8E 5E F2                  mov         ds,word ptr -0xe[bp] 
00AB    8B 87 56 01               mov         ax,word ptr 0x156[bx] 
00AF    D1 E0                     shl         ax,0x01 
00B1    D1 E0                     shl         ax,0x01 
00B3    C5 B7 52 01               lds         si,dword ptr 0x152[bx] 
00B7    01 C6                     add         si,ax 
00B9    8B 14                     mov         dx,word ptr [si] 
00BB    8B 44 02                  mov         ax,word ptr 0x2[si] 
00BE    3B 46 F0                  cmp         ax,word ptr -0x10[bp] 
00C1    75 DF                     jne         L$5 
00C3    39 FA                     cmp         dx,di 
00C5    75 DB                     jne         L$5 
00C7    8E 5E F2                  mov         ds,word ptr -0xe[bp] 
00CA    89 8F 58 01               mov         word ptr 0x158[bx],cx 
00CE    EB B2                     jmp         L$3 
00D0                          L$6:
00D0    8E 5E F0                  mov         ds,word ptr -0x10[bp] 
00D3    8B 05                     mov         ax,word ptr [di] 
00D5    BA 00 00                  mov         dx,DGROUP:CONST 
00D8    8E DA                     mov         ds,dx 
00DA    A3 00 00                  mov         word ptr _steps,ax 
00DD    8B 0E 00 00               mov         cx,word ptr _window_count 
00E1                          L$7:
00E1    49                        dec         cx 
00E2    83 F9 FF                  cmp         cx,0xffff 
00E5    74 47                     je          L$8 
00E7    89 C8                     mov         ax,cx 
00E9    D1 E0                     shl         ax,0x01 
00EB    D1 E0                     shl         ax,0x01 
00ED    BA 00 00                  mov         dx,DGROUP:CONST 
00F0    8E DA                     mov         ds,dx 
00F2    C5 3E 00 00               lds         di,dword ptr _windows 
00F6    01 C7                     add         di,ax 
00F8    C5 35                     lds         si,dword ptr [di] 
00FA    8B 7C 3C                  mov         di,word ptr 0x3c[si] 
00FD    8B 44 3E                  mov         ax,word ptr 0x3e[si] 
0100    3D 00 00                  cmp         ax,seg vpuconsole_program_ 
0103    75 DC                     jne         L$7 
0105    81 FF 00 00               cmp         di,offset vpuconsole_program_ 
0109    75 D6                     jne         L$7 
010B    C4 7C 40                  les         di,dword ptr 0x40[si] 
010E    26 8B 55 16               mov         dx,word ptr es:0x16[di] 
0112    26 8B 45 18               mov         ax,word ptr es:0x18[di] 
0116    3B 46 F2                  cmp         ax,word ptr -0xe[bp] 
0119    75 C6                     jne         L$7 
011B    39 DA                     cmp         dx,bx 
011D    75 C2                     jne         L$7 
011F    B8 00 00                  mov         ax,DGROUP:CONST 
0122    8E C0                     mov         es,ax 
0124    26 89 36 00 00            mov         word ptr es:_running_window,si 
0129    26 8C 1E 02 00            mov         word ptr es:_running_window+0x2,ds 
012E                          L$8:
012E    8D 66 F6                  lea         sp,-0xa[bp] 
0131    5F                        pop         di 
0132    5E                        pop         si 
0133    5A                        pop         dx 
0134    59                        pop         cx 
0135    5B                        pop         bx 
0136    9A 00 00 00 00            call        __EPI 
013B    5D                        pop         bp 
013C    4D                        dec         bp 
013D    CB                        retf        

Routine Size: 307 bytes,    Routine Base: vpuhwint_TEXT + 000B

No disassembly errors

Segment: CONST WORD USE16 00000000 bytes

Segment: CONST2 WORD USE16 00000000 bytes

Segment: _DATA WORD USE16 00000000 bytes

