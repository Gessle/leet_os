/*
  Optimized VGA driver
*/

#include <stdlib.h>
#include <stdio.h>
#include <i86.h>
#include <dos.h>
#include <malloc.h>
#include <conio.h>

#define _RES_X 640
#define _RES_Y 480

unsigned char __far * __based ( __segname ("_CODE" ) ) VGA = (unsigned char __far *)0xA0000000L;
unsigned char __based(__segname("_CODE")) ega4_lastoperation = -1;

unsigned char __far* __based(__segname("_CODE")) lastvgaptr;
unsigned char __based(__segname("_CODE")) lastcolor = -1;
unsigned char __based(__segname("_CODE")) lastbit;

//unsigned char __far * __based(__segname("_CODE")) plane_buff;
#define PIXEL_BUFF_SIZE _RES_X
unsigned char __based(__segname("_CODE")) plane_buff[PIXEL_BUFF_SIZE>>1];
#define plane_ptr(i) ((unsigned char __based(__segname("_CODE"))*)(&plane_buff[i*(PIXEL_BUFF_SIZE>>3)]))
unsigned __based(__segname("_CODE")) adj_pixels = 0;
unsigned long __based(__segname("_CODE")) first_adj_px = -1;

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

void dummy_read(unsigned char byte);
#pragma aux dummy_read = \
  parm [al];

void memset(void __far *dst, unsigned value, unsigned count);
#pragma aux memset = \
  "cld" \
  "test cl, 0x01" \
  "jz set_words" \
  "dec cx" \
  "add di, cx" \
  "mov byte ptr es:[di], al" \
  "sub di, cx" \
  "set_words:" \
  "shr cx, 1" \
  "mov ah, al" \
  "rep stosw" \
  parm [es di] [ax] [cx];

void memcpy(void __far *dst, void __far *src, unsigned count);
#pragma aux memcpy = \
  "cld" \
  "rep movsb" \
  parm [es di] [ds si] [cx];

void wmemcpy_pragma(void __far *dst, void __far *src, unsigned count);
#pragma aux wmemcpy_pragma = \
  "cld" \
  "shr cx, 1" \
  "rep movsw" \
  parm [es di] [ds si] [cx];

void wmemcpy(char __far *dst, char __far *src, unsigned count)
{
  if(count & 1)
  {
    count--;
    dst[count] = src[count];
  }
  wmemcpy_pragma(dst, src, count);
}



void out(unsigned port, unsigned char byte);  
#pragma aux out = \
  "out dx, al" \
  parm [dx] [al];

void wordout(unsigned port, unsigned word);
#pragma aux wordout = \
  "out dx, ax" \
  parm [dx] [ax];

inline unsigned mul_80(unsigned num)
{
  return (num<<6)+(num<<4);
}

inline unsigned long pixel_linear_pos(unsigned x, unsigned y)
{
  return ((unsigned long)y<<9) + (y<<7) + x;
}

void update_planes(void)
{
  unsigned vgaoffset = first_adj_px >> 3;
  unsigned char __far *vgaptr = &VGA[vgaoffset];
  unsigned char copy_count = adj_pixels >> 3;
  unsigned i = 4;

//  out(0x3CE, 0x5);
//  out(0x3CF, 0x0);
  wordout(0x3CE, 0x0005);
//  out(0x3CE, 0x08);
//  out(0x3CF, 0xFF);
  wordout(0x3CE, 0xFF08);

  out(0x3C4, 0x02);
  while(i--)
  {
    out(0x3C5, 0x01 << i);
    wmemcpy(vgaptr, plane_ptr(i), copy_count);
    plane_ptr(i)[0] = plane_ptr(i)[copy_count];
  }
  out(0x3C5, 0x0F);
  lastvgaptr += copy_count;
  adj_pixels &= 7;
  ega4_lastoperation = -1;
}

void save_pixel_to_plane(unsigned offset, unsigned char color)
{
  unsigned char plane_offset = offset >> 3;
  unsigned char set_bit = 0x80 >> (offset & 7);
  unsigned i = 4;

  while(i--)
  {
    if((color >> i) & 1)
      plane_ptr(i)[plane_offset] |= set_bit;
    else
      plane_ptr(i)[plane_offset] &= ~set_bit;
  }
}

