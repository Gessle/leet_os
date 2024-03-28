Module: E:\OHJELM~1\FREEDO~1\388\DRIVERS\tcpip.c
GROUP: 'DGROUP' CONST,CONST2,_DATA

Segment: _TEXT BYTE USE16 0000022C bytes
0000                          _open_sockets:
0000    00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
	--- Above line repeats 6 times ---
0070    00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
0080                          _open_socket_count:
0080    00 00                                           ..

Routine Size: 130 bytes,    Routine Base: _TEXT + 0000

0082                          socket_read_:
0082    53                        push        bx 
0083    89 C3                     mov         bx,ax 
0085    B8 02 00                  mov         ax,0x0002 
0088    CD 61                     int         0x61 
008A    5B                        pop         bx 
008B    CB                        retf        

Routine Size: 10 bytes,    Routine Base: _TEXT + 0082

008C                          socket_write_:
008C    51                        push        cx 
008D    89 C1                     mov         cx,ax 
008F    B8 03 00                  mov         ax,0x0003 
0092    CD 61                     int         0x61 
0094    59                        pop         cx 
0095    CB                        retf        

Routine Size: 10 bytes,    Routine Base: _TEXT + 008C

0096                          socket_is_open_:
0096    53                        push        bx 
0097    52                        push        dx 
0098    89 C2                     mov         dx,ax 
009A    2E A1 00 00               mov         ax,word ptr cs:_open_socket_count 
009E                          L$1:
009E    48                        dec         ax 
009F    3D FF FF                  cmp         ax,0xffff 
00A2    74 0B                     je          L$2 
00A4    89 C3                     mov         bx,ax 
00A6    D1 E3                     shl         bx,0x01 
00A8    2E 3B 97 00 00            cmp         dx,word ptr cs:_open_sockets[bx] 
00AD    75 EF                     jne         L$1 
00AF                          L$2:
00AF    5A                        pop         dx 
00B0    5B                        pop         bx 
00B1    C3                        ret         

Routine Size: 28 bytes,    Routine Base: _TEXT + 0096

00B2                          socket_close_:
00B2    53                        push        bx 
00B3    51                        push        cx 
00B4    52                        push        dx 
00B5    89 C1                     mov         cx,ax 
00B7    E8 00 00                  call        socket_is_open_ 
00BA    85 C0                     test        ax,ax 
00BC    7C 1F                     jl          L$3 
00BE    2E FF 0E 00 00            dec         word ptr cs:_open_socket_count 
00C3    2E 8B 1E 00 00            mov         bx,word ptr cs:_open_socket_count 
00C8    29 C3                     sub         bx,ax 
00CA    D1 E3                     shl         bx,0x01 
00CC    D1 E0                     shl         ax,0x01 
00CE    89 C2                     mov         dx,ax 
00D0    83 C2 02                  add         dx,0x0002 
00D3    81 C2 00 00               add         dx,offset _open_sockets 
00D7    05 00 00                  add         ax,offset _open_sockets 
00DA    E8 00 00                  call        memmove_ 
00DD                          L$3:
00DD    89 CB                     mov         bx,cx 
00DF    B8 04 00                  mov         ax,0x0004 
00E2    CD 61                     int         0x61 
00E4    5A                        pop         dx 
00E5    59                        pop         cx 
00E6    5B                        pop         bx 
00E7    CB                        retf        

Routine Size: 54 bytes,    Routine Base: _TEXT + 00B2

00E8                          socket_open_:
00E8    52                        push        dx 
00E9    56                        push        si 
00EA    89 D8                     mov         ax,bx 
00EC    E8 00 00                  call        socket_is_open_ 
00EF    85 C0                     test        ax,ax 
00F1    7D 09                     jge         L$4 
00F3    2E A1 00 00               mov         ax,word ptr cs:_open_socket_count 
00F7    3D 3F 00                  cmp         ax,0x003f 
00FA    75 06                     jne         L$5 
00FC                          L$4:
00FC    B8 FF FF                  mov         ax,0xffff 
00FF    5E                        pop         si 
0100    5A                        pop         dx 
0101    CB                        retf        
0102                          L$5:
0102    89 C2                     mov         dx,ax 
0104    89 C6                     mov         si,ax 
0106    D1 E6                     shl         si,0x01 
0108    2E 89 9C 00 00            mov         word ptr cs:_open_sockets[si],bx 
010D    31 C0                     xor         ax,ax 
010F    42                        inc         dx 
0110    2E 89 16 00 00            mov         word ptr cs:_open_socket_count,dx 
0115    5E                        pop         si 
0116    5A                        pop         dx 
0117    CB                        retf        

