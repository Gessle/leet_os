.8086

.MODEL large

public dosidle_handler_
public _diskio_in_progress
public int21wrp_
public _old_int21hdl
public _old_dosidle_handler
public save_stack_seg_
extern int28h_hdl_:BYTE

_TEXT               SEGMENT BYTE PUBLIC USE16 'CODE'
                ASSUME CS:_TEXT                               

_diskio_in_progress: db 0
syscall: db 0
own_ss: dw 0
own_sp: dw 0
;own_ss_2: dw 0
own_sp_2: dw 0
krn_ss: dw 0
krn_sp: dw 0

save_stack_seg_ proc far
  mov cs:own_ss, ss
  retf
save_stack_seg_ endp

dosidle_handler_ proc far
  mov cs:krn_ss, ss
  mov cs:krn_sp, sp
  cli
  mov ss, cs:own_ss
  mov sp, cs:own_sp
  sub sp, 0x20
  sti

  inc cs:_diskio_in_progress

  push ax
;  push bx
;  push cx
;  push dx
;  push di
;  push si
  push es
  push ds
;  push bp
  call far ptr int28h_hdl_
;  pop bp
  pop ds
  pop es
;  pop si
;  pop di
;  pop dx
;  pop cx
;  pop bx
  pop ax

  dec cs:_diskio_in_progress

  mov ss, cs:krn_ss
  mov sp, cs:krn_sp
  db 0xEA
  _old_dosidle_handler: dd 0
dosidle_handler_ endp

int21wrp_ proc far
  push bp                            ; save bp
  xchg bp, sp                        ; xchg bp, sp
  mov sp, ss:[bp+6]                  ; get iret flags to sp
  and sp, NOT 0x0200                     ; disable interrupts from iret flags
  xchg sp, bp                        ; restore sp and bp
  push bp                            ; bp to flags
  popf                               ;
  sti                                ; enable interrupts
  pop bp                             ; restore bp
  

  pushf                              ; save flags for the test instruction
  test cs:_diskio_in_progress, 0xFF  ; is disk i/o in progress?
  jnz int21wrp_kernelstack           ; if yes, switch to kernel stack
  test cs:syscall, 0xFF              ; if no, is a DOS syscall in progress?
  jz int21wrp_save_stack             ; if no, save my own stack
  popf                               ; else restore flags
  jmp int21wrp_ownstack              ; and jump to original int21h handler
  
  int21wrp_save_stack:
  popf                               ; restore flags
  mov cs:own_sp, sp                  ; save my own stack
  jmp int21wrp_ownstack              ; 

  int21wrp_kernelstack:              ; if use kernel stack:
  popf                               ; restore flags
  mov cs:own_sp_2, sp                ; save own stack
  mov ss, cs:krn_ss                  ; restore kernel stack
  mov sp, cs:krn_sp                  ;

  int21wrp_ownstack:                 ; call to original int 21h handler
  pushf                              ; push flags
  inc cs:syscall
  popf

  pushf
  cli
  db 0x9A                            ; far call
  _old_int21hdl: dd 0                ; 

  pushf                              ; save flags
  dec cs:syscall

  test cs:_diskio_in_progress, 0xFF  ; is disk i/o in progress?
  jz int21wrp_end                    ; if not, we are not in kernel stack

  popf                               ; restore flags
  mov ss, cs:own_ss                ; restore my own stack
  mov sp, cs:own_sp_2                ; 
  jmp int21wrp_restoreflags          ; jump

  int21wrp_end:
  popf                               ; restore flags
  int21wrp_restoreflags:

  push bp                            ; save bp
  pushf                              ; move flags to bp
  pop bp                             ;
  cli
  xchg bp, sp                        ; move flags to sp, sp to bp
  mov word ptr ss:[bp+6], sp         ; save flags from sp for iret
  mov sp, bp                         ; restore sp
  sti
  pop bp                             ; restore bp

  iret
int21wrp_ endp

_TEXT ENDS

END
