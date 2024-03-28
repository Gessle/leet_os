wcc -ms -s -zdf -r -zu dbuffvga.c
REM wcc -ms -s -r dbuffvga.c
wasm dbuffasm.asm
wlink SYSTEM DOS FILE dbuffasm.obj FILE dbuffvga.obj NAME dbuffvga OPTION MAP=dbuffvga.map
