Module: E:\OHJELM~1\FREEDO~1\332\DRIVERS\null.c
GROUP: 'DGROUP' CONST,CONST2,_DATA

Segment: _TEXT BYTE USE16 000000C4 bytes
0000                          null_read_:
0000                          null_write_:
0000                          null_open_:
0000                          null_close_:
0000    31 C0                     xor         ax,ax 
0002    CB                        retf        

Routine Size: 3 bytes,    Routine Base: _TEXT + 0000

0003                          main_:
0003    52                        push        dx 
0004    56                        push        si 
0005    57                        push        di 
0006    55                        push        bp 
0007    89 E5                     mov         bp,sp 
0009    83 EC 30                  sub         sp,0x0030 
000C    89 C2                     mov         dx,ax 
000E    89 4E FE                  mov         word ptr -0x2[bp],cx 
0011    B9 10 00                  mov         cx,0x0010 
0014    8C D0                     mov         ax,ss 
0016    8E C0                     mov         es,ax 
0018    8D 7E D0                  lea         di,-0x30[bp] 
001B    B8 00 00                  mov         ax,DGROUP:CONST 
001E    8E D8                     mov         ds,ax 
0020    BE 00 00                  mov         si,offset DGROUP:L$8 
0023    F3 A5                     rep movsw   
0025    83 FA 02                  cmp         dx,0x0002 
0028    75 6E                     jne         L$3 
002A    8C 56 F0                  mov         word ptr -0x10[bp],ss 
002D    8D 46 D0                  lea         ax,-0x30[bp] 
0030    89 46 F2                  mov         word ptr -0xe[bp],ax 
0033    C7 46 F4 00 00            mov         word ptr -0xc[bp],0x0000 
0038    B8 10 00                  mov         ax,0x0010 
003B    50                        push        ax 
003C    8E 5E FE                  mov         ds,word ptr -0x2[bp] 
003F    8B 47 04                  mov         ax,word ptr 0x4[bx] 
0042    8B 57 06                  mov         dx,word ptr 0x6[bx] 
0045    31 DB                     xor         bx,bx 
0047    31 C9                     xor         cx,cx 
0049    E8 00 00                  call        strtoul_ 
004C    8D 5E F0                  lea         bx,-0x10[bp] 
004F    16                        push        ss 
0050    53                        push        bx 
0051    30 E4                     xor         ah,ah 
0053    8C D1                     mov         cx,ss 
0055    E8 00 00                  call        int86_ 
0058    83 7E F0 00               cmp         word ptr -0x10[bp],0x0000 
005C    74 3C                     je          L$4 
005E    B8 00 00                  mov         ax,offset DGROUP:L$7 
0061    BA 00 00                  mov         dx,DGROUP:CONST 
0064    E8 00 00                  call        puts_ 
0067    B8 00 00                  mov         ax,DGROUP:CONST 
006A    8E D8                     mov         ds,ax 
006C    83 3E 04 00 00            cmp         word ptr ___iob+0x4,0x0000 
0071    7E 11                     jle         L$1 
0073    C4 1E 00 00               les         bx,dword ptr ___iob 
0077    26 8A 07                  mov         al,byte ptr es:[bx] 
007A    30 E4                     xor         ah,ah 
007C    2D 0D 00                  sub         ax,0x000d 
007F    3D 0D 00                  cmp         ax,0x000d 
0082    77 0B                     ja          L$2 
0084                          L$1:
0084    B8 00 00                  mov         ax,offset ___iob 
0087    BA 00 00                  mov         dx,seg ___iob 
008A    E8 00 00                  call        fgetc_ 
008D    EB 2B                     jmp         L$6 
008F                          L$2:
008F    FF 0E 04 00               dec         word ptr ___iob+0x4 
0093    43                        inc         bx 
0094    89 1E 00 00               mov         word ptr ___iob,bx 
0098                          L$3:
0098    EB 20                     jmp         L$6 
009A                          L$4:
009A    B1 04                     mov         cl,0x04 
009C    B8 0F 00                  mov         ax,offset main_+0xf 
009F    D3 E8                     shr         ax,cl 
00A1    3D 12 00                  cmp         ax,0x0012 
00A4    73 08                     jae         L$5 
00A6    BA 12 00                  mov         dx,0x0012 
00A9    31 C0                     xor         ax,ax 
00AB    E8 00 00                  call        _dos_keep_ 
00AE                          L$5:
00AE    B1 04                     mov         cl,0x04 
00B0    BA 0F 00                  mov         dx,offset main_+0xf 
00B3    D3 EA                     shr         dx,cl 
00B5    31 C0                     xor         ax,ax 
00B7    E8 00 00                  call        _dos_keep_ 
00BA                          L$6:
00BA    B8 01 00                  mov         ax,0x0001 
00BD    89 EC                     mov         sp,bp 
00BF    5D                        pop         bp 
00C0    5F                        pop         di 
00C1    5E                        pop         si 
00C2    5A                        pop         dx 
00C3    C3                        ret         

Routine Size: 193 bytes,    Routine Base: _TEXT + 0003

No disassembly errors

Segment: CONST WORD USE16 0000002F bytes
0000                          L$7:
0000    45 72 72 6F 72 20 77 68 69 6C 65 20 6C 6F 61 64 Error while load
0010    69 6E 67 20 4E 55 4C 4C 20 6D 6F 64 75 6C 65 2E ing NULL module.
0020    20 50 72 65 73 73 20 61 6E 79 20 6B 65 79 00     Press any key.

Segment: CONST2 WORD USE16 00000000 bytes

Segment: _DATA WORD USE16 00000020 bytes
0000                          L$8:
0000    00 00 00 00                                     ....
0004    00 00 00 00                                     DD	null_read_
0008    00 00 00 00                                     DD	null_write_
000C    00 00 00 00                                     DD	null_open_
0010    00 00 00 00                                     DD	null_close_
0014    FF FF FF FF 00 00 00 00 FF FF FF FF             ............

