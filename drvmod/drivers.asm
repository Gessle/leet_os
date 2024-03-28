.387
		PUBLIC	driver_id_
		PUBLIC	dev_cmd_
		PUBLIC	dev_read_
		PUBLIC	dev_write_
		PUBLIC	dev_open_
		PUBLIC	dev_close_
		PUBLIC	dev_lseek_
		PUBLIC	dev_fsync_
		PUBLIC	driver_installer_
		PUBLIC	load_driver_
		PUBLIC	_driver_count
		PUBLIC	_driver_func_ptr
		PUBLIC	_drivers
		EXTRN	__STK:BYTE
		EXTRN	strcmp_:BYTE
		EXTRN	allocate_new_driver_:BYTE
		EXTRN	video_mode_:BYTE
		EXTRN	install_display_driver_:BYTE
		EXTRN	_videodriver:BYTE
		EXTRN	_dos_getvect_:BYTE
		EXTRN	sprintf_:BYTE
		EXTRN	_dos_setvect_:BYTE
		EXTRN	dosexec_:BYTE
		EXTRN	__VIDEO_MODE:BYTE
		EXTRN	_video:BYTE
		EXTRN	draw_screen_:BYTE
		EXTRN	strncpy_:BYTE
		EXTRN	_big_code_:BYTE
DGROUP		GROUP	CONST,CONST2,_DATA,_BSS
drivers_TEXT		SEGMENT	BYTE PUBLIC USE16 'CODE'
		ASSUME CS:drivers_TEXT, DS:DGROUP, SS:DGROUP
driver_id_:
    push        ax 
    mov         ax,10H 
    call        far ptr __STK 
    pop         ax 
    push        bx 
    push        cx 
    push        si 
    push        di 
    push        bp 
    mov         bp,sp 
    sub         sp,2 
    mov         di,ax 
    mov         word ptr -2[bp],dx 
    mov         si,word ptr ss:_driver_count 
L$1:
    dec         si 
    cmp         si,0ffffH 
    je          L$2 
    mov         ax,si 
    shl         ax,1 
    shl         ax,1 
    add         ax,si 
    shl         ax,1 
    shl         ax,1 
    sub         ax,si 
    shl         ax,1 
    mov         bx,word ptr ss:_drivers 
    mov         dx,word ptr ss:_drivers+2 
    add         ax,bx 
    mov         bx,di 
    mov         cx,word ptr -2[bp] 
    call        far ptr strcmp_ 
    test        ax,ax 
    jne         L$1 
L$2:
    mov         ax,si 
    mov         sp,bp 
    pop         bp 
    pop         di 
    pop         si 
    pop         cx 
    pop         bx 
    retf        
dev_cmd_:
    push        ax 
    mov         ax,8 
    call        far ptr __STK 
    pop         ax 
    push        si 
    push        di 
    mov         si,ax 
    mov         di,dx 
    mov         dx,bx 
    shl         ax,1 
    shl         ax,1 
    add         ax,si 
    shl         ax,1 
    shl         ax,1 
    sub         ax,si 
    shl         ax,1 
    lds         bx,dword ptr ss:_drivers 
    add         bx,ax 
    mov         si,word ptr 20H[bx] 
    mov         ax,word ptr 22H[bx] 
    test        ax,ax 
    jne         L$3 
    test        si,si 
    je          L$6 
L$3:
    mov         word ptr ss:_driver_func_ptr,si 
    mov         word ptr ss:_driver_func_ptr+2,ax 
    mov         ax,word ptr 6[bx] 
    cmp         ax,1 
    jl          L$5 
    mov         ax,ss 
    mov         es,ax 
    mov         ax,dx 
    mov         dx,cx 
    mov         bx,di 
L$4:
    call        dword ptr es:_driver_func_ptr 
    pop         di 
    pop         si 
    retf        
L$5:
    cmp         ax,0ffffH 
    jne         L$6 
    mov         ax,ss 
    mov         es,ax 
    mov         ax,dx 
    mov         dx,cx 
    jmp         L$4 
L$6:
    mov         ax,0ffffH 
    pop         di 
    pop         si 
    retf        
