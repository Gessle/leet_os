/*
  UNIX-type NULL device driver

  Works as a "device" that can be written and read from.

  Outputs only null characters and disposes everything what is written to it.

  This can be used as an example driver. This is probably the simplest driver possible.
*/

#include <stdlib.h>
#include <stdio.h>
#include <i86.h>
#include <dos.h>

#define MIN_PARAGRAPHS 0x11

// Read function: Fill the buffer with zeroes.
#pragma aux null_read value [ax] parm [dx bx] [cx];
int __far null_read(char __far *buff, unsigned count)
{
  unsigned n = count;
  while(n--)
    *buff++ = 0;

  return count;
}

// Write function: Just return the count.
#pragma aux null_write value [ax] parm [dx bx] [cx];
int __far null_write(char __far *buff, unsigned count)
{
  return count;
}

// Open function: Just return 0. NULL stream is now "open".
#pragma aux null_open parm [ax] value [ax];
int __far null_open(unsigned mode)
{
  return 0;
}

// Close function: Just return 0. NULL stream is now "closed."
#pragma aux null_close value [ax];
int __far null_close(void)
{
  return 0;
}

// Main function: Install the driver. Above functions are kept in memory.
int main(int argc, char **argv)
{
  // Interrupt to use when calling the driver installer function
  unsigned char driver_install_int;
  // -1-terminating table of functions that this driver provides.
  const int __far (__far *function_table[])() =
  {
    0, //                cmd function. Cannot give commands to NULL device
    null_read, //        read function
    null_write, //       write function
    null_open, // ...
    null_close,    
    (void __far *)-1, // end of function table    
    0, // length of command function argument array.
    (void __far *)-1 // count of devices. This driver does not support multiple devices    
  };
  union REGS regs;

//  printf("%p %p\n", null_write, function_table[2]);
  if(argc != 2)
    return 1;

  regs.w.ax = ((unsigned long)function_table) >> 16; // segment of the pointer to the table
  regs.w.bx = (unsigned short)function_table; // offset
  regs.w.cx = 0; // driver type = 0 (no display driver or any special driver)

  driver_install_int = strtoul(argv[1], 0, 0x10); // Interrupt vector that points to the driver installer function
  int86(driver_install_int, &regs, &regs); // Call the driver installer
  if(regs.w.ax) // if AX != 0, some error happened
  {
    puts("Error while loading NULL module. Press any key");
    getchar();
  }
  else
  {
//    _dos_keep(0, (_FP_OFF( main ) + 0xF) >> 4 ); // exit and leave functions before main() in memory
    if(((_FP_OFF( main ) + 0xF) >> 4) + 0x10 < MIN_PARAGRAPHS) // add 0x10 paragraphs for PSP
      _dos_keep(0, MIN_PARAGRAPHS ); // it's recommended to allocate at least 0x11 paragraphs because of bugs in MS-DOS and Windows NT
    _dos_keep(0, ((_FP_OFF( main ) + 0xF) >> 4 ) + 0x10 ); // exit and leave functions before main() in memory    
  }
  return 1;
}
