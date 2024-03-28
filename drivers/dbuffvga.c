/*
  Double-buffered VGA driver
*/

#include <stdlib.h>
#include <stdio.h>
#include <i86.h>
#include <dos.h>
#include <malloc.h>
#include <conio.h>
#include <string.h>

//#define MIN_PARAGRAPHS 0x11

unsigned char __far * __based ( __segname ("_CODE" ) ) plane[4];
unsigned char __far * __based ( __segname ("_CODE" ) ) VGA = (unsigned char __far *)0xA0000000L;

void __far *tsr_alloc(unsigned size);
#pragma aux tsr_alloc = \
  "add bx, 0xF" \
  "mov cl, 4" \
  "shr bx, cl" \
  "mov ah, 0x48" \
  "int 0x21" \
  "jnc success" \
  "xor ax, ax" \
  "xor cx, cx" \
  "jmp fail" \
  "success:" \
  "mov cx, ax" \
  "dec cx" \
  "mov ax, 0x10" \
  "fail:" \
  value [cx ax] parm [bx];

void tsr_free_pragma(unsigned segment);
#pragma aux tsr_free_pragma = \
  "push ax" \
  "mov ah, 0x49" \
  "int 0x21" \
  "pop ax" \
  parm [es];

void tsr_free(void __far *ptr)
{
  tsr_free_pragma(FP_SEG(ptr) + 1);
}


void out(unsigned port, unsigned char byte);  
#pragma aux out = \
  "out dx, al" \
  parm [dx] [al];

inline void select_plane(unsigned plane)
{
  out(0x3C4, 0x02);
  out(0x3C5, 0x01 << plane);
}

void setmem(void __far *dst, unsigned value, unsigned wordcount);
#pragma aux setmem = \
  "pushf" \
  "cld" \
  "rep stosw" \
  "popf" \
  parm [es di] [ax] [cx];

void setmemb(void __far *dst, unsigned value, unsigned bytecount);
#pragma aux setmemb = \
  "pushf" \
  "cld" \
  "rep stosb" \
  "popf" \
  parm [es di] [ax] [cx];    


void copymem(void __far *dst, void __far *src, unsigned wordcount);
#pragma aux copymem = \
  "pushf" \
  "cld" \
  "rep movsw" \
  "popf" \
  parm [es di] [ds si] [cx];  

void __far update_screen(void)
{
  unsigned i = 4;

  out(0x3CE, 0x5);
  out(0x3CF, 0x0);
  out(0x3CE, 0x08);
  out(0x3CF, 0xFF);
  

  while(i--)
  {
    select_plane(i);
    copymem(VGA, plane[i], 0x9600>>1);
  }
}

//void __far update_screen(void);

#pragma aux put_pixel parm [ax] [bx] [cx];
void __far put_pixel(unsigned x, unsigned y, unsigned color)
{
  unsigned long offset = y*80+(x>>3);
  unsigned char set_bit = 0x80 >> (x&7);

  if(x >= 640 || y >= 480) return;

  if(color & 0x08) plane[3][offset] |= set_bit;
  else plane[3][offset] &= ~set_bit;
  if(color & 0x04) plane[2][offset] |= set_bit;
  else plane[2][offset] &= ~set_bit;
  if(color & 0x02) plane[1][offset] |= set_bit;
  else plane[1][offset] &= ~set_bit;
  if(color & 0x01) plane[0][offset] |= set_bit;
  else plane[0][offset] &= ~set_bit;

}

//#pragma aux put_pixel parm [ax] [bx] [cx];
//void __far put_pixel(unsigned x, unsigned y, unsigned color);

#pragma aux get_pixel parm [ax] [bx] value [ax];
unsigned __far get_pixel(unsigned x, unsigned y)
{
  unsigned offset = y*80+(x>>3);
  unsigned char set_bit = 0x80 >> (x&7);
  unsigned retval = 0;

  if(plane[0][offset] & set_bit) retval |= 0x01;
  if(plane[1][offset] & set_bit) retval |= 0x02;
  if(plane[2][offset] & set_bit) retval |= 0x04;
  if(plane[3][offset] & set_bit) retval |= 0x08;    

  return retval;
}

