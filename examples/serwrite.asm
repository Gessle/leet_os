.387
		PUBLIC	serial_write_
		EXTRN	__PRO:BYTE
		EXTRN	_serialbase:BYTE
		EXTRN	inp_:BYTE
		EXTRN	_serport_buffer:BYTE
		EXTRN	delay_:BYTE
		EXTRN	outp_:BYTE
		EXTRN	__EPI:BYTE
		EXTRN	_big_code_:BYTE
DGROUP		GROUP	CONST,CONST2,_DATA
serwrite_TEXT		SEGMENT	BYTE PUBLIC USE16 'CODE'
		ASSUME CS:serwrite_TEXT, DS:DGROUP, SS:DGROUP
    DB	73H, 65H, 72H, 69H, 61H, 6cH, 5fH, 77H
    DB	72H, 69H, 74H, 65H, 0cH
serial_write_:
    inc         bp 
    push        bp 
    mov         bp,sp 
    call        far ptr __PRO 
    push        bx 
    push        cx 
    push        si 
    push        di 
    push        ax 
    mov         cl,al 
    mov         byte ptr -0aH[bp],dl 
    xor         ch,ch 
    xor         si,si 
    mov         dx,DGROUP:CONST 
L$1:
    mov         al,cl 
    xor         ah,ah 
    shl         ax,1 
    mov         ds,dx 
    lds         bx,dword ptr _serialbase 
    add         bx,ax 
    mov         ax,word ptr [bx] 
    add         ax,5 
    call        far ptr inp_ 
    test        al,20H 
    je          L$1 
L$2:
    inc         si 
    je          L$5 
    mov         dl,cl 
    mov         bl,cl 
    xor         bh,bh 
    shl         bx,1 
    shl         bx,1 
    mov         ax,seg _serport_buffer 
    mov         ds,ax 
    lds         di,dword ptr _serport_buffer[bx] 
    mov         ax,word ptr 104H[di] 
    mov         bx,ax 
    and         bl,3 
    test        bl,1 
    je          L$3 
    test        al,8 
    jne         L$4 
L$3:
    xor         dh,dh 
    mov         bx,dx 
    shl         bx,1 
    shl         bx,1 
    mov         ax,seg _serport_buffer 
    mov         ds,ax 
    lds         di,dword ptr _serport_buffer[bx] 
    mov         ax,word ptr 104H[di] 
    and         al,3 
    test        al,2 
    je          L$5 
    shl         dx,1 
    mov         ax,DGROUP:CONST 
    mov         ds,ax 
    lds         bx,dword ptr _serialbase 
    add         bx,dx 
    mov         ax,word ptr [bx] 
    add         ax,6 
    call        far ptr inp_ 
    test        al,10H 
    jne         L$5 
L$4:
    cmp         si,0fff0H 
    jbe         L$2 
    mov         ax,1 
    call        far ptr delay_ 
    jmp         L$2 
L$5:
    inc         ch 
    cmp         ch,1 
    je          L$6 
    mov         ax,1 
    call        far ptr delay_ 
L$6:
    test        si,si 
    je          L$7 
    cmp         ch,0ffH 
    jne         L$8 
L$7:
    mov         al,1 
    jmp         L$9 
L$8:
    mov         dl,byte ptr -0aH[bp] 
    xor         dh,dh 
    mov         al,cl 
    xor         ah,ah 
    mov         di,ax 
    shl         di,1 
    mov         ax,DGROUP:CONST 
    mov         ds,ax 
    lds         bx,dword ptr _serialbase 
    add         bx,di 
    mov         ax,word ptr [bx] 
    call        far ptr outp_ 
    mov         ax,DGROUP:CONST 
    mov         ds,ax 
    lds         bx,dword ptr _serialbase 
    add         bx,di 
    mov         ax,word ptr [bx] 
    add         ax,5 
    call        far ptr inp_ 
    test        al,2 
    jne         L$5 
    xor         al,al 
L$9:
    lea         sp,-8[bp] 
    pop         di 
    pop         si 
    pop         cx 
    pop         bx 
    call        far ptr __EPI 
    pop         bp 
    dec         bp 
    retf        
serwrite_TEXT		ENDS
CONST		SEGMENT	WORD PUBLIC USE16 'DATA'
CONST		ENDS
CONST2		SEGMENT	WORD PUBLIC USE16 'DATA'
CONST2		ENDS
_DATA		SEGMENT	WORD PUBLIC USE16 'DATA'
_DATA		ENDS
		END
