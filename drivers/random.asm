Module: E:\OHJELM~1\FREEDO~1\333\DRIVERS\random.c
GROUP: 'DGROUP' CONST,CONST2,_DATA

Segment: _TEXT BYTE USE16 0000013E bytes
0000                          _random_seed:
0000    00 00                                           ..
0002                          _new_bp:
0002    00 00                                           ..
0004                          _random_inc_2:
0004    00 00                                           ..
0006                          _random_inc_1:
0006    00 00                                           ..
0008                          _random_inc_0:
0008    00 00                                           ..
000A                          _new_ss:
000A    00 00                                           ..
000C                          _new_sp:
000C    00 00                                           ..
000E                          _old_bp:
000E    00 00                                           ..
0010                          _old_sp:
0010    00 00                                           ..
0012                          _old_ss:
0012    00 00                                           ..

Routine Size: 20 bytes,    Routine Base: _TEXT + 0000

0014                          random_:
0014    53                        push        bx 
0015    51                        push        cx 
0016    52                        push        dx 
0017    56                        push        si 
0018    2E A1 00 00               mov         ax,word ptr cs:_random_inc_0 
001C    05 0C 00                  add         ax,0x000c 
001F    2E FF 06 00 00            inc         word ptr cs:_random_inc_0 
0024    2E 01 06 00 00            add         word ptr cs:_random_inc_1,ax 
0029    2E A1 00 00               mov         ax,word ptr cs:_random_inc_1 
002D    2D 13 00                  sub         ax,0x0013 
0030    2E 29 06 00 00            sub         word ptr cs:_random_inc_2,ax 
0035    2E A1 00 00               mov         ax,word ptr cs:_random_inc_0 
0039    2E 03 06 00 00            add         ax,word ptr cs:_random_inc_1 
003E    2E 8B 36 00 00            mov         si,word ptr cs:_random_inc_2 
0043    01 C6                     add         si,ax 
0045    2E 8B 1E 00 00            mov         bx,word ptr cs:_random_seed 
004A    89 D8                     mov         ax,bx 
004C    D1 E0                     shl         ax,0x01 
004E    D1 E0                     shl         ax,0x01 
0050    29 D8                     sub         ax,bx 
0052    B1 03                     mov         cl,0x03 
0054    D3 E0                     shl         ax,cl 
0056    29 D8                     sub         ax,bx 
0058    31 D2                     xor         dx,dx 
005A    B9 6F 67                  mov         cx,0x676f 
005D    F7 F1                     div         cx 
005F    2E 89 16 00 00            mov         word ptr cs:_random_seed,dx 
0064    89 F0                     mov         ax,si 
0066    01 D8                     add         ax,bx 
0068    30 E4                     xor         ah,ah 
006A    5E                        pop         si 
006B    5A                        pop         dx 
006C    59                        pop         cx 
006D    5B                        pop         bx 
006E    C3                        ret         

Routine Size: 91 bytes,    Routine Base: _TEXT + 0014

