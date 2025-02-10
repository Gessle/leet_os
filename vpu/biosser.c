#include "serdef.c"

unsigned short __far *serialbase = (unsigned short __far*)0x00400000L;

struct serport_buffer __far *serport_buffer[4] = { 0, 0, 0, 0 };

unsigned char serial_write(unsigned char port, unsigned char chr);

void log_error(char *error);

inline int serial_send(unsigned char port, unsigned char chr)
{
  // wait until bit 5 is set
  while(!(inbyte(serialbase[port]+5) & 0x20));

  outbyte(serialbase[port], chr);
  if(inbyte(serialbase[port]+5) & 0x02)
    return 1; // overrun
  
  return 0;
}

void (__interrupt __far *prev_int_b)(void) = 0;
void (__interrupt __far *prev_int_c)(void) = 0;

unsigned char old_int_mask[2];

void __interrupt serial_int_handler(void);

static void serial_flow_control_read(unsigned char port)
{
  if(serport_buffer[port]->flow_control & 0x01)
  {
    // send XON
    if(!serial_send(port, 0x11))
      serport_buffer[port]->xoff_sent = 0;
  }
  if(serport_buffer[port]->flow_control & 0x02)
  {  
    // RTS/CTS
    //  deactivate -DTR and -RTS
    outbyte(serialbase[port]+4, 0x0B);
  }
}

int serial_read(unsigned char port)
{
  register unsigned char error;
  if(error = serport_buffer[port]->error)
  {
    if(error & SERIAL_PARITY_ERROR)
      log_error("COM Parity error!");
    if(error & SERIAL_FRAMING_ERROR)
      log_error("COM Framing error!");
    if(error & SERIAL_BUFFER_FULL)
      log_error("COM Received byte when buffer full!");          
    return -2;
  }

  if(serport_buffer[port]->xoff_sent
    && (serport_buffer[port]->input_offset - serport_buffer[port]->output_offset) < SERIALCOMM_BUFF_CTS_LIMIT)
    serial_flow_control_read(port);

  // buffer empty
  if(serport_buffer[port]->output_offset == serport_buffer[port]->input_offset)
    return -1;

  return serport_buffer[port]->buffer[serport_buffer[port]->output_offset++];
}

unsigned short baud_divisors[] = 
{
      // baud rates:  
2304, // 0 == 50 baud
1040, // 1 == 110 baud
768,  // 2 == 150 baud
384,  // 3 == 300 baud
192,  // 4 == 600 baud
96,   // 5 == 1200 baud
48,   // 6 == 2400 baud
24,   // 7 == 4800 baud
12,   // 8 == 9600 baud
6,    // 9 == 19200 baud
3,    // 10 == 38400 baud
2,    // 11 == 57600 baud
1     // 12 == 115200 baud
};

/*
  OPEN syscall MODE word HIGH BYTE bits 0-3 == baud rate
                         HIGH BYTE bits 7,6 == flow control
                           (0 = none, 01 = XON/XOFF, 11 = RTS/CTS)

  LOW BYTE
    =
    BIT 0-1: word length: 00 == 5, 01 == 6, 10 == 7, 11 == 8
    BIT 2: stop bits: 0 == 1, 1 == 2
    BIT 3-4: parity: x0 == none, 01 == odd, 11 = even
    BIT 5 == stuck parity
    BIT 6 == enable break control
*/
int serial_init(unsigned char port, unsigned short mode)
{
  unsigned baud_divisor = (mode>>8)&0x0F;

  if(serport_buffer[port])
    return 1;

  if(baud_divisor>=sizeof(baud_divisors))
    return 1;

  serport_buffer[port] = calloc(1, sizeof(struct serport_buffer));
  if(!serport_buffer[port])
  {
    nomemory();
    return 1;
  }

  serport_buffer[port]->flow_control = (unsigned short)(mode>>14);

  if(port&1)
  {
    if(!prev_int_b)
      prev_int_b = _dos_getvect( 0xB ); 
    _dos_setvect(0x0B, serial_int_handler);
    old_int_mask[0] = inbyte(0x21);
    outbyte(0x21, old_int_mask[0] & ~0x08);        
  }
  else
  {
    if(!prev_int_c)
      prev_int_c = _dos_getvect( 0xC ); 
    _dos_setvect(0x0C, serial_int_handler);
    old_int_mask[1] = inbyte(0x21);
    outbyte(0x21, old_int_mask[1] & ~0x10);        
  }

  // enable DLAB
  outbyte(serialbase[port]+3, 0x80);
  // set baud rate
  outbyte(serialbase[port]+0, baud_divisors[baud_divisor]);
  outbyte(serialbase[port]+1, baud_divisors[baud_divisor]>>8);
  // disable DLAB & set data bits, parity and stop bits
  outbyte(serialbase[port]+3, mode&0x007F);
//  // enable FIFO
//  outp(serialbase[port]+2, 0xC7);
//  outp(serialbase[port]+2, 0x07);
  // disable FIFO
  outbyte(serialbase[port]+2, 0x00);
  // set interrupt when receiving  
  outbyte(serialbase[port]+1, 0x01);
  // set RTS/DSR, IRQ enable
  outbyte(serialbase[port]+4, 0x0B);

  inbyte(serialbase[port]);

  return 0;   
}

void close_serialport(unsigned char port)
{
  free(serport_buffer[port]);
  serport_buffer[port] = 0;
  if(port&1 && !serport_buffer[3] && !serport_buffer[1])
  {
    _dos_setvect(0x0B, prev_int_b);
    outbyte(0x21, old_int_mask[0]);
  }
  else if(!serport_buffer[2] && !serport_buffer[0])
  {
    _dos_setvect(0x0C, prev_int_c);
    outbyte(0x21, old_int_mask[1]);
  }
}


