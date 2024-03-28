#include <dos.h>
#include <stdlib.h>
#include <string.h>

#include "..\extdlc.c"

//#include "drvdef.c"
#include "drvstrc.c"

extern void __far (*put_pixel_screen)(unsigned int x, unsigned int y, unsigned color);
extern unsigned __far (*get_pixel_screen)(unsigned int x, unsigned int y);
extern unsigned char __far (*draw_rectangle_screen)(int x0, int y0, unsigned int width, unsigned int height, unsigned color);
extern unsigned char __far (*fill_screen_color)(unsigned color);
extern unsigned char _FONT_HEIGHT;
extern void __far (*update_screen)(void);
extern void __far (*driver_videomode)(void);
extern void __far (*video_memcpy)(void);


void video_mode(unsigned char mode);
extern unsigned driver_count;
extern struct driver *drivers;
extern int videodriver;
extern unsigned _RES_X;
extern unsigned _RES_Y;
extern unsigned char _VIDEO_MODE;
extern unsigned char video;

inline unsigned allocate_new_driver(void)
{
  void *new_pointer;  
  if(!driver_count++)
  {
    // allocate driver table
    drivers = calloc(driver_count, sizeof(struct driver));
    if(!drivers)
    {
      nomemory:
      --driver_count;
      return 0;
    }
  }
  else
  {
    new_pointer = realloc(drivers, driver_count*sizeof(struct driver));
    if(!new_pointer) goto nomemory;
    else drivers = new_pointer;
    memset(&drivers[driver_count-1], 0, sizeof(struct driver));
  }
  return driver_count;
}


inline void install_display_driver(void __far * __far *sfunc_table)
{
  put_pixel_screen = sfunc_table[0];
  get_pixel_screen = sfunc_table[1];
  draw_rectangle_screen = sfunc_table[2];
  fill_screen_color = sfunc_table[3];
  update_screen = sfunc_table[4];
  video_memcpy = sfunc_table[5];

  driver_videomode = sfunc_table[6];  
  
  _RES_X = (unsigned short)sfunc_table[7];
  _RES_Y = ((unsigned long)sfunc_table[7]) >> 16;

  video = _VIDEO_MODE = ((unsigned long)sfunc_table[8]) >> 16;
  _FONT_HEIGHT = *(unsigned char*)&sfunc_table[8];
}

/* This interrupt handler is called by the resident program before exiting.

   AX:BX = segment and offset where the function table is
   CX = device type (0 == normal device, 1 == graphics adapter)

   -1 is returned to AX if an error occurred.

   If driver was loaded successfully, 0 is returned.
*/
void __interrupt driver_installer(union INTPACK regs)
{
  unsigned pfunc_table_seg = regs.w.ax;
  unsigned pfunc_table_offset = regs.w.bx;
  unsigned device_type = regs.w.cx;
  unsigned sfunc_table_seg = regs.w.dx;
  unsigned sfunc_table_offset = regs.w.si;
  void __far * __far *pfunc_table = MK_FP(pfunc_table_seg, pfunc_table_offset);
  void __far * __far *sfunc_table = MK_FP(sfunc_table_seg, sfunc_table_offset);  
  unsigned di = driver_count;
  int (__far *__far *dfunc_table[9])();
  unsigned n = 0;

  if(!allocate_new_driver())
  {
    video_mode(0x03);    
    regs.w.ax = -1;
    return;
  }

  dfunc_table[0] = &drivers[di].cmd_func;
  dfunc_table[1] = &drivers[di].read_func;  
  dfunc_table[2] = &drivers[di].write_func;
  dfunc_table[3] = &drivers[di].open_func;
  dfunc_table[4] = &drivers[di].close_func;
  dfunc_table[5] = &drivers[di].lseek_func;
  dfunc_table[6] = &drivers[di].fsync_func;
  dfunc_table[7] = &drivers[di].error_func;
  dfunc_table[8] = &drivers[di].unload_func;
  
  do
  {
    *dfunc_table[n] = pfunc_table[n];
    n++;
  }
  while(pfunc_table[n] != (void __far *)-1);
  
  drivers[di].cmd_func_arglen = *(unsigned short*)&pfunc_table[++n];
  drivers[di].dev_count = *(unsigned short*)&pfunc_table[++n];  

//  printf("%p %p %p\n", pfunc_table[2], *dfunc_table[2], drivers[di].write_func);
  if(device_type == 1)
  {
    install_display_driver(sfunc_table);
    videodriver = driver_count;
  }

  regs.w.ax = 0;
}

