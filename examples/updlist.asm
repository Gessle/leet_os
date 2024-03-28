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

str items { "Eka rivi" "2. rivi" "3. rivi" "4. rivi" "5" }
str items2 { "p””" "dsdsdsds" "gfdggf" }

:createlist
;scr call for window list
mov ax 16
; X coords in window
mov bx 50
; Y coords in window
mov cx 50
; ptr to array of pointers to strings
mov dx &items
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

:updatelist
mov ax 17
mov dx &items2
mov ex $window
mov hx 3
scr
ret

:updatewin
mov ax 3
scr
ret

:main
call createwindow
;call init_itemtable
call createlist
call updatewin
wait
call updatelist
call updatewin