Routine Size: 48 bytes,    Routine Base: _TEXT + 00E8

0118                          open_port_:
0118    53                        push        bx 
0119    51                        push        cx 
011A    89 D1                     mov         cx,dx 
011C    89 C3                     mov         bx,ax 
011E    B8 05 00                  mov         ax,0x0005 
0121    CD 61                     int         0x61 
0123    59                        pop         cx 
0124    5B                        pop         bx 
0125    C3                        ret         

Routine Size: 14 bytes,    Routine Base: _TEXT + 0118

0126                          tcp_cmd_:
0126    51                        push        cx 
0127    56                        push        si 
0128    1E                        push        ds 
0129    89 C3                     mov         bx,ax 
012B    8E DA                     mov         ds,dx 
012D    8A 07                     mov         al,byte ptr [bx] 
012F    8D 77 03                  lea         si,0x3[bx] 
0132    3C 02                     cmp         al,0x02 
0134    72 10                     jb          L$6 
0136    76 29                     jbe         L$8 
0138    3C FF                     cmp         al,0xff 
013A    74 44                     je          L$12 
013C    3C 04                     cmp         al,0x04 
013E    74 38                     je          L$11 
0140    3C 03                     cmp         al,0x03 
0142    74 2A                     je          L$9 
0144    EB 41                     jmp         L$13 
0146                          L$6:
0146    3C 01                     cmp         al,0x01 
0148    74 0B                     je          L$7 
014A    84 C0                     test        al,al 
014C    75 39                     jne         L$13 
014E    B8 00 00                  mov         ax,0x0000 
0151    CD 61                     int         0x61 
0153    EB 35                     jmp         L$14 
0155                          L$7:
0155    8B 5F 01                  mov         bx,word ptr 0x1[bx] 
0158    31 C9                     xor         cx,cx 
015A    B8 01 00                  mov         ax,0x0001 
015D    CD 61                     int         0x61 
015F    EB 29                     jmp         L$14 
0161                          L$8:
0161    8B 5F 01                  mov         bx,word ptr 0x1[bx] 
0164    B9 01 00                  mov         cx,0x0001 
0167    B8 01 00                  mov         ax,0x0001 
016A    CD 61                     int         0x61 
016C    EB 1C                     jmp         L$14 
016E                          L$9:
016E    8B 47 01                  mov         ax,word ptr 0x1[bx] 
0171    31 D2                     xor         dx,dx 
0173                          L$10:
0173    E8 00 00                  call        open_port_ 
0176    EB 12                     jmp         L$14 
0178                          L$11:
0178    8B 47 01                  mov         ax,word ptr 0x1[bx] 
017B    BA 01 00                  mov         dx,0x0001 
017E    EB F3                     jmp         L$10 
0180                          L$12:
0180    B8 06 00                  mov         ax,0x0006 
0183    CD 61                     int         0x61 
0185    EB 03                     jmp         L$14 
0187                          L$13:
0187    B8 FF FF                  mov         ax,0xffff 
018A                          L$14:
018A    1F                        pop         ds 
018B    5E                        pop         si 
018C    59                        pop         cx 
018D    CB                        retf        

Routine Size: 104 bytes,    Routine Base: _TEXT + 0126

