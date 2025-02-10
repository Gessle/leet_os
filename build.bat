REM Type "build.bat debug" if you want to make a debug build.
echo Building the main program...

wasm -q vpu\vpudint.asm
wasm -q timerrtn.asm
wasm -q int28hdl\idlehdl.asm
wasm -q vgadraw.asm

if "%1"=="debug" goto debug_build

wcc -dMAIN %1 %2 %3 %4  -s -ml -os -ox -q main.c
wcc %1 %2 %3 %4  -s -ml -os -ox -q vpu\console\vpu_main.c
wcc %1 %2 %3 %4  -s -ml -os -ox -q vpu\serwrite.c
wcc %1 %2 %3 %4  -s -ml -os -zu -ox -q vpu\serrecv.c
wcc %1 %2 %3 %4  -s -ml -os -ox -oh -q drvmod\drivers.c
rem wcc -s -ml -od -q drvmod\drivers.c
wcc %1 %2 %3 %4  -s -ml -os -zu -ox -q  drvmod\drvinst.c
wcc %1 %2 %3 %4  -s -ml -os -zu -ox -q -r vpu\vpuhwint.c
wcc %1 %2 %3 %4  -s -ml -os -ox -q xpmparse\xpmparse.c
wcc %1 %2 %3 %4  -s -ml -os -ox -q vpu\86serpar.c
wcc %1 %2 %3 %4  -s -ml -os -ox -q ibmpc.c
wcc %1 %2 %3 %4  -s -ml -os -ox -q int28hdl\int28hdl.c

goto link
:debug_build
wcc %2 %3 %4 -s -ml -os -zu dalloc.c
wcc %2 %3 %4 -dDEBUG_ON -dMAIN -ee -ep -en -ml -os -ox main.c
wcc %2 %3 %4 -dDEBUG_ON -ee -ep -en -ml -os vpu\console\vpu_main.c
wcc %2 %3 %4 -dDEBUG_ON -ee -ep -en -ml -os vpu\serwrite.c
wcc %2 %3 %4 -dDEBUG_ON -ee -ep -en -ml -os -zu vpu\serrecv.c
wcc %2 %3 %4 -dDEBUG_ON -ee -ep -en -ml -os drvmod\drivers.c
wcc %2 %3 %4 -dDEBUG_ON -ee -ep -en -ml -os -zu drvmod\drvinst.c
wcc %2 %3 %4 -dDEBUG_ON -ee -ep -en -ml -os -zu -r vpu\vpuhwint.c
wcc %2 %3 %4 -dDEBUG_ON -ee -ep -en -ml -os xpmparse\xpmparse.c
wcc %2 %3 %4 -dDEBUG_ON -ee -ep -en -ml -os vpu\86serpar.c
wcc %2 %3 %4 -dDEBUG_ON -ee -ep -en -ml -os ibmpc.c
wcc %2 %3 %3 -dDEBUG_ON -ee -ep -en -ml -os int28hdl\int28hdl.c

link.bat debug
goto endbuild

:link
link.bat

:endbuild

echo on
