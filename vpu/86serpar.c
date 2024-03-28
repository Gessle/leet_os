#include <stdlib.h>
#include <dos.h>
//#include <i86.h>

#include "..\extdlc.c"

unsigned char inbyte(unsigned port);
#pragma aux inbyte = \
  "in al, dx" \
  parm [dx] value [al];

void outbyte(unsigned port, unsigned char byte);
#pragma aux outbyte = \
  "out dx, al" \
  parm [dx] [ax];

void nomemory(void);

#include "biosser.c"
#include "biospar.c"
