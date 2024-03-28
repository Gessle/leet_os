Module: E:\OHJELM~1\FREEDO~1\346\DRIVERS\dbuffvga.c
GROUP: 'DGROUP' CONST,CONST2,_DATA

Segment: _TEXT BYTE USE16 000007D9 bytes
0000                          _plane:
0000    00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
0010                          _VGA:
0010    00 00 00 A0                                     ....

Routine Size: 20 bytes,    Routine Base: _TEXT + 0000

0014                          update_screen_:
0014    53                        push        bx 
0015    51                        push        cx 
0016    52                        push        dx 
0017    56                        push        si 
0018    57                        push        di 
0019    1E                        push        ds 
001A    06                        push        es 
001B    BB 04 00                  mov         bx,0x0004 
001E    B0 05                     mov         al,0x05 
0020    BA CE 03                  mov         dx,0x03ce 
0023    EE                        out         dx,al 
0024    30 C0                     xor         al,al 
0026    BA CF 03                  mov         dx,0x03cf 
0029    EE                        out         dx,al 
002A    B0 08                     mov         al,0x08 
002C    BA CE 03                  mov         dx,0x03ce 
002F    EE                        out         dx,al 
0030    B0 FF                     mov         al,0xff 
0032    BA CF 03                  mov         dx,0x03cf 
0035    EE                        out         dx,al 
0036                          L$1:
0036    4B                        dec         bx 
0037    83 FB FF                  cmp         bx,0xffff 
003A    74 2E                     je          L$2 
003C    89 D9                     mov         cx,bx 
003E    B0 02                     mov         al,0x02 
0040    BA C4 03                  mov         dx,0x03c4 
0043    EE                        out         dx,al 
0044    B0 01                     mov         al,0x01 
0046    D2 E0                     shl         al,cl 
0048    BA C5 03                  mov         dx,0x03c5 
004B    EE                        out         dx,al 
004C    89 DE                     mov         si,bx 
004E    D1 E6                     shl         si,0x01 
0050    D1 E6                     shl         si,0x01 
0052    2E 8B 84 00 00            mov         ax,word ptr cs:_plane[si] 
0057    2E 8E 9C 02 00            mov         ds,word ptr cs:_plane+0x2[si] 
005C    2E C4 3E 00 00            les         di,dword ptr cs:_VGA 
0061    B9 00 4B                  mov         cx,0x4b00 
0064    89 C6                     mov         si,ax 
0066    F3 A5                     rep movsw   
0068    EB CC                     jmp         L$1 
006A                          L$2:
006A    07                        pop         es 
006B    1F                        pop         ds 
006C    5F                        pop         di 
006D    5E                        pop         si 
006E    5A                        pop         dx 
006F    59                        pop         cx 
0070    5B                        pop         bx 
0071    CB                        retf        

Routine Size: 94 bytes,    Routine Base: _TEXT + 0014

0072                          put_pixel_:
0072    52                        push        dx 
0073    56                        push        si 
0074    57                        push        di 
0075    1E                        push        ds 
0076    89 C7                     mov         di,ax 
0078    89 DE                     mov         si,bx 
007A    89 CB                     mov         bx,cx 
007C    89 F2                     mov         dx,si 
007E    D1 E2                     shl         dx,0x01 
0080    D1 E2                     shl         dx,0x01 
0082    01 F2                     add         dx,si 
0084    B1 04                     mov         cl,0x04 
0086    D3 E2                     shl         dx,cl 
0088    B1 03                     mov         cl,0x03 
008A    D3 E8                     shr         ax,cl 
008C    01 C2                     add         dx,ax 
008E    89 F9                     mov         cx,di 
0090    80 E1 07                  and         cl,0x07 
0093    B8 80 00                  mov         ax,0x0080 
0096    D3 F8                     sar         ax,cl 
0098    89 C1                     mov         cx,ax 
009A    81 FF 80 02               cmp         di,0x0280 
009E    73 52                     jae         L$7 
00A0    81 FE E0 01               cmp         si,0x01e0 
00A4    73 69                     jae         L$10 
00A6    F6 C3 08                  test        bl,0x08 
00A9    74 0D                     je          L$3 
00AB    2E C5 3E 0C 00            lds         di,dword ptr cs:_plane+0xc 
00B0    89 D6                     mov         si,dx 
00B2    01 FE                     add         si,di 
00B4    08 04                     or          byte ptr [si],al 
00B6    EB 0D                     jmp         L$4 
00B8                          L$3:
00B8    2E C5 3E 0C 00            lds         di,dword ptr cs:_plane+0xc 
00BD    89 D6                     mov         si,dx 
00BF    01 FE                     add         si,di 
00C1    F6 D1                     not         cl 
00C3    20 0C                     and         byte ptr [si],cl 
00C5                          L$4:
00C5    F6 C3 04                  test        bl,0x04 
00C8    74 0B                     je          L$5 
00CA    2E C5 36 08 00            lds         si,dword ptr cs:_plane+0x8 
00CF    01 D6                     add         si,dx 
00D1    08 04                     or          byte ptr [si],al 
00D3    EB 0D                     jmp         L$6 
00D5                          L$5:
00D5    2E C5 36 08 00            lds         si,dword ptr cs:_plane+0x8 
00DA    01 D6                     add         si,dx 
00DC    88 C1                     mov         cl,al 
00DE    F6 D1                     not         cl 
00E0    20 0C                     and         byte ptr [si],cl 
00E2                          L$6:
00E2    F6 C3 02                  test        bl,0x02 
00E5    74 0D                     je          L$8 
00E7    2E C5 36 04 00            lds         si,dword ptr cs:_plane+0x4 
00EC    01 D6                     add         si,dx 
00EE    08 04                     or          byte ptr [si],al 
00F0    EB 0F                     jmp         L$9 
00F2                          L$7:
00F2    EB 28                     jmp         L$12 
00F4                          L$8:
00F4    2E C5 36 04 00            lds         si,dword ptr cs:_plane+0x4 
00F9    01 D6                     add         si,dx 
00FB    88 C1                     mov         cl,al 
00FD    F6 D1                     not         cl 
00FF    20 0C                     and         byte ptr [si],cl 
0101                          L$9:
0101    F6 C3 01                  test        bl,0x01 
0104    74 0B                     je          L$11 
0106    2E C5 1E 00 00            lds         bx,dword ptr cs:_plane 
010B    01 D3                     add         bx,dx 
010D    08 07                     or          byte ptr [bx],al 
010F                          L$10:
010F    EB 0B                     jmp         L$12 
0111                          L$11:
0111    2E C5 1E 00 00            lds         bx,dword ptr cs:_plane 
0116    01 D3                     add         bx,dx 
0118    F6 D0                     not         al 
011A    20 07                     and         byte ptr [bx],al 
011C                          L$12:
011C    1F                        pop         ds 
011D    5F                        pop         di 
011E    5E                        pop         si 
011F    5A                        pop         dx 
0120    CB                        retf        

