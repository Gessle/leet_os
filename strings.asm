.8086

.MODEL large


PUBLIC copymem_
PUBLIC lenstr_
PUBLIC copystr_
PUBLIC setmem_

_TEXT               SEGMENT BYTE PUBLIC USE16 'CODE'
                ASSUME CS:_TEXT                               

; ds:si = source, es:di = destination, cx = count
copymem_ proc far
  pushf
  cld  
  push bx
  push bp  
  push cx
  mov bp, sp
  
; prevent pointer from overflowing
  push di
  push es
  mov bx, di
  mov cl, 4
  shr bx, cl ; how many paragraphs to add
  mov cx, es
  add cx, bx
  mov es, cx
  and di, 0xF ; di = di % 0x10
  
; same for source
  push si
  push ds
  mov bx, si
  mov cl, 4
  shr bx, cl
  mov cx, ds
  add cx, bx
  mov ds, cx
  and si, 0xF  
  
  mov cx, [bp]
  
  ; if source operand seems to be in VGA memory, copy byte by byte
  push ax
  mov ax, ds
  cmp ax, 0xA000
  pop ax
  jge copy_bytes
  
  test cx, 0x01
  jz copy_words ; if even count, copy only words
  ; else copy the last byte
  
  push di
  push si
  push ax
  dec cx
  add si, cx
  add di, cx
  mov al, ds:[si]
  mov es:[di], al
  pop ax
  pop si
  pop di
  
  copy_words:
  shr cx, 1
  rep movsw
  jmp copymem_end
  
  copy_bytes:
  rep movsb
  
  copymem_end:
  pop ds
  pop si
  pop es
  pop di
  pop cx
  pop bp
  pop bx
  popf
  retf  
copymem_ endp

; es:di = destination, al = set byte, cx = count
setmem_ proc far
  pushf
  cld
  push bp
  push cx
  mov bp, sp
  push bx
; prevent pointer from overflowing
  push di
  push es
  mov bx, di
  mov cl, 4
  shr bx, cl ; how many paragraphs to add
  mov cx, es
  add cx, bx
  mov es, cx
  and di, 0xF ; di = di % 0x10

  mov cx, [bp]
  test cx, 0x01
  jz set_words
  
  dec cx
  push di
  add di, cx
  mov es:[di], al
  pop di
  
  set_words:
  mov ah, al
  shr cx, 1
  rep stosw

  pop es
  pop di
  pop bx
  pop cx
  pop bp
  popf
  retf
setmem_ endp

; es:di = pointer to string
lenstr_ proc far
  pushf
  cld
  push cx
  push bx
; prevent pointer from overflowing
  push di
  push es
  mov bx, di
  mov cl, 4
  shr bx, cl ; how many paragraphs to add
  mov cx, es
  add cx, bx
  mov es, cx
  and di, 0xF ; di = di % 0x10
; find null byte  
  mov cx, 0xFFFF
  sub cx, di
  mov al, 0
  push di
  repne scasb
  mov ax, di
  dec ax
  pop di
; string length == ax - di
  sub ax, di
  pop es
  pop di
  pop bx
  pop cx
  popf
  retf
lenstr_ endp

; ds:si = source, es:di = destination
copystr_ proc far
; get length of the string
  push ax
  push cx
  push es
  push di
  mov di, si
  mov ax, ds
  mov es, ax
  call lenstr_
  pop di
  pop es
  mov cx, ax ; cx = length of string
  inc cx
  call copymem_
  pop cx
  pop ax
  retf
copystr_ endp

_TEXT ENDS

END