018E                          main_:
018E    53                        push        bx 
018F    51                        push        cx 
0190    56                        push        si 
0191    57                        push        di 
0192    1E                        push        ds 
0193    06                        push        es 
0194    55                        push        bp 
0195    89 E5                     mov         bp,sp 
0197    83 EC 2E                  sub         sp,0x002e 
019A    50                        push        ax 
019B    89 D3                     mov         bx,dx 
019D    B9 10 00                  mov         cx,0x0010 
01A0    8C D0                     mov         ax,ss 
01A2    8E C0                     mov         es,ax 
01A4    8D 7E D2                  lea         di,-0x2e[bp] 
01A7    B8 00 00                  mov         ax,DGROUP:CONST 
01AA    8E D8                     mov         ds,ax 
01AC    BE 00 00                  mov         si,offset DGROUP:L$20 
01AF    F3 A5                     rep movsw   
01B1    83 7E D0 02               cmp         word ptr -0x30[bp],0x0002 
01B5    75 68                     jne         L$18 
01B7    8C 56 F2                  mov         word ptr -0xe[bp],ss 
01BA    8D 46 D2                  lea         ax,-0x2e[bp] 
01BD    89 46 F4                  mov         word ptr -0xc[bp],ax 
01C0    C7 46 F6 00 00            mov         word ptr -0xa[bp],0x0000 
01C5    8B 47 02                  mov         ax,word ptr 0x2[bx] 
01C8    BB 10 00                  mov         bx,0x0010 
01CB    31 D2                     xor         dx,dx 
01CD    E8 00 00                  call        strtoul_ 
01D0    30 E4                     xor         ah,ah 
01D2    8D 5E F2                  lea         bx,-0xe[bp] 
01D5    8D 56 F2                  lea         dx,-0xe[bp] 
01D8    E8 00 00                  call        int86_ 
01DB    8B 46 F2                  mov         ax,word ptr -0xe[bp] 
01DE    85 C0                     test        ax,ax 
01E0    74 30                     je          L$17 
01E2    B8 00 00                  mov         ax,offset DGROUP:L$19 
01E5    E8 00 00                  call        puts_ 
01E8    83 3E 02 00 00            cmp         word ptr ___iob+0x2,0x0000 
01ED    7E 10                     jle         L$15 
01EF    8B 1E 00 00               mov         bx,word ptr ___iob 
01F3    8A 07                     mov         al,byte ptr [bx] 
01F5    30 E4                     xor         ah,ah 
01F7    2D 0D 00                  sub         ax,0x000d 
01FA    3D 0D 00                  cmp         ax,0x000d 
01FD    77 08                     ja          L$16 
01FF                          L$15:
01FF    B8 00 00                  mov         ax,offset ___iob 
0202    E8 00 00                  call        fgetc_ 
0205    EB 18                     jmp         L$18 
0207                          L$16:
0207    FF 0E 02 00               dec         word ptr ___iob+0x2 
020B    43                        inc         bx 
020C    89 1E 00 00               mov         word ptr ___iob,bx 
0210    EB 0D                     jmp         L$18 
0212                          L$17:
0212    B1 04                     mov         cl,0x04 
0214    BA 0F 00                  mov         dx,offset main_+0xf 
0217    D3 EA                     shr         dx,cl 
0219    83 C2 10                  add         dx,0x0010 
021C    E8 00 00                  call        _dos_keep_ 
021F                          L$18:
021F    B8 01 00                  mov         ax,0x0001 
0222    89 EC                     mov         sp,bp 
0224    5D                        pop         bp 
0225    07                        pop         es 
0226    1F                        pop         ds 
0227    5F                        pop         di 
0228    5E                        pop         si 
0229    59                        pop         cx 
022A    5B                        pop         bx 
022B    C3                        ret         

Routine Size: 158 bytes,    Routine Base: _TEXT + 018E

No disassembly errors

Segment: CONST WORD USE16 00000030 bytes
0000                          L$19:
0000    45 72 72 6F 72 20 77 68 69 6C 65 20 6C 6F 61 64 Error while load
0010    69 6E 67 20 54 43 50 49 50 20 6D 6F 64 75 6C 65 ing TCPIP module
0020    2E 20 50 72 65 73 73 20 61 6E 79 20 6B 65 79 00 . Press any key.

Segment: CONST2 WORD USE16 00000000 bytes

Segment: _DATA WORD USE16 00000020 bytes
0000                          L$20:
0000    00 00 00 00                                     DD	tcp_cmd_
0004    00 00 00 00                                     DD	socket_read_
0008    00 00 00 00                                     DD	socket_write_
000C    00 00 00 00                                     DD	socket_open_
0010    00 00 00 00                                     DD	socket_close_
0014    FF FF FF FF 80 00 00 00 FF 7F 00 00             ............