Routine Size: 175 bytes,    Routine Base: _TEXT + 0072

0121                          get_pixel_:
0121    51                        push        cx 
0122    52                        push        dx 
0123    56                        push        si 
0124    1E                        push        ds 
0125    55                        push        bp 
0126    89 E5                     mov         bp,sp 
0128    83 EC 02                  sub         sp,0x0002 
012B    89 C6                     mov         si,ax 
012D    89 D8                     mov         ax,bx 
012F    D1 E0                     shl         ax,0x01 
0131    D1 E0                     shl         ax,0x01 
0133    01 D8                     add         ax,bx 
0135    B1 04                     mov         cl,0x04 
0137    D3 E0                     shl         ax,cl 
0139    B1 03                     mov         cl,0x03 
013B    89 F2                     mov         dx,si 
013D    D3 EA                     shr         dx,cl 
013F    01 C2                     add         dx,ax 
0141    89 F1                     mov         cx,si 
0143    83 E1 07                  and         cx,0x0007 
0146    B8 80 00                  mov         ax,0x0080 
0149    D3 F8                     sar         ax,cl 
014B    89 46 FE                  mov         word ptr -0x2[bp],ax 
014E    8A 5E FE                  mov         bl,byte ptr -0x2[bp] 
0151    31 C0                     xor         ax,ax 
0153    2E C5 36 00 00            lds         si,dword ptr cs:_plane 
0158    01 D6                     add         si,dx 
015A    8A 0C                     mov         cl,byte ptr [si] 
015C    88 5E FE                  mov         byte ptr -0x2[bp],bl 
015F    88 6E FF                  mov         byte ptr -0x1[bp],ch 
0162    85 4E FE                  test        word ptr -0x2[bp],cx 
0165    74 03                     je          L$13 
0167    B8 01 00                  mov         ax,0x0001 
016A                          L$13:
016A    2E C5 36 04 00            lds         si,dword ptr cs:_plane+0x4 
016F    01 D6                     add         si,dx 
0171    8A 3C                     mov         bh,byte ptr [si] 
0173    88 7E FE                  mov         byte ptr -0x2[bp],bh 
0176    C6 46 FF 00               mov         byte ptr -0x1[bp],0x00 
017A    88 D9                     mov         cl,bl 
017C    30 ED                     xor         ch,ch 
017E    85 4E FE                  test        word ptr -0x2[bp],cx 
0181    74 02                     je          L$14 
0183    0C 02                     or          al,0x02 
0185                          L$14:
0185    2E C5 36 08 00            lds         si,dword ptr cs:_plane+0x8 
018A    01 D6                     add         si,dx 
018C    8A 0C                     mov         cl,byte ptr [si] 
018E    30 ED                     xor         ch,ch 
0190    88 5E FE                  mov         byte ptr -0x2[bp],bl 
0193    88 6E FF                  mov         byte ptr -0x1[bp],ch 
0196    85 4E FE                  test        word ptr -0x2[bp],cx 
0199    74 02                     je          L$15 
019B    0C 04                     or          al,0x04 
019D                          L$15:
019D    2E C5 36 0C 00            lds         si,dword ptr cs:_plane+0xc 
01A2    01 D6                     add         si,dx 
01A4    8A 14                     mov         dl,byte ptr [si] 
01A6    30 F6                     xor         dh,dh 
01A8    30 FF                     xor         bh,bh 
01AA    85 DA                     test        dx,bx 
01AC    74 02                     je          L$16 
01AE    0C 08                     or          al,0x08 
01B0                          L$16:
01B0    89 EC                     mov         sp,bp 
01B2    5D                        pop         bp 
01B3    1F                        pop         ds 
01B4    5E                        pop         si 
01B5    5A                        pop         dx 
01B6    59                        pop         cx 
01B7    CB                        retf        

Routine Size: 151 bytes,    Routine Base: _TEXT + 0121