dev_read_:
    push        ax 
    mov         ax,8 
    call        far ptr __STK 
    pop         ax 
    push        bx 
    push        cx 
    mov         bx,ax 
    shl         ax,1 
    shl         ax,1 
    add         ax,bx 
    shl         ax,1 
    shl         ax,1 
    sub         ax,bx 
    shl         ax,1 
    lds         bx,dword ptr ss:_drivers 
    add         bx,ax 
    mov         ax,word ptr 8[bx] 
    mov         cx,word ptr 0aH[bx] 
    test        cx,cx 
    jne         L$7 
    test        ax,ax 
    je          L$10 
L$7:
    mov         word ptr ss:_driver_func_ptr,ax 
    mov         word ptr ss:_driver_func_ptr+2,cx 
    mov         ax,word ptr 6[bx] 
    cmp         ax,1 
    jl          L$9 
    mov         ax,ss 
    mov         es,ax 
    mov         ax,dx 
L$8:
    call        dword ptr es:_driver_func_ptr 
    pop         cx 
    pop         bx 
    retf        
L$9:
    cmp         ax,0ffffH 
    jne         L$10 
    mov         ax,ss 
    mov         es,ax 
    jmp         L$8 
L$10:
    mov         ax,0ffffH 
    pop         cx 
    pop         bx 
    retf        
dev_write_:
    push        ax 
    mov         ax,8 
    call        far ptr __STK 
    pop         ax 
    push        cx 
    push        si 
    mov         cx,ax 
    mov         si,dx 
    mov         dl,bl 
    shl         ax,1 
    shl         ax,1 
    add         ax,cx 
    shl         ax,1 
    shl         ax,1 
    sub         ax,cx 
    shl         ax,1 
    lds         bx,dword ptr ss:_drivers 
    add         bx,ax 
    mov         ax,word ptr 0cH[bx] 
    mov         cx,word ptr 0eH[bx] 
    test        cx,cx 
    jne         L$11 
    test        ax,ax 
    je          L$14 
L$11:
    mov         cx,ax 
    mov         ax,word ptr 0eH[bx] 
    mov         word ptr ss:_driver_func_ptr,cx 
    mov         word ptr ss:_driver_func_ptr+2,ax 
    mov         ax,word ptr 6[bx] 
    cmp         ax,1 
    jl          L$13 
    mov         bx,ss 
    mov         al,dl 
    xor         ah,ah 
    mov         es,bx 
    mov         bx,si 
L$12:
    call        dword ptr es:_driver_func_ptr 
    pop         si 
    pop         cx 
    retf        
L$13:
    cmp         ax,0ffffH 
    jne         L$14 
    mov         bx,ss 
    mov         al,dl 
    xor         ah,ah 
    mov         es,bx 
    jmp         L$12 
L$14:
    mov         ax,0ffffH 
    pop         si 
    pop         cx 
    retf        
dev_open_:
    push        ax 
    mov         ax,8 
    call        far ptr __STK 
    pop         ax 
    push        cx 
    push        si 
    mov         cx,ax 
    mov         si,dx 
    mov         dx,bx 
    shl         ax,1 
    shl         ax,1 
    add         ax,cx 
    shl         ax,1 
    shl         ax,1 
    sub         ax,cx 
    shl         ax,1 
    lds         bx,dword ptr ss:_drivers 
    add         bx,ax 
    mov         cx,word ptr 10H[bx] 
    mov         ax,word ptr 12H[bx] 
    test        ax,ax 
    jne         L$15 
    test        cx,cx 
    je          L$18 
L$15:
    mov         ax,cx 
    mov         cx,word ptr 12H[bx] 
    mov         word ptr ss:_driver_func_ptr,ax 
    mov         word ptr ss:_driver_func_ptr+2,cx 
    mov         ax,word ptr 6[bx] 
    cmp         ax,1 
    jl          L$17 
    mov         ax,ss 
    mov         es,ax 
    mov         ax,dx 
    mov         bx,si 
L$16:
    call        dword ptr es:_driver_func_ptr 
    pop         si 
    pop         cx 
    retf        
L$17:
    cmp         ax,0ffffH 
    jne         L$18 
    mov         ax,ss 
    mov         es,ax 
    mov         ax,dx 
    jmp         L$16 
L$18:
    mov         ax,0ffffH 
    pop         si 
    pop         cx 
    retf        
