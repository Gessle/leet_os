#include "serdef.c"

unsigned char inbyte(unsigned port);
#pragma aux inbyte = \
  "in al, dx" \
  parm [dx] value [al];

void outbyte(unsigned port, unsigned char byte);
#pragma aux outbyte = \
  "out dx, al" \
  parm [dx] [ax];

extern unsigned short __far *serialbase;

extern struct serport_buffer __far *serport_buffer[];

inline int serial_send(unsigned char port, unsigned char chr)
{
  // wait until bit 5 is set
  while(!(inbyte(serialbase[port]+5) & 0x20));

  outbyte(serialbase[port], chr);
  if(inbyte(serialbase[port]+5) & 0x02)
    return 1; // overrun
  
  return 0;
}

inline void serial_flow_control_recv(unsigned char port)
{
  // XON/XOFF  
  if(serport_buffer[port]->flow_control & 0x01)
  {
    if(!serial_send(port, 0x13))
      serport_buffer[port]->xoff_sent = 1;
  }
  // hardware
  if(serport_buffer[port]->flow_control & 0x02)
  {
      //  activate -DTR and -RTS
      outbyte(serialbase[port]+4, 0x08);
  }
}

void __interrupt serial_int_handler(void)
{
  unsigned char c;
  unsigned char port = 0;
  register unsigned char buffer_free = 0;
  do
  {
    if(!(inbyte(serialbase[port]+2) & 0x01)) // interrupt caused by this port
    {
      c = inbyte(serialbase[port]+5);
      if(c & 0x8C) // error detected
      {
        if(c & 0x04) serport_buffer[port]->error |= SERIAL_PARITY_ERROR;
        if(c & 0x08) serport_buffer[port]->error |= SERIAL_FRAMING_ERROR;
      }
      else if(c & 0x01) // data ready and the interrupt was not caused by break
      {
        c = inbyte(serialbase[port]);
        if(serport_buffer[port]->flow_control & 0x01)
        {
          if(c == 0x13)  // XOFF received
            serport_buffer[port]->xoff_received = 1;
          else if(c == 0x11) // XON received
            serport_buffer[port]->xoff_received = 0;
          else goto receive_char;
        }
        else
        {
          receive_char:
          serport_buffer[port]->buffer[serport_buffer[port]->input_offset++] = c;
          if((buffer_free -= (serport_buffer[port]->input_offset - serport_buffer[port]->output_offset)) < SERIALCOMM_BUFF_CTS_LIMIT)
          {
            if(!buffer_free)
              serport_buffer[port]->error |= SERIAL_BUFFER_FULL;
            else
              serial_flow_control_recv(port);
          }
        }
      }
    }
  }
  while(++port < 4 && serialbase[port]);

  outbyte(0x20, 0x20);
  return;
}
