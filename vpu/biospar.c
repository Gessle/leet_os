#define LPT_PULSE_DELAY 1

void shortdelay(unsigned long interval);

unsigned short __far *parallelbase = (unsigned short __far*)0x00400008L;

char parallel_init(unsigned short port, unsigned int mode)
{
  unsigned short control_port = parallelbase[port]+2;

  if(!parallelbase[port])
    return 1;

    // disable interrupts
    outbyte(control_port, inbyte(control_port) & ~0x10);    
  
    // reset the device and select the device and set direction to write    
    outbyte(control_port, inbyte(control_port) & ~(0x04 | 0x20));
    delay(50);
    outbyte(control_port, inbyte(control_port) | 0x04 | 0x08);

//    if mode is !0, switch auto line feed on (BIOS call does not do this!!)
    if(mode & 0x00FF)
      outbyte(control_port, inbyte(control_port) | 0x02);

    delay(1000);

  return 0;
 /* __asm
  {
    mov ah, 0x01
    mov dx, port
    int 21
  }*/
}

unsigned char parallel_write(unsigned short port, unsigned char chr)
{
  unsigned char retval;
  unsigned short control_port = parallelbase[port]+2;
  unsigned short printer_port = parallelbase[port];
  unsigned short status_port = parallelbase[port]+1;
  unsigned short retries = 0;


  if(inbyte(status_port) & 0x20)
    return 1;

  outbyte(printer_port, chr);

  while(!(inbyte(status_port) & 0x80) && ++retries)
    shortdelay(LPT_PULSE_DELAY);

  retries = 0;

  shortdelay(LPT_PULSE_DELAY);

  outbyte(control_port, 0x0D);

  shortdelay(LPT_PULSE_DELAY);

  outbyte(control_port, 0x0C);

  inbyte(status_port);

  while(!(inbyte(status_port) & 0x80) && ++retries)
    shortdelay(LPT_PULSE_DELAY);

  if(!(inbyte(status_port) & 0x08))
    return 2;

  return 0;
  

// this works if the BIOS does not  have bugs

/*  __asm
  {
    mov ah, 0x02
    mov dx, port
    int 0x17
    test ah, 0x20
    jne outofpaper
    
    mov ah, 0x00
    mov al, chr
    int 0x17

    test ah, 00001001b
    jne error
    mov retval, 0
    jmp end

    error:
    mov retval, 2
    jmp end
    outofpaper:
    mov retval, 1
    end:
  }
  return retval;

*/

}

/*
// todo: implement this
inline int parallel_read(unsigned char port)
{
/*  unsigned short parport = parallelbase[port];
  unsigned short status_port = parallelbase[port]+1;
  unsigned short control_port = parallelbase[port]+2;
  unsigned char c;
  unsigned char waits = 0;

  // set the BUSY bit to 1
  outp(control_port, inp(control_port)  | 0x80);    

  // wait until strobe bit is set
  while(!(inp(status_port) & 0x01))
  {
    delay(1);
    waits++;
    if(!waits) return -1;
  }
  waits = 0;
  // wait until strobe bit is not set
  while((inp(status_port) & 0x01))
  {
    delay(1);
    waits++;
    if(!waits) return -1;
  }  

  // set the busy bit to 0
  outp(control_port, inp(control_port) & ~0x80);  

  // set the read bit (bit 5)
  outp(control_port, inp(control_port) | 0x20);
  // read char
  c = inp(parport);
  // set the read bit to 0
  outp(control_port, inp(control_port) & (~0x20));

  // set ACK bit to 1
  outp(control_port, inp(control_port) | 0x40);
  delay(LPT_PULSE_DELAY);
  // and back to 0
  outp(control_port, inp(control_port) & ~0x40);
  // set the BUSY bit back to 1
  outp(control_port, inp(control_port)  | 0x80);    
  
  return c;*//*
  return -2;
}*/
