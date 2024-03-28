#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>

char message[1000];
#define putstr(x) puts(x)
void nomemory(){}

#include "biosser.c"

void main()
{
  serial_init(0, 0x0607);
  while(1)
    puts(serport_buffer[0]->buffer);;
}
