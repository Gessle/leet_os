stack 512
code 1 2048
data 1 4096
interrupts 5

str str1 "Näppäintä painettu"


mov ax $str1
setint 0x0001 intr_handler
jmp end

:intr_handler
printstr ax
iret

:end
wait