dev_close_:
    push        ax 
    mov         ax,8 
    call        far ptr __STK 
    pop         ax 
    push        bx 
    push        cx 
    mov         bx,ax 
    shl         ax,1 
    shl         ax,1 
    add         ax,bx 
    shl         ax,1 
    shl         ax,1 
    sub         ax,bx 
    shl         ax,1 
    lds         bx,dword ptr ss:_drivers 
    add         bx,ax 
    mov         ax,word ptr 14H[bx] 
    mov         cx,word ptr 16H[bx] 
    test        cx,cx 
    jne         L$19 
    test        ax,ax 
    je          L$22 
L$19:
    mov         word ptr ss:_driver_func_ptr,ax 
    mov         word ptr ss:_driver_func_ptr+2,cx 
    mov         ax,word ptr 6[bx] 
    cmp         ax,1 
    jl          L$21 
    mov         ax,ss 
    mov         es,ax 
    mov         ax,dx 
L$20:
    call        dword ptr es:_driver_func_ptr 
    pop         cx 
    pop         bx 
    retf        
L$21:
    cmp         ax,0ffffH 
    jne         L$22 
    mov         ax,ss 
    mov         es,ax 
    jmp         L$20 
L$22:
    mov         ax,0ffffH 
    pop         cx 
    pop         bx 
    retf        
dev_lseek_:
    push        ax 
    mov         ax,8 
    call        far ptr __STK 
    pop         ax 
    push        cx 
    push        si 
    mov         cx,ax 
    mov         si,dx 
    mov         dx,bx 
    shl         ax,1 
    shl         ax,1 
    add         ax,cx 
    shl         ax,1 
    shl         ax,1 
    sub         ax,cx 
    shl         ax,1 
    lds         bx,dword ptr ss:_drivers 
    add         bx,ax 
    mov         cx,word ptr 18H[bx] 
    mov         ax,word ptr 1aH[bx] 
    test        ax,ax 
    jne         L$23 
    test        cx,cx 
    je          L$26 
L$23:
    mov         ax,cx 
    mov         cx,word ptr 1aH[bx] 
    mov         word ptr ss:_driver_func_ptr,ax 
    mov         word ptr ss:_driver_func_ptr+2,cx 
    mov         ax,word ptr 6[bx] 
    cmp         ax,1 
    jl          L$25 
    mov         ax,ss 
    mov         es,ax 
    mov         ax,dx 
    mov         bx,si 
L$24:
    call        dword ptr es:_driver_func_ptr 
    pop         si 
    pop         cx 
    retf        
L$25:
    cmp         ax,0ffffH 
    jne         L$26 
    mov         ax,ss 
    mov         es,ax 
    mov         ax,dx 
    jmp         L$24 
L$26:
    mov         ax,0ffffH 
    pop         si 
    pop         cx 
    retf        
dev_fsync_:
    push        ax 
    mov         ax,8 
    call        far ptr __STK 
    pop         ax 
    push        bx 
    push        cx 
    mov         bx,ax 
    shl         ax,1 
    shl         ax,1 
    add         ax,bx 
    shl         ax,1 
    shl         ax,1 
    sub         ax,bx 
    shl         ax,1 
    lds         bx,dword ptr ss:_drivers 
    add         bx,ax 
    mov         ax,word ptr 1cH[bx] 
    mov         cx,word ptr 1eH[bx] 
    test        cx,cx 
    jne         L$27 
    test        ax,ax 
    je          L$30 
L$27:
    mov         word ptr ss:_driver_func_ptr,ax 
    mov         word ptr ss:_driver_func_ptr+2,cx 
    mov         ax,word ptr 6[bx] 
    cmp         ax,1 
    jl          L$29 
    mov         ax,ss 
    mov         es,ax 
    mov         ax,dx 
L$28:
    call        dword ptr es:_driver_func_ptr 
    pop         cx 
    pop         bx 
    retf        
L$29:
    cmp         ax,0ffffH 
    jne         L$30 
    mov         ax,ss 
    mov         es,ax 
    jmp         L$28 
L$30:
    mov         ax,0ffffH 
    pop         cx 
    pop         bx 
    retf        