#pragma aux vmemcpy parm [ax] [bx] [cx] [dx] [si] [di];
void __far vmemcpy(unsigned x, unsigned y, unsigned width, unsigned height, int dx, int dy)
{
  unsigned n;
  unsigned nx;
  for(n=0;n<height;n++)
    for(nx=0;nx<width;nx++)
      put_pixel(x+dx+nx, y+dy+n, get_pixel(x+nx, y+n));
}

//#pragma aux get_pixel parm [ax] [bx] value [ax];
//unsigned __far get_pixel(unsigned x, unsigned y);

//#pragma aux draw_rectangle parm [ax] [bx] [cx] [dx] [si];
//void __far draw_rectangle(unsigned x, unsigned y, unsigned width, unsigned height, unsigned color);

#pragma aux draw_rectangle parm [ax] [bx] [cx] [dx] [si];
void __far draw_rectangle(unsigned x0, unsigned y0, unsigned width, unsigned height, unsigned color)
{
/*  unsigned int n;  
  while(height--)
  {
    for(n=width;n--;)
      put_pixel_rtn(x+n, y+height, color);   
  }*/
  unsigned int m = height;
  unsigned int l = (((x0+width)-((x0+width)&7)) - (x0)>>3);
  unsigned int n;
  unsigned char __far *vgap;
  unsigned char set_bits;
  int sc = x0;
  unsigned i;
  unsigned char __far *planep[4];

  if(x0&7)
  {
    set_bits = 0xFF>>(x0&7);
    while(m--)
    {
      if(color & 0x08)
        *(plane[3] + (y0)*80+(x0>>3)) |= set_bits;
      else
        *(plane[3] + (y0)*80+(x0>>3)) &= ~set_bits;
      if(color & 0x04)
        *(plane[2] + (y0)*80+(x0>>3)) |= set_bits;
      else
        *(plane[2] + (y0)*80+(x0>>3)) &= ~set_bits;
      if(color & 0x02)
        *(plane[1] + (y0)*80+(x0>>3)) |= set_bits;
      else
        *(plane[1] + (y0)*80+(x0>>3)) &= ~set_bits;        
      if(color & 0x01)
        *(plane[0] + (y0++)*80+(x0>>3)) |= set_bits;
      else
        *(plane[0] + (y0++)*80+(x0>>3)) &= ~set_bits;                
    }
    m = height;
    y0-=height;
    sc+=8;
  }

  while(m--)
  {
    planep[0] = (plane[0] + (y0)*80+(sc>>3));//cgapointer(sc, y0++);
    planep[1] = (plane[1] + (y0)*80+(sc>>3));
    planep[2] = (plane[2] + (y0)*80+(sc>>3));
    planep[3] = (plane[3] + (y0++)*80+(sc>>3));        

    if(color & 0x01)
      setmemb(planep[0], 0xFF, l);
    else
      setmemb(planep[0], 0x00, l);
    if(color & 0x02)
      setmemb(planep[1], 0xFF, l);
    else
      setmemb(planep[1], 0x00, l);
    if(color & 0x04)
      setmemb(planep[2], 0xFF, l);
    else
      setmemb(planep[2], 0x00, l);
    if(color & 0x08)
      setmemb(planep[3], 0xFF, l);
    else
      setmemb(planep[3], 0x00, l);      

      
  }

  if((x0+width)&7)
  {
    set_bits = ~(0xFF>>((x0+width)&7));
    m=height;
    y0--;
    sc = (x0+width);
    while(m--)
    {
      if(color & 0x08)
        *(plane[3] + (y0)*80+(sc>>3)) |= set_bits;
      else
        *(plane[3] + (y0)*80+(sc>>3)) &= ~set_bits;
      if(color & 0x04)
        *(plane[2] + (y0)*80+(sc>>3)) |= set_bits;
      else
        *(plane[2] + (y0)*80+(sc>>3)) &= ~set_bits;
      if(color & 0x02)
        *(plane[1] + (y0)*80+(sc>>3)) |= set_bits;
      else
        *(plane[1] + (y0)*80+(sc>>3)) &= ~set_bits;        
      if(color & 0x01)
        *(plane[0] + (y0--)*80+(sc>>3)) |= set_bits;
      else
        *(plane[0] + (y0--)*80+(sc>>3)) &= ~set_bits;                
    }
  }    
}

