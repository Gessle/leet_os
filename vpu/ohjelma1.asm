stack 512
code 2048 1
data 4096 1
interrupts 5

.data
unsigned short ptr2 0x0202

str str1 "Näppäintä painettu"

.code

mov ax, str1
setint 0x0001, intr_handler
jmp end

:intr_handler
printstr ax
iret

:end
wait
