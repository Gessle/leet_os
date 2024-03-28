; Window list test (big textbox)

jmp main

:createwindow
str title "Iso tekstilaatikkoikkuna"
word window 0
mov ax 0
mov bx 400
mov cx 300
mov dx $title
mov ex 1
mov fx 0
scr
mov $window ax
ret

:init_itemtable
str items 10
str row1 "Ensimm„inen rivi"
str row2 "Toinen rivi"
str row3 "Kolmas rivi"
str row4 "Nelj„s rivi"
str row5 "Viides rivi"
mov ax $items
mov cx 2
mov bx $row1
mov *ax bx
add ax cx
mov bx $row2
mov *ax bx
add ax cx
mov bx $row3
mov *ax bx
add ax cx
mov bx $row4
mov *ax bx
add ax cx
mov bx $row5
mov *ax bx
ret

:createlist
;scr call for window list
mov ax 16
; X coords in window
mov bx 50
; Y coords in window
mov cx 50
; ptr to array of pointers to strings
mov dx $items
; window descriptor
mov ex $window
; width
mov fx 200
; height
mov gx 150
; count of entries
mov hx 5
scr
ret

:updatewin
mov ax 3
scr
ret

:main
call createwindow
call init_itemtable
call createlist
call updatewin