void (__interrupt __far *prev_int_24)(void); 

void log_error(char *);
void show_error_console(void);

/*
Sets the original 0x24 interrupt
*/
void reset_int24(void)
{
  _dos_setvect( 0x24, prev_int_24 );
}

  char errormsg[100];
  char error1[] = "error while";
  char *error2[] = { "reading", "writing" };
  char *error3[] = { "DOS area", "FAT", "root", "file" };

  char *dierror[] = {
  "Write protection", // 0
  "Unknown drive", // 1
  "Drive not ready", // 2
  "Invalid command", // 3
  "Data", // 4
  "Incorrect length of request", // 5
  "Seek", // 6
  "Invalid format", // 7
  "Sector not found", // 8
  "Out of paper", // 9
  "Write", // 10
  "Read", // 11
  "General", // 12
  "Inappropriate disk change" // 13
  };

/*
This is a critical error handler
*/
static void __interrupt int24handler(union INTPACK r)
{
  unsigned char device;
  unsigned int reg_di;
  unsigned char errorbits;
//  union REGS regs;
  __asm
  {
    mov device, al
    mov errorbits, ah
    mov reg_di, di
  }
  reg_di <<= 8;
  reg_di >>= 8;

  if(errorbits & 0x80)
    sprintf(errormsg, "%s %s %s %s.", dierror[reg_di], error1, error2[errorbits&0x01], error3[(errorbits>>1)&0x03]);
  else
    sprintf(errormsg, "%s %s %s %s at device %u.", dierror[reg_di], error1, error2[errorbits&0x01], error3[(errorbits>>1)&0x03], device);  
  
//  dialog(20, 20, 350, 50, 0, 0, 1, errormsg, errortitle, 0);
  log_error(errormsg);
  show_error_console();

  r.h.al = 0x03;
  
}

/*
Sets the interrupt 0x24
*/
void set_int24(void)
{
  prev_int_24 = _dos_getvect( 0x24 ); 
  _dos_setvect(0x24, int24handler);
}
