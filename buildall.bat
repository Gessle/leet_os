echo off

echo Compiling installer...
wcl -bcl=com -s -os -ox -oh -q install.c

echo Compiling startup program...
wcl -bcl=com -s -os -ox -oh -q stshell.c
echo Compiling shortcut editor...
wcl -bcl=com -s -os -ox -oh -q sceditor.c
echo Compiling icon editor...
wcl -bcl=com -s -os -ox -oh -q iconedit.c
echo Compiling font editor...
wcl -bcl=com -s -os -ox -oh -q fontedit.c
echo Compiling cursor editor...
wcl -bcl=com -s -os -ox -oh -q curedit.c
echo Compiling configuration program...
wcl -bcl=com -s -os -ox -oh -q config.c
echo Compiling filetype associations editor...
wcl -bcl=com -s -os -ox -oh -q ascsfx.c

echo Compiling screen savers...
wcl -bcl=com -s -os -ox -oh -q matrix.c
move /Y matrix.com scrnsvrs\
wcl -bcl=com -s -os -ox -oh -q vgastars.c
move /Y vgastars.com scrnsvrs\
wcl -bcl=com -s -os -ox -oh -q cgastars.c
move /Y cgastars.com scrnsvrs\

echo Compiling XPM3 to XPM2 converter...
wcl -s -os -ox -oh -q -mc xpmparse\2xpm2.c

echo Assembling XPM2 viewer...
assemble xpmview.asm xpmview.app

echo Assembling command interpreter...
assemble dosh.asm dosh.app

echo Compiling the assembler...
cd vpu\assemble\
build2
rem cd ..\..\


rem echo Building the main program...
rem build

echo on