driver_installer_:
    push        ax 
    push        cx 
    push        dx 
    push        bx 
    push        sp 
    push        bp 
    push        si 
    push        di 
    push        ds 
    push        es 
    push        ax 
    push        ax 
    mov         bp,sp 
    sub         sp,26H 
    cld         
    mov         ax,word ptr 16H[bp] 
    mov         cx,word ptr 10H[bp] 
    mov         word ptr -8[bp],cx 
    mov         bx,word ptr 12H[bp] 
    mov         dx,word ptr 0aH[bp] 
    mov         word ptr -2[bp],ax 
    mov         word ptr -6[bp],dx 
    mov         word ptr -0aH[bp],bx 
    mov         ax,DGROUP:CONST 
    mov         ds,ax 
    mov         ax,word ptr _driver_count 
    mov         word ptr -4[bp],ax 
    xor         dx,dx 
    call        far ptr allocate_new_driver_ 
    test        ax,ax 
    jne         L$31 
    mov         ax,3 
    call        far ptr video_mode_ 
    mov         word ptr 16H[bp],0ffffH 
    jmp         near ptr L$35 
L$31:
    mov         bx,word ptr -4[bp] 
    shl         bx,1 
    shl         bx,1 
    add         bx,word ptr -4[bp] 
    shl         bx,1 
    shl         bx,1 
    sub         bx,word ptr -4[bp] 
    shl         bx,1 
    mov         ax,DGROUP:CONST 
    mov         ds,ax 
    mov         ax,word ptr _drivers 
    mov         si,word ptr _drivers+2 
    add         bx,ax 
    mov         word ptr -24H[bp],si 
    lea         ax,20H[bx] 
    mov         word ptr -26H[bp],ax 
    mov         word ptr -20H[bp],si 
    lea         ax,8[bx] 
    mov         word ptr -22H[bp],ax 
    mov         word ptr -1cH[bp],si 
    lea         ax,0cH[bx] 
    mov         word ptr -1eH[bp],ax 
    mov         word ptr -18H[bp],si 
    lea         ax,10H[bx] 
    mov         word ptr -1aH[bp],ax 
    mov         word ptr -14H[bp],si 
    lea         ax,14H[bx] 
    mov         word ptr -16H[bp],ax 
    mov         word ptr -10H[bp],si 
    lea         ax,18H[bx] 
    mov         word ptr -12H[bp],ax 
    mov         word ptr -0cH[bp],si 
    add         bx,1cH 
    mov         word ptr -0eH[bp],bx 
    mov         es,word ptr -2[bp] 
L$32:
    mov         si,dx 
    shl         si,1 
    shl         si,1 
    mov         bx,cx 
    add         bx,si 
    lds         di,dword ptr -26H[bp+si] 
    mov         si,word ptr es:[bx] 
    mov         bx,word ptr es:2[bx] 
    mov         word ptr [di],si 
    mov         word ptr 2[di],bx 
    inc         dx 
    je          L$33 
    mov         si,dx 
    shl         si,1 
    shl         si,1 
    mov         ds,word ptr -2[bp] 
    add         si,cx 
    mov         di,word ptr [si] 
    mov         bx,word ptr 2[si] 
    cmp         bx,0ffffH 
    jne         L$32 
    cmp         di,0ffffH 
    jne         L$32 
L$33:
    inc         dx 
    mov         bx,dx 
    shl         bx,1 
    shl         bx,1 
    mov         ds,word ptr -2[bp] 
    add         bx,cx 
    mov         al,byte ptr [bx] 
    xor         ah,ah 
    mov         bx,word ptr -4[bp] 
    shl         bx,1 
    shl         bx,1 
    add         bx,word ptr -4[bp] 
    shl         bx,1 
    shl         bx,1 
    sub         bx,word ptr -4[bp] 
    shl         bx,1 
    mov         si,DGROUP:CONST 
    mov         ds,si 
    lds         si,dword ptr _drivers 
    add         si,bx 
    mov         word ptr 24H[si],ax 
    inc         dx 
    shl         dx,1 
    shl         dx,1 
    mov         ds,word ptr -2[bp] 
    mov         si,cx 
    add         si,dx 
    mov         dl,byte ptr [si] 
    xor         dh,dh 
    mov         ax,DGROUP:CONST 
    mov         ds,ax 
    lds         cx,dword ptr _drivers 
    add         bx,cx 
    mov         word ptr 6[bx],dx 
    cmp         word ptr -8[bp],1 
    jne         L$34 
    mov         ax,word ptr -6[bp] 
    mov         dx,word ptr -0aH[bp] 
    call        far ptr install_display_driver_ 
    mov         ax,DGROUP:CONST 
    mov         ds,ax 
    mov         ax,word ptr _driver_count 
    mov         word ptr _videodriver,ax 