//#pragma aux fill_screen parm [ax];
//void __far fill_screen(unsigned color);

#pragma aux fill_screen parm [ax];
void __far fill_screen(unsigned color)
{
  if(color & 0x08) setmem(plane[3], 0xFFFF, 0x9600>>1);
  else setmem(plane[3], 0x0000, 0x9600>>1);

  if(color & 0x04) setmem(plane[2], 0xFFFF, 0x9600>>1);
  else setmem(plane[2], 0x0000, 0x9600>>1);

  if(color & 0x02) setmem(plane[1], 0xFFFF, 0x9600>>1);
  else setmem(plane[1], 0x0000, 0x9600>>1);

  if(color & 0x01) setmem(plane[0], 0xFFFF, 0x9600>>1);
  else setmem(plane[0], 0x0000, 0x9600>>1);
}

void __far set_mode(void);

// Main function: Install the driver. Above functions are kept in memory.
int main(int argc, char **argv)
{
  // Interrupt to use when calling the driver installer function
  unsigned char driver_install_int;
  // -1-terminating table of functions that this driver provides.
  int __far (__far *function_table[])() =
  {
    0, //                cmd function. Cannot give commands to VGA device
    (void __far *)-1, // end of function table    
    0, // length of command function argument array.
    (void __far *)-1 // count of devices. This driver does not support multiple devices    
  };
  void __far (__far *screenfunc_table[])() =
  {
    put_pixel,
    (void __far *)get_pixel,
    draw_rectangle,
    fill_screen,
    update_screen,
    vmemcpy, // video memcpy function
    set_mode,
    (void __far *)(640 | ((unsigned long)480 << 16)),
    (void __far *)(((unsigned long)0xFF << 16) | 8)
  };
  union REGS regs;
  
  if(argc != 2)
    return 1;

  plane[0] = tsr_alloc(0x9600* sizeof(char));
  plane[1] = tsr_alloc(0x9600* sizeof(char));
  plane[2] = tsr_alloc(0x9600* sizeof(char));
  plane[3] = tsr_alloc(0x9600* sizeof(char));  
  if(!plane[0] || !plane[1] || !plane[2] || !plane[3])
  {
    tsr_free(plane[0]);
    tsr_free(plane[1]);
    tsr_free(plane[2]);
    tsr_free(plane[3]);    
    return 1;
  }

  regs.w.ax = ((unsigned long)function_table) >> 16; // segment of the pointer to the table
  regs.w.bx = (unsigned short)function_table; // offset
  regs.w.cx = 1; // driver type = 1 (display driver)
  regs.w.dx = ((unsigned long)screenfunc_table) >> 16;
  regs.w.si = (unsigned short)screenfunc_table;

  driver_install_int = strtoul(argv[1], 0, 0x10); // Interrupt vector that points to the driver installer function
  int86(driver_install_int, &regs, &regs); // Call the driver installer
  if(regs.w.ax) // if AX != 0, some error happened
  {
    puts("Error while loading DOUBLEBUFFERED VGA module. Press any key");
    getchar();
  }
  else
  {
    set_mode();
/*    if(((_FP_OFF( main ) + 0xF) >> 4) + 0x10 < MIN_PARAGRAPHS) // add 0x10 paragraphs for PSP
      _dos_keep(0, MIN_PARAGRAPHS ); // it's recommended to allocate at least 0x12 paragraphs because of bugs in MS-DOS and Windows NT*/
    _dos_keep(0, ((_FP_OFF( main ) + 0xF) >> 4) + 0x10 ); // exit and leave functions before main() in memory    
  }
  return 1;
}
