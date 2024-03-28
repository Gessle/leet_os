#include <conio.h>
#include <dos.h>

unsigned char inbyte(unsigned port);
#pragma aux inbyte = \
  "in al, dx" \
  parm [dx] value [al];

void outbyte(unsigned port, unsigned char byte);
#pragma aux outbyte = \
  "out dx, al" \
  parm [dx] [ax];

#include "..\extdlc.c"

#include "serdef.c"

extern unsigned short __far *serialbase;
extern struct serport_buffer __far *serport_buffer[];

inline unsigned char serial_flow_control_send(unsigned char port)
{
  if(serport_buffer[port]->flow_control & 0x01)
  {
    // XON/XOFF
    if(serport_buffer[port]->xoff_received)
      return 1;
  }
  if(serport_buffer[port]->flow_control & 0x02)  
  {  
    // RTS/CTS
    // check CTS line
    if(!(inbyte(serialbase[port]+6) & 0x10))
      return 1;      
  }
  return 0;
}


unsigned char serial_write(unsigned char port, unsigned char chr)
{
  // wait until bit 5 is set
  while(!(inbyte(serialbase[port]+5) & 0x20));
  // check flow control
  if(serial_flow_control_send(port))
    return 1;

  outbyte(serialbase[port], chr);
  if(inbyte(serialbase[port]+5) & 0x02)
    return 1; // overrun
  
  return 0;
}


