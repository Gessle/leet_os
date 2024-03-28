str kerronimi "Kerro nimesi"
str hei "Terve, "
str nimi 20

; annetaan kayttajalle kehotus kertoa nimensa
mov ax $kerronimi
printstr ax

; otetaan kayttajan syote vastaan
mov ax $nimi
add ax ex
mov bx 12
getstr ax bx

; tervehditaan kayttajaa
mov ax $hei
mov bx $nimi
printstr ax
printstr bx