01B8                          draw_rectangle_:
01B8    57                        push        di 
01B9    1E                        push        ds 
01BA    06                        push        es 
01BB    55                        push        bp 
01BC    89 E5                     mov         bp,sp 
01BE    83 EC 20                  sub         sp,0x0020 
01C1    50                        push        ax 
01C2    51                        push        cx 
01C3    52                        push        dx 
01C4    56                        push        si 
01C5    89 D6                     mov         si,dx 
01C7    01 C8                     add         ax,cx 
01C9    89 C2                     mov         dx,ax 
01CB    30 E6                     xor         dh,ah 
01CD    80 E2 07                  and         dl,0x07 
01D0    29 D0                     sub         ax,dx 
01D2    89 C2                     mov         dx,ax 
01D4    2B 56 DE                  sub         dx,word ptr -0x22[bp] 
01D7    B1 03                     mov         cl,0x03 
01D9    D3 EA                     shr         dx,cl 
01DB    8B 46 DE                  mov         ax,word ptr -0x22[bp] 
01DE    89 46 F8                  mov         word ptr -0x8[bp],ax 
01E1    F6 46 DE 07               test        byte ptr -0x22[bp],0x07 
01E5    74 43                     je          L$18 
01E7    89 C1                     mov         cx,ax 
01E9    80 E1 07                  and         cl,0x07 
01EC    B8 FF 00                  mov         ax,0x00ff 
01EF    D3 F8                     sar         ax,cl 
01F1                          L$17:
01F1    4E                        dec         si 
01F2    83 FE FF                  cmp         si,0xffff 
01F5    74 36                     je          L$19 
01F7    89 D9                     mov         cx,bx 
01F9    D1 E1                     shl         cx,0x01 
01FB    D1 E1                     shl         cx,0x01 
01FD    89 4E FA                  mov         word ptr -0x6[bp],cx 
0200    01 5E FA                  add         word ptr -0x6[bp],bx 
0203    B1 04                     mov         cl,0x04 
0205    D3 66 FA                  shl         word ptr -0x6[bp],cl 
0208    B1 03                     mov         cl,0x03 
020A    8B 7E DE                  mov         di,word ptr -0x22[bp] 
020D    D3 EF                     shr         di,cl 
020F    89 F9                     mov         cx,di 
0211    F6 46 D8 08               test        byte ptr -0x28[bp],0x08 
0215    74 19                     je          L$20 
0217    2E 8B 3E 0C 00            mov         di,word ptr cs:_plane+0xc 
021C    2E 8E 1E 0E 00            mov         ds,word ptr cs:_plane+0xe 
0221    03 7E FA                  add         di,word ptr -0x6[bp] 
0224    01 CF                     add         di,cx 
0226    08 05                     or          byte ptr [di],al 
0228    EB 1B                     jmp         L$21 
022A                          L$18:
022A    E9 13 01                  jmp         L$28 
022D                          L$19:
022D    E9 07 01                  jmp         L$27 
0230                          L$20:
0230    2E 8B 3E 0C 00            mov         di,word ptr cs:_plane+0xc 
0235    2E 8E 1E 0E 00            mov         ds,word ptr cs:_plane+0xe 
023A    03 7E FA                  add         di,word ptr -0x6[bp] 
023D    01 CF                     add         di,cx 
023F    88 C4                     mov         ah,al 
0241    F6 D4                     not         ah 
0243    20 25                     and         byte ptr [di],ah 
0245                          L$21:
0245    89 D9                     mov         cx,bx 
0247    D1 E1                     shl         cx,0x01 
0249    D1 E1                     shl         cx,0x01 
024B    89 4E FC                  mov         word ptr -0x4[bp],cx 
024E    01 5E FC                  add         word ptr -0x4[bp],bx 
0251    B1 04                     mov         cl,0x04 
0253    D3 66 FC                  shl         word ptr -0x4[bp],cl 
0256    B1 03                     mov         cl,0x03 
0258    8B 7E DE                  mov         di,word ptr -0x22[bp] 
025B    D3 EF                     shr         di,cl 
025D    89 F9                     mov         cx,di 
025F    F6 46 D8 04               test        byte ptr -0x28[bp],0x04 
0263    74 13                     je          L$22 
0265    2E 8B 3E 08 00            mov         di,word ptr cs:_plane+0x8 
026A    2E 8E 1E 0A 00            mov         ds,word ptr cs:_plane+0xa 
026F    03 7E FC                  add         di,word ptr -0x4[bp] 
0272    01 CF                     add         di,cx 
0274    08 05                     or          byte ptr [di],al 
0276    EB 15                     jmp         L$23 
0278                          L$22:
0278    2E 8B 3E 08 00            mov         di,word ptr cs:_plane+0x8 
027D    2E 8E 1E 0A 00            mov         ds,word ptr cs:_plane+0xa 
0282    03 7E FC                  add         di,word ptr -0x4[bp] 
0285    01 CF                     add         di,cx 
0287    88 C4                     mov         ah,al 
0289    F6 D4                     not         ah 
028B    20 25                     and         byte ptr [di],ah 
028D                          L$23:
028D    89 D9                     mov         cx,bx 
028F    D1 E1                     shl         cx,0x01 
0291    D1 E1                     shl         cx,0x01 
0293    89 4E FE                  mov         word ptr -0x2[bp],cx 
0296    01 5E FE                  add         word ptr -0x2[bp],bx 
0299    B1 04                     mov         cl,0x04 
029B    D3 66 FE                  shl         word ptr -0x2[bp],cl 
029E    B1 03                     mov         cl,0x03 
02A0    8B 7E DE                  mov         di,word ptr -0x22[bp] 
02A3    D3 EF                     shr         di,cl 
02A5    89 F9                     mov         cx,di 
02A7    F6 46 D8 02               test        byte ptr -0x28[bp],0x02 
02AB    74 13                     je          L$24 
02AD    2E 8B 3E 04 00            mov         di,word ptr cs:_plane+0x4 
02B2    2E 8E 1E 06 00            mov         ds,word ptr cs:_plane+0x6 
02B7    03 7E FE                  add         di,word ptr -0x2[bp] 
02BA    01 CF                     add         di,cx 
02BC    08 05                     or          byte ptr [di],al 
02BE    EB 15                     jmp         L$25 
02C0                          L$24:
02C0    2E 8B 3E 04 00            mov         di,word ptr cs:_plane+0x4 
02C5    2E 8E 1E 06 00            mov         ds,word ptr cs:_plane+0x6 
02CA    03 7E FE                  add         di,word ptr -0x2[bp] 
02CD    01 CF                     add         di,cx 
02CF    88 C4                     mov         ah,al 
02D1    F6 D4                     not         ah 
02D3    20 25                     and         byte ptr [di],ah 
02D5                          L$25:
02D5    8D 4F 01                  lea         cx,0x1[bx] 
02D8    89 4E F0                  mov         word ptr -0x10[bp],cx 
02DB    89 D9                     mov         cx,bx 
02DD    D1 E1                     shl         cx,0x01 
02DF    D1 E1                     shl         cx,0x01 
02E1    89 4E E2                  mov         word ptr -0x1e[bp],cx 
02E4    01 5E E2                  add         word ptr -0x1e[bp],bx 
02E7    B1 04                     mov         cl,0x04 
02E9    D3 66 E2                  shl         word ptr -0x1e[bp],cl 
02EC    B1 03                     mov         cl,0x03 
02EE    8B 7E DE                  mov         di,word ptr -0x22[bp] 
02F1    D3 EF                     shr         di,cl 
02F3    F6 46 D8 01               test        byte ptr -0x28[bp],0x01 
02F7    74 1D                     je          L$26 
02F9    2E 8B 1E 00 00            mov         bx,word ptr cs:_plane 
02FE    89 5E E0                  mov         word ptr -0x20[bp],bx 
0301    2E 8E 1E 02 00            mov         ds,word ptr cs:_plane+0x2 
0306    8B 5E F0                  mov         bx,word ptr -0x10[bp] 
0309    8B 4E E0                  mov         cx,word ptr -0x20[bp] 
030C    03 4E E2                  add         cx,word ptr -0x1e[bp] 
030F    01 CF                     add         di,cx 
0311    08 05                     or          byte ptr [di],al 
0313    E9 DB FE                  jmp         L$17 
0316                          L$26:
0316    2E 8B 1E 00 00            mov         bx,word ptr cs:_plane 
031B    89 5E E0                  mov         word ptr -0x20[bp],bx 
031E    2E 8E 1E 02 00            mov         ds,word ptr cs:_plane+0x2 
0323    8B 5E F0                  mov         bx,word ptr -0x10[bp] 
0326    8B 4E E0                  mov         cx,word ptr -0x20[bp] 
0329    03 4E E2                  add         cx,word ptr -0x1e[bp] 
032C    01 CF                     add         di,cx 
032E    88 C4                     mov         ah,al 
0330    F6 D4                     not         ah 
0332    20 25                     and         byte ptr [di],ah 
0334    E9 BA FE                  jmp         L$17 
0337                          L$27:
0337    8B 76 DA                  mov         si,word ptr -0x26[bp] 
033A    29 F3                     sub         bx,si 
033C    83 46 F8 08               add         word ptr -0x8[bp],0x0008 
0340                          L$28:
0340    4E                        dec         si 
0341    83 FE FF                  cmp         si,0xffff 
0344    75 03                     jne         L$29 
0346    E9 16 01                  jmp         L$37 
0349                          L$29:
0349    89 D8                     mov         ax,bx 
034B    D1 E0                     shl         ax,0x01 
034D    D1 E0                     shl         ax,0x01 
034F    01 D8                     add         ax,bx 
0351    B1 04                     mov         cl,0x04 
0353    D3 E0                     shl         ax,cl 
0355    2E 8B 0E 00 00            mov         cx,word ptr cs:_plane 
035A    2E 8B 3E 02 00            mov         di,word ptr cs:_plane+0x2 
035F    89 7E F6                  mov         word ptr -0xa[bp],di 
0362    01 C1                     add         cx,ax 
0364    89 4E E0                  mov         word ptr -0x20[bp],cx 
0367    B1 03                     mov         cl,0x03 
0369    8B 7E F8                  mov         di,word ptr -0x8[bp] 
036C    D3 FF                     sar         di,cl 
036E    89 F9                     mov         cx,di 
0370    8B 7E E0                  mov         di,word ptr -0x20[bp] 
0373    01 CF                     add         di,cx 
0375    89 7E E6                  mov         word ptr -0x1a[bp],di 
0378    2E 8B 3E 04 00            mov         di,word ptr cs:_plane+0x4 
037D    89 7E E4                  mov         word ptr -0x1c[bp],di 
0380    2E 8B 3E 06 00            mov         di,word ptr cs:_plane+0x6 
0385    89 7E E0                  mov         word ptr -0x20[bp],di 
0388    8B 7E E4                  mov         di,word ptr -0x1c[bp] 
038B    01 C7                     add         di,ax 
038D    89 7E E4                  mov         word ptr -0x1c[bp],di 
0390    8B 7E E0                  mov         di,word ptr -0x20[bp] 
0393    89 7E EA                  mov         word ptr -0x16[bp],di 
0396    8B 7E E4                  mov         di,word ptr -0x1c[bp] 
0399    01 CF                     add         di,cx 
039B    89 7E E8                  mov         word ptr -0x18[bp],di 
039E    2E 8B 3E 08 00            mov         di,word ptr cs:_plane+0x8 
03A3    89 7E E0                  mov         word ptr -0x20[bp],di 
03A6    2E 8B 3E 0A 00            mov         di,word ptr cs:_plane+0xa 
03AB    89 7E E4                  mov         word ptr -0x1c[bp],di 
03AE    8B 7E E0                  mov         di,word ptr -0x20[bp] 
03B1    01 C7                     add         di,ax 
03B3    89 7E E0                  mov         word ptr -0x20[bp],di 
03B6    8B 7E E4                  mov         di,word ptr -0x1c[bp] 
03B9    89 7E F2                  mov         word ptr -0xe[bp],di 
03BC    8B 7E E0                  mov         di,word ptr -0x20[bp] 
03BF    01 CF                     add         di,cx 
03C1    89 7E EC                  mov         word ptr -0x14[bp],di 
03C4    2E 8B 3E 0C 00            mov         di,word ptr cs:_plane+0xc 
03C9    89 7E E0                  mov         word ptr -0x20[bp],di 
03CC    2E 8B 3E 0E 00            mov         di,word ptr cs:_plane+0xe 
03D1    43                        inc         bx 
03D2    03 46 E0                  add         ax,word ptr -0x20[bp] 
03D5    89 7E EE                  mov         word ptr -0x12[bp],di 
03D8    01 C8                     add         ax,cx 
03DA    89 46 F4                  mov         word ptr -0xc[bp],ax 
03DD    F6 46 D8 01               test        byte ptr -0x28[bp],0x01 
03E1    74 0F                     je          L$30 
03E3    89 D1                     mov         cx,dx 
03E5    B8 FF 00                  mov         ax,0x00ff 
03E8    8B 7E E6                  mov         di,word ptr -0x1a[bp] 
03EB    8E 46 F6                  mov         es,word ptr -0xa[bp] 
03EE    F3 AA                     rep stosb   
03F0    EB 0C                     jmp         L$31 
03F2                          L$30:
03F2    89 D1                     mov         cx,dx 
03F4    31 C0                     xor         ax,ax 
03F6    8B 7E E6                  mov         di,word ptr -0x1a[bp] 
03F9    8E 46 F6                  mov         es,word ptr -0xa[bp] 
03FC    F3 AA                     rep stosb   
03FE                          L$31:
03FE    F6 46 D8 02               test        byte ptr -0x28[bp],0x02 
0402    74 0C                     je          L$32 
0404    89 D1                     mov         cx,dx 
0406    B8 FF 00                  mov         ax,0x00ff 
0409    C4 7E E8                  les         di,dword ptr -0x18[bp] 
040C    F3 AA                     rep stosb   
040E    EB 09                     jmp         L$33 
0410                          L$32:
0410    89 D1                     mov         cx,dx 
0412    31 C0                     xor         ax,ax 
0414    C4 7E E8                  les         di,dword ptr -0x18[bp] 
0417    F3 AA                     rep stosb   
0419                          L$33:
0419    F6 46 D8 04               test        byte ptr -0x28[bp],0x04 
041D    74 0F                     je          L$34 
041F    89 D1                     mov         cx,dx 
0421    B8 FF 00                  mov         ax,0x00ff 
0424    8B 7E EC                  mov         di,word ptr -0x14[bp] 
0427    8E 46 F2                  mov         es,word ptr -0xe[bp] 
042A    F3 AA                     rep stosb   
042C    EB 0C                     jmp         L$35 
042E                          L$34:
042E    89 D1                     mov         cx,dx 
0430    31 C0                     xor         ax,ax 
0432    8B 7E EC                  mov         di,word ptr -0x14[bp] 
0435    8E 46 F2                  mov         es,word ptr -0xe[bp] 
0438    F3 AA                     rep stosb   
043A                          L$35:
043A    F6 46 D8 08               test        byte ptr -0x28[bp],0x08 
043E    74 10                     je          L$36 
0440    89 D1                     mov         cx,dx 
0442    B8 FF 00                  mov         ax,0x00ff 
0445    8B 7E F4                  mov         di,word ptr -0xc[bp] 
0448    8E 46 EE                  mov         es,word ptr -0x12[bp] 
044B    F3 AA                     rep stosb   
044D    E9 F0 FE                  jmp         L$28 
0450                          L$36:
0450    89 D1                     mov         cx,dx 
0452    31 C0                     xor         ax,ax 
0454    8B 7E F4                  mov         di,word ptr -0xc[bp] 
0457    8E 46 EE                  mov         es,word ptr -0x12[bp] 
045A    F3 AA                     rep stosb   
045C    E9 E1 FE                  jmp         L$28 
045F                          L$37:
045F    8B 56 DE                  mov         dx,word ptr -0x22[bp] 
0462    03 56 DC                  add         dx,word ptr -0x24[bp] 
0465    F6 C2 07                  test        dl,0x07 
0468    74 4C                     je          L$39 
046A    89 D1                     mov         cx,dx 
046C    80 E1 07                  and         cl,0x07 
046F    B8 FF 00                  mov         ax,0x00ff 
0472    D3 F8                     sar         ax,cl 
0474    F7 D0                     not         ax 
0476    8B 76 DA                  mov         si,word ptr -0x26[bp] 
0479    4B                        dec         bx 
047A    89 56 F8                  mov         word ptr -0x8[bp],dx 
047D                          L$38:
047D    4E                        dec         si 
047E    83 FE FF                  cmp         si,0xffff 
0481    74 33                     je          L$39 
0483    89 DA                     mov         dx,bx 
0485    D1 E2                     shl         dx,0x01 
0487    D1 E2                     shl         dx,0x01 
0489    01 DA                     add         dx,bx 
048B    B1 04                     mov         cl,0x04 
048D    D3 E2                     shl         dx,cl 
048F    B1 03                     mov         cl,0x03 
0491    8B 7E F8                  mov         di,word ptr -0x8[bp] 
0494    D3 FF                     sar         di,cl 
0496    89 F9                     mov         cx,di 
0498    F6 46 D8 08               test        byte ptr -0x28[bp],0x08 
049C    74 1B                     je          L$40 
049E    2E 8B 3E 0C 00            mov         di,word ptr cs:_plane+0xc 
04A3    89 7E E0                  mov         word ptr -0x20[bp],di 
04A6    2E 8E 1E 0E 00            mov         ds,word ptr cs:_plane+0xe 
04AB    89 D7                     mov         di,dx 
04AD    03 7E E0                  add         di,word ptr -0x20[bp] 
04B0    01 CF                     add         di,cx 
04B2    08 05                     or          byte ptr [di],al 
04B4    EB 1D                     jmp         L$41 
04B6                          L$39:
04B6    E9 0C 01                  jmp         L$47 
04B9                          L$40:
04B9    2E 8B 3E 0C 00            mov         di,word ptr cs:_plane+0xc 
04BE    89 7E E4                  mov         word ptr -0x1c[bp],di 
04C1    2E 8E 1E 0E 00            mov         ds,word ptr cs:_plane+0xe 
04C6    89 D7                     mov         di,dx 
04C8    03 7E E4                  add         di,word ptr -0x1c[bp] 
04CB    01 CF                     add         di,cx 
04CD    88 C4                     mov         ah,al 
04CF    F6 D4                     not         ah 
04D1    20 25                     and         byte ptr [di],ah 
04D3                          L$41:
04D3    89 DA                     mov         dx,bx 
04D5    D1 E2                     shl         dx,0x01 
04D7    D1 E2                     shl         dx,0x01 
04D9    01 DA                     add         dx,bx 
04DB    B1 04                     mov         cl,0x04 
04DD    D3 E2                     shl         dx,cl 
04DF    B1 03                     mov         cl,0x03 
04E1    8B 7E F8                  mov         di,word ptr -0x8[bp] 
04E4    D3 FF                     sar         di,cl 
04E6    89 F9                     mov         cx,di 
04E8    F6 46 D8 04               test        byte ptr -0x28[bp],0x04 
04EC    74 18                     je          L$42 
04EE    2E 8B 3E 08 00            mov         di,word ptr cs:_plane+0x8 
04F3    89 7E E4                  mov         word ptr -0x1c[bp],di 
04F6    2E 8E 1E 0A 00            mov         ds,word ptr cs:_plane+0xa 
04FB    89 D7                     mov         di,dx 
04FD    03 7E E4                  add         di,word ptr -0x1c[bp] 
0500    01 CF                     add         di,cx 
0502    08 05                     or          byte ptr [di],al 
0504    EB 1A                     jmp         L$43 
0506                          L$42:
0506    2E 8B 3E 08 00            mov         di,word ptr cs:_plane+0x8 
050B    89 7E E4                  mov         word ptr -0x1c[bp],di 
050E    2E 8E 1E 0A 00            mov         ds,word ptr cs:_plane+0xa 
0513    89 D7                     mov         di,dx 
0515    03 7E E4                  add         di,word ptr -0x1c[bp] 
0518    01 CF                     add         di,cx 
051A    88 C4                     mov         ah,al 
051C    F6 D4                     not         ah 
051E    20 25                     and         byte ptr [di],ah 
0520                          L$43:
0520    89 DA                     mov         dx,bx 
0522    D1 E2                     shl         dx,0x01 
0524    D1 E2                     shl         dx,0x01 
0526    01 DA                     add         dx,bx 
0528    B1 04                     mov         cl,0x04 
052A    D3 E2                     shl         dx,cl 
052C    B1 03                     mov         cl,0x03 
052E    8B 7E F8                  mov         di,word ptr -0x8[bp] 
0531    D3 FF                     sar         di,cl 
0533    89 F9                     mov         cx,di 
0535    F6 46 D8 02               test        byte ptr -0x28[bp],0x02 
0539    74 18                     je          L$44 
053B    2E 8B 3E 04 00            mov         di,word ptr cs:_plane+0x4 
0540    89 7E E4                  mov         word ptr -0x1c[bp],di 
0543    2E 8E 1E 06 00            mov         ds,word ptr cs:_plane+0x6 
0548    89 D7                     mov         di,dx 
054A    03 7E E4                  add         di,word ptr -0x1c[bp] 
054D    01 CF                     add         di,cx 
054F    08 05                     or          byte ptr [di],al 
0551    EB 1A                     jmp         L$45 
0553                          L$44:
0553    2E 8B 3E 04 00            mov         di,word ptr cs:_plane+0x4 
0558    89 7E E4                  mov         word ptr -0x1c[bp],di 
055B    2E 8E 1E 06 00            mov         ds,word ptr cs:_plane+0x6 
0560    89 D7                     mov         di,dx 
0562    03 7E E4                  add         di,word ptr -0x1c[bp] 
0565    01 CF                     add         di,cx 
0567    88 C4                     mov         ah,al 
0569    F6 D4                     not         ah 
056B    20 25                     and         byte ptr [di],ah 
056D                          L$45:
056D    8D 7F FF                  lea         di,-0x1[bx] 
0570    89 DA                     mov         dx,bx 
0572    D1 E2                     shl         dx,0x01 
0574    D1 E2                     shl         dx,0x01 
0576    01 DA                     add         dx,bx 
0578    B1 04                     mov         cl,0x04 
057A    D3 E2                     shl         dx,cl 
057C    B1 03                     mov         cl,0x03 
057E    8B 5E F8                  mov         bx,word ptr -0x8[bp] 
0581    D3 FB                     sar         bx,cl 
0583    89 D9                     mov         cx,bx 
0585    F6 46 D8 01               test        byte ptr -0x28[bp],0x01 
0589    74 1B                     je          L$46 
058B    2E 8B 1E 00 00            mov         bx,word ptr cs:_plane 
0590    89 5E E4                  mov         word ptr -0x1c[bp],bx 
0593    2E 8E 1E 02 00            mov         ds,word ptr cs:_plane+0x2 
0598    89 FB                     mov         bx,di 
059A    89 D7                     mov         di,dx 
059C    03 7E E4                  add         di,word ptr -0x1c[bp] 
059F    01 CF                     add         di,cx 
05A1    08 05                     or          byte ptr [di],al 
05A3    E9 D7 FE                  jmp         L$38 
05A6                          L$46:
05A6    2E 8B 1E 00 00            mov         bx,word ptr cs:_plane 
05AB    89 5E E4                  mov         word ptr -0x1c[bp],bx 
05AE    2E 8E 1E 02 00            mov         ds,word ptr cs:_plane+0x2 
05B3    89 FB                     mov         bx,di 
05B5    89 D7                     mov         di,dx 
05B7    03 7E E4                  add         di,word ptr -0x1c[bp] 
05BA    01 CF                     add         di,cx 
05BC    88 C4                     mov         ah,al 
05BE    F6 D4                     not         ah 
05C0    20 25                     and         byte ptr [di],ah 
05C2    E9 B8 FE                  jmp         L$38 
05C5                          L$47:
05C5    89 EC                     mov         sp,bp 
05C7    5D                        pop         bp 
05C8    07                        pop         es 
05C9    1F                        pop         ds 
05CA    5F                        pop         di 
05CB    CB                        retf        