void flush_pixel_cache(void)
{
  if(lastcolor == 0xFF) return;
  if(ega4_lastoperation)
  {
//    out(0x03CE, 0x05);
//    out(0x03CF, 0x02);
    wordout(0x03CE, 0x0205);

//    out(0x03CE, 0x08);
    wordout(0x03CE, (lastbit << 8) | 0x08);
    ega4_lastoperation=0;    
  }
  else
    out(0x03CF, lastbit);

  dummy_read(*lastvgaptr);

  *lastvgaptr = lastcolor;

  lastcolor = -1;
  lastbit = 0;
}

void flush_adj_pixels(void)
{
  unsigned p;
  lastvgaptr = &VGA[first_adj_px>>3];
  if(adj_pixels < 8)
  {
    ind_pixels:while(adj_pixels)
    {
      adj_pixels--;
      lastcolor = 0;
      for(p=4;p--;)
        lastcolor |= (!!((plane_ptr(p)[0] << adj_pixels) & 0x80)) << p;
      lastbit = 0x80 >> adj_pixels;
      flush_pixel_cache();
    }
    lastcolor = -1;
    lastbit = 0;
  }
  else
  {
    update_planes();
    goto ind_pixels;
  }
}

void __far update_screen(void)
{
  if(adj_pixels)
    flush_adj_pixels();
  else
    flush_pixel_cache();
}

#pragma aux put_pixel parm [ax] [bx] [cx];
void __far put_pixel(unsigned x, unsigned y, unsigned color)
{
  unsigned char __far *dptr = &VGA[mul_80(y)+(x>>3)];
  unsigned char set_bit = 0x80 >> (x&7);

  if(adj_pixels)
    if(pixel_linear_pos(x, y) == first_adj_px + adj_pixels)
      goto add_adj_px;
    else
      flush_adj_pixels();
  if(lastcolor == color && dptr == lastvgaptr)
  {
    lastbit |= set_bit;
  }
  else if(!(x&0xF))
  {
    if(!adj_pixels)
    {
      flush_pixel_cache();
      first_adj_px = pixel_linear_pos(x, y);
    }
    else add_adj_px:if(adj_pixels == PIXEL_BUFF_SIZE)
    {
      flush_adj_pixels();
      first_adj_px = pixel_linear_pos(x, y);
    }
    save_pixel_to_plane(adj_pixels++, color);
  }
  else
  {
    flush_pixel_cache();
    lastvgaptr = dptr;
    lastbit = set_bit;
    lastcolor = color;
  }
}

#pragma aux get_pixel parm [ax] [bx] value [ax];
unsigned __far get_pixel(unsigned x, unsigned y)
{
  unsigned vga_off = mul_80(y) + (x >> 3);
  unsigned char __far *dptr = &VGA[vga_off];
  unsigned char set_bit = 0x80 >> (x&7);
  unsigned char retval;
  register unsigned int n = 0x04;

  if(lastcolor != 0xFF && dptr == lastvgaptr && (lastbit & set_bit));
  else if((unsigned long)vga_off - (first_adj_px>>3) < adj_pixels)
  {
    vga_off -= first_adj_px>>3;
    retval = 0;
    while(n--)
    {
      if(plane_ptr(n)[vga_off] & set_bit)
        retval |= 0x01 << n;
    }
    return retval;
  }
  else
  {
    update_screen();
    if(ega4_lastoperation != 1)
    {
      out(0x03CE, 0x04);
      ega4_lastoperation=1;    
    }
    retval = 0;
    while(n--)
    {
      out(0x03CF, n);
      retval <<=1;        
      if((*dptr)&set_bit) retval |= 1;
    }
    if(retval == lastcolor && dptr == lastvgaptr)
      lastbit |= set_bit;
    else
    {
      flush_pixel_cache();
      lastvgaptr = dptr;
      lastbit = set_bit;
      lastcolor = retval;
    }
  }

  return lastcolor;

}

