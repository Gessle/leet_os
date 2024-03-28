unsigned short __far *serialbase = (unsigned short __far*)0x00400000L;

void main()
{
  unsigned char retries = 0;
  unsigned char port = 0;
  unsigned char c;

  outp(serialbase[port]+1, 0x00);
//  outp(serialbase[port]+3, 0x80);
//  outp(serialbase[port]+0, 24);
//  outp(serialbase[port]+1, 0x00);
//  outp(serialbase[port]+3, 0x07);
  outp(serialbase[port]+2, 0xC7);
  outp(serialbase[port]+4, 0x0B);
  

  retry:
  // wait until bit 1 is set
  if(inp(serialbase[port]+5) & 0x01)
  {
//    puts("Luetaan tavu...");
    c = inp(serialbase[port]);
    // check for errors
    putchar(c);
  }
  goto retry;
}