Routine Size: 1044 bytes,    Routine Base: _TEXT + 01B8

05CC                          fill_screen_:
05CC    51                        push        cx 
05CD    52                        push        dx 
05CE    57                        push        di 
05CF    06                        push        es 
05D0    89 C2                     mov         dx,ax 
05D2    A8 08                     test        al,0x08 
05D4    74 0F                     je          L$48 
05D6    2E C4 3E 0C 00            les         di,dword ptr cs:_plane+0xc 
05DB    B9 00 4B                  mov         cx,0x4b00 
05DE    B8 FF FF                  mov         ax,0xffff 
05E1    F3 AB                     rep stosw   
05E3    EB 0C                     jmp         L$49 
05E5                          L$48:
05E5    2E C4 3E 0C 00            les         di,dword ptr cs:_plane+0xc 
05EA    B9 00 4B                  mov         cx,0x4b00 
05ED    31 C0                     xor         ax,ax 
05EF    F3 AB                     rep stosw   
05F1                          L$49:
05F1    F6 C2 04                  test        dl,0x04 
05F4    74 0F                     je          L$50 
05F6    2E C4 3E 08 00            les         di,dword ptr cs:_plane+0x8 
05FB    B9 00 4B                  mov         cx,0x4b00 
05FE    B8 FF FF                  mov         ax,0xffff 
0601    F3 AB                     rep stosw   
0603    EB 0C                     jmp         L$51 
0605                          L$50:
0605    2E C4 3E 08 00            les         di,dword ptr cs:_plane+0x8 
060A    B9 00 4B                  mov         cx,0x4b00 
060D    31 C0                     xor         ax,ax 
060F    F3 AB                     rep stosw   
0611                          L$51:
0611    F6 C2 02                  test        dl,0x02 
0614    74 0F                     je          L$52 
0616    2E C4 3E 04 00            les         di,dword ptr cs:_plane+0x4 
061B    B9 00 4B                  mov         cx,0x4b00 
061E    B8 FF FF                  mov         ax,0xffff 
0621    F3 AB                     rep stosw   
0623    EB 0C                     jmp         L$53 
0625                          L$52:
0625    2E C4 3E 04 00            les         di,dword ptr cs:_plane+0x4 
062A    B9 00 4B                  mov         cx,0x4b00 
062D    31 C0                     xor         ax,ax 
062F    F3 AB                     rep stosw   
0631                          L$53:
0631    F6 C2 01                  test        dl,0x01 
0634    74 0F                     je          L$54 
0636    2E C4 3E 00 00            les         di,dword ptr cs:_plane 
063B    B9 00 4B                  mov         cx,0x4b00 
063E    B8 FF FF                  mov         ax,0xffff 
0641    F3 AB                     rep stosw   
0643    EB 0C                     jmp         L$55 
0645                          L$54:
0645    2E C4 3E 00 00            les         di,dword ptr cs:_plane 
064A    B9 00 4B                  mov         cx,0x4b00 
064D    31 C0                     xor         ax,ax 
064F    F3 AB                     rep stosw   
0651                          L$55:
0651    07                        pop         es 
0652    5F                        pop         di 
0653    5A                        pop         dx 
0654    59                        pop         cx 
0655    CB                        retf        

