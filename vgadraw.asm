.8086

.MODEL large

public put_pixel_ega4_
public get_pixel_ega4_
public sync_pixels_ega4_
public _ega4_lastoperation

DGROUP          GROUP   _DATA
main_TEXT               SEGMENT BYTE PUBLIC USE16 'CODE'
;                ASSUME CS:main_TEXT, DS:DGROUP, SS:DGROUP
                ASSUME CS:main_TEXT

_ega4_lastoperation: db 0xFF
;lastcolor: db 0x00
;pixmask: db 0x00
;lastoffset: dw 0x0000

sync_pixels_ega4_ proc far
  push ax
  push bx
  push cx
  push dx

  cmp cs:_ega4_lastoperation, 0
  je sync_nosetregs
  mov dx, 0x3CE
  mov ax, 0x0205
  out dx, ax
  mov al, 0x08
  out dx, al
  mov cs:_ega4_lastoperation, 0
  sync_nosetregs:

  mov dx, 0x3CF

;  mov al, cs:pixmask
  db 0xB0
  pixmask: db 0

  out dx, al

  mov ax, 0xA000
  mov es, ax
  mov bx, cs:lastoffset
  mov al, cs:lastcolor
  xchg es:[bx], al
  mov cs:pixmask, 0

  pop dx
  pop cx
  pop bx
  pop ax
retf
sync_pixels_ega4_ endp

; ax = x, bx = y
put_pixel_ega4_ proc far
  push dx
  push cx

  ; multiply bx with 80
  mov cl, 6
  mov dx, bx
  shl bx, cl
  mov cl, 4
  shl dx, cl
  add bx, dx
  ; add x/8
  mov dl, al
  dec cl
  shr ax, cl
  add bx, ax
  ; now bx == offset in video memory

  pop cx
;  cmp bx, cs:lastoffset
  db 0x81, 0xFB ; compare with immediate value
  lastoffset: dw 00

  jne put_pixels
;  cmp cl, cs:lastcolor
  db 0x80, 0xF9 ; compare with immediate
  lastcolor: db 0x00

  je color_eq
  put_pixels:

  test cs:pixmask, 0xFF
  jz dontdraw1

  call sync_pixels_ega4_

  dontdraw1:
  mov cs:lastcolor, cl
  mov cs:lastoffset, bx

  color_eq:
  

  ; calculate the bit to set
  and dl, 7
  mov cl, dl
  mov dl, 0x80
  shr dl, cl

  or cs:pixmask, dl

  pop dx
retf
put_pixel_ega4_ endp

; ax = x, bx = y
get_pixel_ega4_ proc far
  push dx
  push cx
;  call near ptr sync_pixels

  ; multiply bx with 80
  mov cl, 6
  mov dx, bx
  shl bx, cl
  mov cl, 4
  shl dx, cl
  add bx, dx
  ; add x/8
  mov dl, al
  dec cl
  shr ax, cl
  add bx, ax
  ; now bx == offset in video memory

  ; calculate the bit to set
  and dl, 7
  mov cl, dl
  mov dl, 0x80
  shr dl, cl
  mov cl, dl

  cmp bx, cs:lastoffset
  jne getpixel_setes
  test cs:pixmask, cl
  jz getpixel_setes
;  cmp cs:_ega4_lastoperation, 0xFF
;  je getpixel_setes

  mov al, cs:lastcolor
  jmp get_pixel_end


  getpixel_setes:

  mov ax, 0xA000
  mov es, ax

  mov ax, 0x0404
  mov dx, 0x3CE
  cmp cs:_ega4_lastoperation, 1
  je read_nosetregs

  test cs:pixmask, 0xFF
  jz dontdraw2 

  call sync_pixels_ega4_

  dontdraw2:
  out dx, ax
  mov cs:_ega4_lastoperation, 1
  read_nosetregs:


  inc dx
  xor ah, ah

  get_pixel_loop:
  test es:[bx], cl
  jz no_bit_set
  or ah, 1
  no_bit_set:
  dec al
  js get_pixel_endloop
  out dx, al
  shl ah, 1
  jmp get_pixel_loop
  get_pixel_endloop:

  xchg al, ah

  get_pixel_end:

;  mov cs:lastoffset, bx
;  mov cs:lastcolor, al
;  mov cs:pixmask, cl

  pop cx
  pop dx
retf
get_pixel_ega4_ endp

main_TEXT ends

_DATA           SEGMENT WORD PUBLIC USE16 'DATA'

_DATA ENDS

END