L$34:
    mov         word ptr 16H[bp],0 
L$35:
    mov         sp,bp 
    pop         ax 
    pop         ax 
    pop         es 
    pop         ds 
    pop         di 
    pop         si 
    pop         bp 
    pop         bx 
    pop         bx 
    pop         dx 
    pop         cx 
    pop         ax 
    iret        
load_driver_:
    push        ax 
    mov         ax,22H 
    call        far ptr __STK 
    pop         ax 
    push        si 
    push        di 
    push        bp 
    mov         bp,sp 
    sub         sp,6 
    push        ax 
    push        dx 
    push        bx 
    push        cx 
    xor         si,si 
    mov         di,word ptr ss:_driver_count 
    mov         al,0 
    int         58H 
    mov         word ptr -2[bp],ax 
L$36:
    inc         si 
    mov         ax,si 
    call        far ptr _dos_getvect_ 
    test        dx,dx 
    jne         L$37 
    test        ax,ax 
    je          L$38 
L$37:
    test        si,si 
    jne         L$36 
    mov         ax,0ffffH 
    jmp         near ptr L$41 
L$38:
    push        si 
    push        ss 
    mov         ax,offset DGROUP:L$42 
    push        ax 
    lea         dx,-6[bp] 
    push        ss 
    push        dx 
    call        far ptr sprintf_ 
    add         sp,0aH 
    mov         bx,offset driver_installer_ 
    mov         cx,seg driver_installer_ 
    mov         ax,si 
    call        far ptr _dos_setvect_ 
    mov         bx,82H 
    mov         al,1 
    int         58H 
    mov         cx,ss 
    lea         bx,-6[bp] 
    mov         ax,word ptr -8[bp] 
    mov         dx,word ptr -0aH[bp] 
    call        far ptr dosexec_ 
    mov         bl,byte ptr -2[bp] 
    xor         bh,bh 
    mov         al,1 
    int         58H 
    mov         al,byte ptr ss:__VIDEO_MODE 
    cmp         al,byte ptr ss:_video 
    je          L$39 
    mov         al,byte ptr ss:_video 
    xor         ah,ah 
    call        far ptr video_mode_ 
    call        far ptr draw_screen_ 
L$39:
    xor         bx,bx 
    xor         cx,cx 
    mov         ax,si 
    call        far ptr _dos_setvect_ 
    cmp         di,word ptr ss:_driver_count 
    je          L$40 
    mov         ax,5 
    push        ax 
    mov         ax,di 
    shl         ax,1 
    shl         ax,1 
    add         ax,di 
    shl         ax,1 
    shl         ax,1 
    sub         ax,di 
    shl         ax,1 
    mov         bx,word ptr ss:_drivers 
    mov         dx,word ptr ss:_drivers+2 
    add         ax,bx 
    mov         bx,word ptr -0cH[bp] 
    mov         cx,word ptr -0eH[bp] 
    call        far ptr strncpy_ 
L$40:
    xor         ax,ax 
L$41:
    mov         sp,bp 
    pop         bp 
    pop         di 
    pop         si 
    retf        
drivers_TEXT		ENDS
CONST		SEGMENT	WORD PUBLIC USE16 'DATA'
L$42:
    DB	25H, 58H, 0

CONST		ENDS
CONST2		SEGMENT	WORD PUBLIC USE16 'DATA'
CONST2		ENDS
_DATA		SEGMENT	WORD PUBLIC USE16 'DATA'
_driver_count:
    DB	0, 0

_DATA		ENDS
_BSS		SEGMENT	WORD PUBLIC USE16 'BSS'
    ORG 0
_driver_func_ptr    LABEL	BYTE
    ORG 4
_drivers    LABEL	BYTE
    ORG 8
_BSS		ENDS

		END