Routine Size: 138 bytes,    Routine Base: _TEXT + 05CC

0656                          main_:
0656    53                        push        bx 
0657    51                        push        cx 
0658    56                        push        si 
0659    57                        push        di 
065A    1E                        push        ds 
065B    06                        push        es 
065C    55                        push        bp 
065D    89 E5                     mov         bp,sp 
065F    83 EC 44                  sub         sp,0x0044 
0662    89 C3                     mov         bx,ax 
0664    89 56 FC                  mov         word ptr -0x4[bp],dx 
0667    8C D0                     mov         ax,ss 
0669    8E C0                     mov         es,ax 
066B    8D 7E DC                  lea         di,-0x24[bp] 
066E    8E D8                     mov         ds,ax 
0670    BE 00 00                  mov         si,offset DGROUP:L$68 
0673    A5                        movsw       
0674    A5                        movsw       
0675    A5                        movsw       
0676    A5                        movsw       
0677    A5                        movsw       
0678    A5                        movsw       
0679    A5                        movsw       
067A    A5                        movsw       
067B    B9 10 00                  mov         cx,0x0010 
067E    8D 7E BC                  lea         di,-0x44[bp] 
0681    BE 10 00                  mov         si,offset DGROUP:L$69 
0684    F3 A5                     rep movsw   
0686    83 FB 02                  cmp         bx,0x0002 
0689    74 03                     je          L$56 
068B    E9 3E 01                  jmp         L$66 
068E                          L$56:
068E    BA 01 00                  mov         dx,0x0001 
0691    B8 00 96                  mov         ax,0x9600 
0694    E8 00 00                  call        _fcalloc_ 
0697    89 C7                     mov         di,ax 
0699    89 D6                     mov         si,dx 
069B    2E A3 00 00               mov         word ptr cs:_plane,ax 
069F    2E 89 16 02 00            mov         word ptr cs:_plane+0x2,dx 
06A4    BA 01 00                  mov         dx,0x0001 
06A7    B8 00 96                  mov         ax,0x9600 
06AA    E8 00 00                  call        _fcalloc_ 
06AD    89 46 FE                  mov         word ptr -0x2[bp],ax 
06B0    89 D3                     mov         bx,dx 
06B2    2E A3 04 00               mov         word ptr cs:_plane+0x4,ax 
06B6    2E 89 16 06 00            mov         word ptr cs:_plane+0x6,dx 
06BB    BA 01 00                  mov         dx,0x0001 
06BE    B8 00 96                  mov         ax,0x9600 
06C1    E8 00 00                  call        _fcalloc_ 
06C4    89 46 FA                  mov         word ptr -0x6[bp],ax 
06C7    89 D1                     mov         cx,dx 
06C9    2E A3 08 00               mov         word ptr cs:_plane+0x8,ax 
06CD    2E 89 16 0A 00            mov         word ptr cs:_plane+0xa,dx 
06D2    BA 01 00                  mov         dx,0x0001 
06D5    B8 00 96                  mov         ax,0x9600 
06D8    E8 00 00                  call        _fcalloc_ 
06DB    2E A3 0C 00               mov         word ptr cs:_plane+0xc,ax 
06DF    2E 89 16 0E 00            mov         word ptr cs:_plane+0xe,dx 
06E4    85 F6                     test        si,si 
06E6    75 04                     jne         L$57 
06E8    85 FF                     test        di,di 
06EA    74 1C                     je          L$60 
06EC                          L$57:
06EC    85 DB                     test        bx,bx 
06EE    75 06                     jne         L$58 
06F0    83 7E FE 00               cmp         word ptr -0x2[bp],0x0000 
06F4    74 12                     je          L$60 
06F6                          L$58:
06F6    85 C9                     test        cx,cx 
06F8    75 06                     jne         L$59 
06FA    83 7E FA 00               cmp         word ptr -0x6[bp],0x0000 
06FE    74 08                     je          L$60 
0700                          L$59:
0700    85 D2                     test        dx,dx 
0702    75 37                     jne         L$61 
0704    85 C0                     test        ax,ax 
0706    75 33                     jne         L$61 
0708                          L$60:
0708    2E A1 00 00               mov         ax,word ptr cs:_plane 
070C    2E 8B 16 02 00            mov         dx,word ptr cs:_plane+0x2 
0711    E8 00 00                  call        _ffree_ 
0714    2E A1 04 00               mov         ax,word ptr cs:_plane+0x4 
0718    2E 8B 16 06 00            mov         dx,word ptr cs:_plane+0x6 
071D    E8 00 00                  call        _ffree_ 
0720    2E A1 08 00               mov         ax,word ptr cs:_plane+0x8 
0724    2E 8B 16 0A 00            mov         dx,word ptr cs:_plane+0xa 
0729    E8 00 00                  call        _ffree_ 
072C    2E A1 0C 00               mov         ax,word ptr cs:_plane+0xc 
0730    2E 8B 16 0E 00            mov         dx,word ptr cs:_plane+0xe 
0735    E8 00 00                  call        _ffree_ 
0738    E9 91 00                  jmp         L$66 
073B                          L$61:
073B    8C 56 EC                  mov         word ptr -0x14[bp],ss 
073E    8D 46 DC                  lea         ax,-0x24[bp] 
0741    89 46 EE                  mov         word ptr -0x12[bp],ax 
0744    C7 46 F0 01 00            mov         word ptr -0x10[bp],0x0001 
0749    8C 56 F2                  mov         word ptr -0xe[bp],ss 
074C    8D 46 BC                  lea         ax,-0x44[bp] 
074F    89 46 F4                  mov         word ptr -0xc[bp],ax 
0752    8B 5E FC                  mov         bx,word ptr -0x4[bp] 
0755    8B 47 02                  mov         ax,word ptr 0x2[bx] 
0758    BB 10 00                  mov         bx,0x0010 
075B    31 D2                     xor         dx,dx 
075D    E8 00 00                  call        strtoul_ 
0760    30 E4                     xor         ah,ah 
0762    8D 5E EC                  lea         bx,-0x14[bp] 
0765    8D 56 EC                  lea         dx,-0x14[bp] 
0768    E8 00 00                  call        int86_ 
076B    83 7E EC 00               cmp         word ptr -0x14[bp],0x0000 
076F    74 34                     je          L$64 
0771    B8 00 00                  mov         ax,offset DGROUP:L$67 
0774    E8 00 00                  call        puts_ 
0777    36 83 3E 02 00 00         cmp         word ptr ss:___iob+0x2,0x0000 
077D    7E 11                     jle         L$62 
077F    36 8B 1E 00 00            mov         bx,word ptr ss:___iob 
0784    8A 07                     mov         al,byte ptr [bx] 
0786    30 E4                     xor         ah,ah 
0788    2D 0D 00                  sub         ax,0x000d 
078B    3D 0D 00                  cmp         ax,0x000d 
078E    77 08                     ja          L$63 
0790                          L$62:
0790    B8 00 00                  mov         ax,offset ___iob 
0793    E8 00 00                  call        fgetc_ 
0796    EB 34                     jmp         L$66 
0798                          L$63:
0798    36 FF 0E 02 00            dec         word ptr ss:___iob+0x2 
079D    43                        inc         bx 
079E    36 89 1E 00 00            mov         word ptr ss:___iob,bx 
07A3    EB 27                     jmp         L$66 
07A5                          L$64:
07A5    9A 00 00 00 00            call        set_mode_ 
07AA    B1 04                     mov         cl,0x04 
07AC    B8 0E 00                  mov         ax,offset main_end_+0xe 
07AF    D3 E8                     shr         ax,cl 
07B1    3D 12 00                  cmp         ax,0x0012 
07B4    73 09                     jae         L$65 
07B6    BA 12 00                  mov         dx,0x0012 
07B9    30 C0                     xor         al,al 
07BB    B4 31                     mov         ah,0x31 
07BD    CD 21                     int         0x21 
07BF                          L$65:
07BF    B1 04                     mov         cl,0x04 
07C1    BA 0E 00                  mov         dx,offset main_end_+0xe 
07C4    D3 EA                     shr         dx,cl 
07C6    30 C0                     xor         al,al 
07C8    B4 31                     mov         ah,0x31 
07CA    CD 21                     int         0x21 
07CC                          L$66:
07CC    B8 01 00                  mov         ax,0x0001 
07CF    89 EC                     mov         sp,bp 
07D1    5D                        pop         bp 
07D2    07                        pop         es 
07D3    1F                        pop         ds 
07D4    5F                        pop         di 
07D5    5E                        pop         si 
07D6    59                        pop         cx 
07D7    5B                        pop         bx 

