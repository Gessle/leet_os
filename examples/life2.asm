code auto
data auto
interrupts 2
stack 64

; game of life

jmp main

;grid size 100x100
str grid 10000
str newgrid 10000
word grid_width 100
word grid_height 100

; get cell state by index
; arguments: ax == cell index
; returns zero or non-zero
:get_cell_index_state
push bx
mov bx $grid
add bx ax
mov ah 0
mov al *bx
pop bx
ret

; get cell state by coords
; arguments: ax == x, bx == y
; returns zero or non-zero
:get_cell_state
push bx
push cx
push dx
mov dx 0
; safety check

mov cx $grid_height
cmp bx cx
jge get_cell_state_end

mov cx 0x8000
and cx bx
cmp cx dx
jne get_cell_state_end
mov cx 0x8000
and cx ax
cmp cx dx
jne get_cell_state_end


mov cx $grid_width
cmp ax cx
jge get_cell_state_end


push ax
push bx
push cx
push dx
push ex
mov cx bx
mov bx ax
mov dx 2
mov ex $window_desc
mov ax 5
scr
pop ex
pop dx
pop cx
pop bx
pop ax

;mov cx $grid_width
mul bx cx
add bx ax
mov ax bx

call get_cell_index_state
mov dx ax
:get_cell_state_end
mov ax dx
pop dx
pop cx
pop bx
ret

; calculate live neighbours of a cell ax, bx
; return value in ax
:live_neighbours
push bx
push cx
push dx
push ex
mov cx 0
mov dx 0

; get state of ax-1, bx
push ax
dec ax
call get_cell_state
cmp ax dx
pop ax
je live_neighbours_j0
inc cx
:live_neighbours_j0

push ax
inc ax
call get_cell_state
cmp ax dx
pop ax
je live_neighbours_j1
inc cx
:live_neighbours_j1

; get state of ax ..., bx -1
push bx
dec bx

push ax
dec ax
call get_cell_state
cmp ax dx
pop ax
je live_neighbours_j2
inc cx
:live_neighbours_j2

push ax
call get_cell_state
cmp ax dx
pop ax
je live_neighbours_j3
inc cx
:live_neighbours_j3

push ax
inc ax
call get_cell_state
cmp ax dx
pop ax
je live_neighbours_j4
inc cx
:live_neighbours_j4

pop bx

; get state of ax ..., bx + 1
push bx
inc bx

push ax
dec ax
call get_cell_state
cmp ax dx
pop ax
je live_neighbours_j5
inc cx
:live_neighbours_j5

push ax
call get_cell_state
cmp ax dx
pop ax
je live_neighbours_j6
inc cx
:live_neighbours_j6

push ax
inc ax
call get_cell_state
cmp ax dx
pop ax
je live_neighbours_j7
inc cx
:live_neighbours_j7

pop bx

mov ax cx
pop ex
pop dx
pop cx
pop bx
ret

; plot a live cell to the new grid
; arguments: ax = x, bx = y
:plot_live_cell
push ax
push bx
push cx
mov cx $grid_width
mul cx bx
add cx ax

mov ax $newgrid
add ax cx
mov *cx 0xF

pop cx
pop bx
pop ax
ret

:plot_dead_cell
push ax
push bx
push cx
mov cx $grid_width
mul cx bx
add cx ax

mov ax $newgrid
add ax cx
mov *cx 0

pop cx
pop bx
pop ax
ret

; live cell transition
; arguments: ax = x, bx = y
:live_cell_trans
push ax
push bx
push cx
; get number of live neighbours to ax
call live_neighbours

mov cx 2
cmp ax cx
; if the cell has fewer than two live neighbours,
; kill the cell
jle live_cell_trans_live0

pop ax
call plot_dead_cell
push ax

:live_cell_trans_live0
mov cx 3
cmp ax cx
; if the cell has more than three live neighbours
; kill the cell
jle live_cell_trans_live1

pop ax
call plot_dead_cell
push ax

:live_cell_trans_live1

;pop ax
;call plot_live_cell
;push ax

pop cx
pop bx
pop ax
ret

:dead_cell_trans
push ax
push bx
push cx
; get number of live neighbours
call live_neighbours
; if the cell has three live neighbours,
; the cell becomes alive
mov cx 3
cmp ax cx
jne dead_cell_trans_end
pop ax
printuint ax
printuint bx
call plot_live_cell
push ax
:dead_cell_trans_end
pop cx
pop bx
pop ax
ret

; loop through a row
; args: bx == row
:cycle_loop_row
push ax
push bx
push cx
push dx
mov cx $grid_width
:loop_row_loop
dec cx

mov ax cx
push ax
call get_cell_state
mov dx 0
cmp ax dx
pop ax
; if the cell is alive, jump
jne cycle_loop_row_livecell
; else do something
; ax is now the x coord
; bx is the y coord
call dead_cell_trans

jmp cycle_loop_row_cont

:cycle_loop_row_livecell
call live_cell_trans

:cycle_loop_row_cont

jcz cycle_loop_row_end

jmp loop_row_loop
:cycle_loop_row_end
pop dx
pop cx
pop bx
pop ax
ret

; loop through the grid and change cell states
:cycle_loop
push ax
push bx
mov ax 0

mov bx $grid_height
:cycle_loop_loop
dec bx
call cycle_loop_row
cmp ax bx
jne cycle_loop_loop

pop bx
pop ax
ret

; draw contents of grid to window
; arguments: ex == window descriptor
:update_window
push ax
push bx
push cx
push dx

pop dx
pop cx
pop bx
pop ax


:main
;create window
str title "Cellular automata"
mov ax 0
mov bx $grid_width
mov cx $grid_height
mov dx $title
scr
word window_desc
mov $window_desc ax

;mov ax .grid
;mov bx $grid
;mov cx 550
;add bx cx
;mov cx 3
;mov dx 0xF
;memset

;mov ax .grid
;mov bx $grid
;mov cx 900
;add bx cx
;mov cx 50
;mov dx 0xF
;memset

mov ax 5
mov bx 5
call plot_live_cell

mov ax 6
mov bx 5
call plot_live_cell

mov ax 7
mov bx 5
call plot_live_cell
call get_cell_state

mov ax 6
mov bx 4
call live_neighbours

:main_loop

mov ax 7
mov bx 0
mov cx 0
mov dx $grid
mov ex $window_desc
mov fx 10000
scr

mov ax .grid
mov bx $grid
mov cx 10000
mov dx .newgrid
mov ex $newgrid
memcpy

call cycle_loop

mov ax .newgrid
mov bx $newgrid
mov cx 10000
mov dx .grid
mov ex $grid
memcpy

jmp main_loop

str debug "asd"
