stack 1166
code 2 32000
data 2 32000
interrupts 28

; määritellään merkkijono hello
str hello "Hello world!"

; kopioidaan ax-rekisteriin merkkijonon alkuosoite
mov ax $hello

; kopioidaan cx-rekisteriin numero 0x05
mov cx 0x05
:label0

; jos cx-rekisteri on nolla, hypätään kohtaan end
jcz end

; kirjoitetaan ruudulle merkkijono, joka löytyy rekisterin ax viittaamasta osoitteesta
printstr ax

; kirjoitetaan ruudulle rekisterin cx sisältö
printuint cx

; dekrementoidaan cx-rekisterin arvo
dec cx

; hypätään kohtaan label0
jmp label0

:end