006F                          main_:
006F    52                        push        dx 
0070    56                        push        si 
0071    57                        push        di 
0072    55                        push        bp 
0073    89 E5                     mov         bp,sp 
0075    83 EC 30                  sub         sp,0x0030 
0078    89 C2                     mov         dx,ax 
007A    89 4E FE                  mov         word ptr -0x2[bp],cx 
007D    B9 10 00                  mov         cx,0x0010 
0080    8C D0                     mov         ax,ss 
0082    8E C0                     mov         es,ax 
0084    8D 7E D0                  lea         di,-0x30[bp] 
0087    B8 00 00                  mov         ax,DGROUP:CONST 
008A    8E D8                     mov         ds,ax 
008C    BE 00 00                  mov         si,offset DGROUP:L$8 
008F    F3 A5                     rep movsw   
0091    83 FA 02                  cmp         dx,0x0002 
0094    75 6E                     jne         L$3 
0096    8C 56 F0                  mov         word ptr -0x10[bp],ss 
0099    8D 46 D0                  lea         ax,-0x30[bp] 
009C    89 46 F2                  mov         word ptr -0xe[bp],ax 
009F    C7 46 F4 00 00            mov         word ptr -0xc[bp],0x0000 
00A4    B8 10 00                  mov         ax,0x0010 
00A7    50                        push        ax 
00A8    8E 5E FE                  mov         ds,word ptr -0x2[bp] 
00AB    8B 47 04                  mov         ax,word ptr 0x4[bx] 
00AE    8B 57 06                  mov         dx,word ptr 0x6[bx] 
00B1    31 DB                     xor         bx,bx 
00B3    31 C9                     xor         cx,cx 
00B5    E8 00 00                  call        strtoul_ 
00B8    8D 5E F0                  lea         bx,-0x10[bp] 
00BB    16                        push        ss 
00BC    53                        push        bx 
00BD    30 E4                     xor         ah,ah 
00BF    8C D1                     mov         cx,ss 
00C1    E8 00 00                  call        int86_ 
00C4    83 7E F0 00               cmp         word ptr -0x10[bp],0x0000 
00C8    74 3C                     je          L$4 
00CA    B8 00 00                  mov         ax,offset DGROUP:L$7 
00CD    BA 00 00                  mov         dx,DGROUP:CONST 
00D0    E8 00 00                  call        puts_ 
00D3    B8 00 00                  mov         ax,DGROUP:CONST 
00D6    8E D8                     mov         ds,ax 
00D8    83 3E 04 00 00            cmp         word ptr ___iob+0x4,0x0000 
00DD    7E 11                     jle         L$1 
00DF    C4 1E 00 00               les         bx,dword ptr ___iob 
00E3    26 8A 07                  mov         al,byte ptr es:[bx] 
00E6    30 E4                     xor         ah,ah 
00E8    2D 0D 00                  sub         ax,0x000d 
00EB    3D 0D 00                  cmp         ax,0x000d 
00EE    77 0B                     ja          L$2 
00F0                          L$1:
00F0    B8 00 00                  mov         ax,offset ___iob 
00F3    BA 00 00                  mov         dx,seg ___iob 
00F6    E8 00 00                  call        fgetc_ 
00F9    EB 39                     jmp         L$6 
00FB                          L$2:
00FB    FF 0E 04 00               dec         word ptr ___iob+0x4 
00FF    43                        inc         bx 
0100    89 1E 00 00               mov         word ptr ___iob,bx 
0104                          L$3:
0104    EB 2E                     jmp         L$6 
0106                          L$4:
0106    2E C7 06 00 00 FF 00      mov         word ptr cs:_new_sp,offset main_+0xff 
010D    2E C7 06 00 00 00 00      mov         word ptr cs:_new_bp,offset main_ 
0114    B1 04                     mov         cl,0x04 
0116    B8 0E 01                  mov         ax,offset main_+0x10e 
0119    D3 E8                     shr         ax,cl 
011B    3D 12 00                  cmp         ax,0x0012 
011E    73 08                     jae         L$5 
0120    BA 12 00                  mov         dx,0x0012 
0123    31 C0                     xor         ax,ax 
0125    E8 00 00                  call        _dos_keep_ 
0128                          L$5:
0128    B1 04                     mov         cl,0x04 
012A    BA 0E 01                  mov         dx,offset main_+0x10e 
012D    D3 EA                     shr         dx,cl 
012F    31 C0                     xor         ax,ax 
0131    E8 00 00                  call        _dos_keep_ 
0134                          L$6:
0134    B8 01 00                  mov         ax,0x0001 
0137    89 EC                     mov         sp,bp 
0139    5D                        pop         bp 
013A    5F                        pop         di 
013B    5E                        pop         si 
013C    5A                        pop         dx 
013D    C3                        ret         

Routine Size: 207 bytes,    Routine Base: _TEXT + 006F

No disassembly errors

Segment: CONST WORD USE16 00000031 bytes
0000                          L$7:
0000    45 72 72 6F 72 20 77 68 69 6C 65 20 6C 6F 61 64 Error while load
0010    69 6E 67 20 52 41 4E 44 4F 4D 20 6D 6F 64 75 6C ing RANDOM modul
0020    65 2E 20 50 72 65 73 73 20 61 6E 79 20 6B 65 79 e. Press any key
0030    00                                              .

Segment: CONST2 WORD USE16 00000000 bytes

Segment: _DATA WORD USE16 00000020 bytes
0000                          L$8:
0000    00 00 00 00                                     ....
0004    00 00 00 00                                     DD	random_read_
0008    00 00 00 00                                     DD	random_write_
000C    00 00 00 00                                     DD	random_write_
0010    00 00 00 00                                     DD	random_close_
0014    FF FF FF FF 00 00 00 00 FF FF FF FF             ............

