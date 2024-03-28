; skip whitespaces
; ax == pointer to string
; returns: pointer to first position
; after whitespaces, if any

:skip_whitespaces
push bx
mov bl ' '

:skip_whitespaces_loop
mov bh *ax
cmp bh bl
jne skip_whitespaces_loop

pop bx
