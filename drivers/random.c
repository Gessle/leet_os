/*
  UNIX-type RANDOM device driver

  Works as a "device" that can be written and read from.

  Outputs random characters and takes input as a seed.

  This can be used as an example driver.
*/

#include <stdlib.h>
#include <stdio.h>
#include <i86.h>
#include <dos.h>

#define MIN_PARAGRAPHS 0x11

unsigned __based ( __segname ("_CODE" ) ) random_inc_0;
unsigned __based ( __segname ("_CODE" ) ) random_inc_1;
unsigned __based ( __segname ("_CODE" ) ) random_inc_2;
unsigned __based ( __segname ("_CODE" ) ) random_seed;

#pragma aux random_read value [ax] parm [dx bx] [cx];
unsigned int __far random_read(char __far *buff, unsigned count)
{  
  unsigned n = count;
  while(n--)
  {
    ++random_inc_0;
    random_inc_1+=random_inc_0+11;
    random_inc_2-=random_inc_1-19;
    *buff++ = 0xFF & (random_inc_0+random_inc_1+random_inc_2 + (random_seed = 23 * random_seed % 26479));
  }
  return count;
}

// Read function: Fill the buffer with zeroes.
//#pragma aux random_read value [ax];
//int __far random_read(void);

// Write function: seed.
#pragma aux random_write parm [dx bx] [cx] value [ax];
int __far random_write(char __far *buff, unsigned count);

// Close function: Just return 0. RANDOM stream is now "closed."
#pragma aux random_close value [ax];
int __far random_close(void);

/*unsigned get_code_segment(void);
#pragma aux get_code_segment = \
  "mov ax, cs" \
  value [ax];*/

// Main function: Install the driver. Above functions are kept in memory.
int main(int argc, char **argv)
{
  // Interrupt to use when calling the driver installer function
  unsigned char driver_install_int;
  // -1-terminating table of functions that this driver provides.
  int __far (__far *function_table[])() =
  {
    0, //                cmd function. Cannot give commands to RANDOM device
    (void __far *)random_read, //        read function
    random_write, //       write function
    random_close, //       open function (in this module it's the same as close function)
    random_close,    
    (void __far *)-1, // end of function table    
    0, // length of command function argument array.
    (void __far *)-1 // count of devices. This driver does not support multiple devices    
  };
  union REGS regs;
  
  if(argc != 2)
    return 1;

  regs.w.ax = ((unsigned long)function_table) >> 16; // segment of the pointer to the table
  regs.w.bx = (unsigned short)function_table; // offset
  regs.w.cx = 0; // driver type = 0 (no display driver or any special driver)

  driver_install_int = strtoul(argv[1], 0, 0x10); // Interrupt vector that points to the driver installer function
  int86(driver_install_int, &regs, &regs); // Call the driver installer
  if(regs.w.ax) // if AX != 0, some error happened
  {
    puts("Error while loading RANDOM module. Press any key");
    getchar();
  }
  else
  {
    if(((_FP_OFF( main ) + 0xF) >> 4) + 0x10 < MIN_PARAGRAPHS) // add 0x10 paragraphs for PSP
      _dos_keep(0, MIN_PARAGRAPHS ); // it's recommended to allocate at least 0x12 paragraphs because of bugs in MS-DOS and Windows NT
    _dos_keep(0, ((_FP_OFF( main ) + 0xF) >> 4) + 0x10 ); // exit and leave functions before main() in memory    
  }
  return 1;
}