Routine Size: 386 bytes,    Routine Base: _TEXT + 0656

07D8                          main_end_:
07D8    C3                        ret         

Routine Size: 1 bytes,    Routine Base: _TEXT + 07D8

No disassembly errors

Segment: CONST WORD USE16 0000003D bytes
0000                          L$67:
0000    45 72 72 6F 72 20 77 68 69 6C 65 20 6C 6F 61 64 Error while load
0010    69 6E 67 20 44 4F 55 42 4C 45 42 55 46 46 45 52 ing DOUBLEBUFFER
0020    45 44 20 56 47 41 20 6D 6F 64 75 6C 65 2E 20 50 ED VGA module. P
0030    72 65 73 73 20 61 6E 79 20 6B 65 79 00          ress any key.

Segment: CONST2 WORD USE16 00000000 bytes

Segment: _DATA WORD USE16 00000030 bytes
0000                          L$68:
0000    00 00 00 00 FF FF FF FF 00 00 00 00 FF FF FF FF ................
0010                          L$69:
0010    00 00 00 00                                     DD	put_pixel_
0014    00 00 00 00                                     DD	get_pixel_
0018    00 00 00 00                                     DD	draw_rectangle_
001C    00 00 00 00                                     DD	fill_screen_
0020    00 00 00 00                                     DD	update_screen_
0024    00 00 00 00                                     DD	set_mode_
0028    80 02 E0 01 08 00 FF 00                         ........