#pragma aux vmemcpy parm [ax] [bx] [cx] [dx] [si] [di];
void __far vmemcpy(unsigned x, unsigned y, unsigned width, unsigned height, int dx, int dy)
{
  unsigned char __far *dstptr = &VGA[mul_80(y+dy)+((x+dx)>>3)];
  unsigned char __far *srcptr = &VGA[mul_80(y)+(x>>3)];
  unsigned n;
  unsigned nx;

  if(x & 7)
  {
    for(n=0;n<height;n++)
    {
      for(nx=0;(x+nx) & 7;nx++)
        put_pixel(x+dx+nx, y+dy+n, get_pixel(x+nx, y+n));
    }
    dstptr++;
    srcptr++;
    width -= nx;
  }
  else nx = 0;
  if(width & 7)
  {
    x += nx;
    for(n=0;n<height;n++)
    {
      for(nx=width&7;nx--;)
        put_pixel(x+dx+(width&~7)+nx, y+dy+n, get_pixel(x+(width&~7)+nx, y+n));
    }
  }

  update_screen();
  if(ega4_lastoperation)
  {
//    out(0x03CE, 0x05);
//    out(0x03CF, 0x02);
    wordout(0x03CE, 0x0205);
//    out(0x03CE, 0x08);
    wordout(0x03CE, 0x0008);
    ega4_lastoperation = 0;
  }
  else
    out(0x03CF, 0x00); 
  width >>= 3;
  while(height--)
  {
    memcpy(dstptr, srcptr, width);
    dstptr += 80;
    srcptr += 80;
  }
}

#pragma aux draw_rectangle parm [ax] [bx] [cx] [dx] [si];
void __far draw_rectangle(unsigned x0, unsigned y0, unsigned width, unsigned height, unsigned color)
{
  register unsigned int m = height;
  unsigned int l = (((x0+width)-((x0+width)&7)) - (x0))>>3;
  unsigned char __far *vgap = VGA + mul_80(y0) + (x0>>3);

  update_screen();

  if(ega4_lastoperation)
  {
//    out(0x03CE, 0x05);
//    out(0x03CF, 0x02);
    wordout(0x03CE, 0x0205);

//    out(0x03CE, 0x08);
    wordout(0x03CE, ((0xFF>>(x0&7)) << 8) | 0x08);
    ega4_lastoperation=0;    
  }
  else
    out(0x03CF, 0xFF>>(x0&7));

  // left side
  if(x0&7)
  {
//    out(0x03CF, 0xFF>>(x0&7));
    while(m--)
    {
      dummy_read(*vgap);
      *vgap = color;
      vgap += _RES_X>>3;
    }
    vgap = VGA + mul_80(y0) + (x0>>3) + 1;
    m = height;
  }
  // center

  out(0x03CF, 0xFF);
  while(m--)
  {
    memset(vgap, color, l);
    vgap += (_RES_X>>3);
  }
  
  // right side
  if((x0+width)&7)
  {
    out(0x03CF, ~(0xFF>>((x0+width)&7)));
    vgap = VGA + mul_80(y0) + ((x0+width)>>3);
    for(m=height;m--;)
    {
      dummy_read(*vgap);
      *vgap = color;
      vgap += _RES_X>>3;
    }
  }  

}

#pragma aux fill_screen parm [ax];
void __far fill_screen(unsigned color)
{
  unsigned int cnt = ((_RES_X)>>3) * (_RES_Y)+1;

  update_screen();
  
//  out(0x03CE, 0x05);
//  out(0x03CF, 0x02);
  wordout(0x03CE, 0x0205);

//  out(0x03CE, 0x08);
//  out(0x03CF, 0xFF);  
  wordout(0x03CE, 0xFF08);

  memset(VGA, color, cnt);

  ega4_lastoperation = 0;
}

void set_mode_pragma(void);
#pragma aux set_mode_pragma = \
  "push ax" \
  "mov ax, 0x0012" \
  "int 0x10" \
  "pop ax" \
  modify [ax];

void __far set_mode(void)
{
  set_mode_pragma();
}

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
    vmemcpy,
    set_mode,
    (void __far *)(640 | ((unsigned long)480 << 16)),
    (void __far *)(((unsigned long)0xFF << 16) | 8)
  };
  union REGS regs;
  
  if(argc != 2)
    return 1;

//  plane_buff = tsr_alloc(PIXEL_BUFF_SIZE>>1);

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
    _dos_keep(0, ((_FP_OFF( main ) + 0xF) >> 4) + 0x10 ); // exit and leave functions before main() in memory    
  }
  return 1;
}
