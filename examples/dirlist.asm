; dir list example

str filename 13
str stat_struct 20
word dir_desc 0
str dir "C:\\WINNT"

;chdir
mov ax 4
mov bx $dir
sys


; open dir
mov ax 5
mov bx $dir
sys

cmp ax ex
jne end

mov $dir_desc ax
mov ex 0
mov cx $filename
:loop
;readdir
mov bx $dir_desc
mov ax 6
sys
cmp ax ex
jne close

printstr cx
printuint dx

;stat
mov ax 10
mov bx $stat_struct
sys
mov al *bx
printuint al
inc bx
mov ax *bx
printuint ax
mov dx 4
add bx dx
mov ax *bx
printuint ax

jmp loop

;close dir
:close
mov ax 7
sys
:end