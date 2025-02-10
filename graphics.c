extern struct mouse_status *mouse;

extern void __far (*update_screen)(void);
void __far sync_pixels_ega4(void);

void hide_mouse_cursor(void);

void hide_mouse_if_in_box(int x, int y, unsigned width, unsigned height)
{
  if(mouse && !mouse->hidden)
  {
    if(x <= mouse->pointer_x+8 && y <= mouse->pointer_y+12 &&
       x+width > mouse->pointer_x && y+height > mouse->pointer_y)
         hide_mouse_cursor();
  }
}

unsigned char _VIDEO_MODE;

//int ega4_lastoperation = -1;
extern unsigned char __based(__segname("main_TEXT")) ega4_lastoperation;

void dummy_read(unsigned char byte);
#pragma aux dummy_read = \
  parm [al];

// function pointers for drawing to screen
void _far (*video_memcpy)(unsigned long dst, unsigned long src, unsigned bytecount);
void _far (*put_pixel_screen)(unsigned int x, unsigned int y, unsigned color);

void call_video_memcpy(unsigned x, unsigned y, unsigned width, unsigned height, int dx, int dy, unsigned segment);
#pragma aux call_video_memcpy = \
  "call dword ptr es:video_memcpy" \
  parm [ax] [bx] [cx] [dx] [si] [di] [es];

void call_put_pixel(unsigned x, unsigned y, unsigned color, unsigned segment);
#pragma aux call_put_pixel = \
  "call dword ptr es:put_pixel_screen" \
  parm [ax] [bx] [cx] [es];
#pragma aux put_pixel_13h parm [ax] [bx] [cx];
#pragma aux put_pixel_cga parm [ax] [bx] [cx];
#pragma aux put_pixel_ega2 parm [ax] [bx] [cx];
#pragma aux put_pixel_ega4 parm [ax] [bx] [cx];

void _far (*fill_screen_color)(unsigned color);
void call_fill_screen(unsigned color, unsigned segment);
#pragma aux call_fill_screen = \
  "call dword ptr es:fill_screen_color" \
  parm [ax] [es];
#pragma aux fill_screen_px parm [ax];
#pragma aux fill_screen_cga parm [ax];
#pragma aux fill_screen_ega2 parm [ax];
#pragma aux fill_screen_ega4 parm [ax];

void fill_screen(unsigned color)
{
  hide_mouse_cursor();
  call_fill_screen(color, FP_SEG(&fill_screen_color));
}

unsigned _far (*get_pixel_screen)(unsigned int x, unsigned int y);
unsigned call_get_pixel(unsigned x, unsigned y, unsigned segment);
#pragma aux call_get_pixel = \
  "call dword ptr es:get_pixel_screen" \
  parm [ax] [bx] [es] value [ax];
#pragma aux get_pixel_13h parm [ax] [bx] value [ax];
#pragma aux get_pixel_cga parm [ax] [bx] value [ax];
#pragma aux get_pixel_ega2 parm [ax] [bx] value [ax];
#pragma aux get_pixel_ega4 parm [ax] [bx] value [ax];

void _far (*draw_rectangle_screen)(int x0, int y0, unsigned int width, unsigned int height, unsigned color);
void call_draw_rectangle(int x0, int y0, unsigned width, unsigned height, unsigned color, unsigned segment);
#pragma aux call_draw_rectangle = \
  "call dword ptr es:draw_rectangle_screen" \
  parm [ax] [bx] [cx] [dx] [si] [es];
#pragma aux draw_rectangle_px parm [ax] [bx] [cx] [dx] [si];
#pragma aux draw_rectangle_cga parm [ax] [bx] [cx] [dx] [si];
#pragma aux draw_rectangle_ega2 parm [ax] [bx] [cx] [dx] [si];
#pragma aux draw_rectangle_ega4 parm [ax] [bx] [cx] [dx] [si];

inline unsigned mul_80(unsigned num)
{
  return (num << 6) + (num << 4);
}

void _far scrmemcpy(unsigned x, unsigned y, unsigned width, unsigned height, int dx, int dy)
{
  call_video_memcpy(x, y, width, height, dx, dy, FP_SEG(&video_memcpy));
}

/*
  Puts a pixel to the screen in mode 13h
*/
void _far put_pixel_13h(unsigned int x, unsigned int y, unsigned color)
{
  VGA[(y<<8)+(y<<6)+x] = color;
}

/*
Puts a pixel to the screen in mode 0x06
*/
void _far put_pixel_cga(unsigned int x, unsigned int y, unsigned color)
{
  unsigned char __far *dptr;
  unsigned char set_bit = 0x80 >> (x&7);
//  dptr = &VGA[(y>>1)*80+(x>>3)];
  dptr = &VGA[mul_80(y>>1) + (x>>3)];

  if(y&0x01)
    dptr+=0x2000;

  if(!color)
    *dptr &= ~set_bit;
  else
    *dptr |= set_bit;
}

/*
Puts a pixel to the screen in mode 0x09 or 0x11
*/
void _far put_pixel_ega2(unsigned int x, unsigned int y, unsigned color)
{
//  unsigned char __far *dptr=&VGA[y*80+(x>>3)];
  unsigned char __far *dptr = &VGA[mul_80(y) + (x>>3)];
  unsigned char set_bit = 0x80 >> (x&7);

  if(color)
    *dptr |= set_bit;
  else
    *dptr &= ~set_bit;
}

/*
Puts a pixel to the screen in mode 0x10 or 0x12
*/
void _far put_pixel_ega4(unsigned int x, unsigned int y, unsigned color);
/*
{
//  unsigned char __far *dptr=&VGA[y*80+(x>>3)];
  unsigned char __far *dptr = &VGA[mul_80(y)+(x>>3)];
  unsigned char set_bit = 0x80 >> (x&7);

  if(ega4_lastoperation)
  {
    outbyte(0x03CE, 0x05);
    outbyte(0x03CF, 0x02);

    outbyte(0x03CE, 0x08);
    ega4_lastoperation=0;    
  }
  outbyte(0x03CF, set_bit);

  dummy_read(*dptr);

  *dptr = color;

 
}*/

void put_pixel_raw(unsigned x, unsigned y, unsigned color)
{
  if(x < _RES_X && y < _RES_Y)
    call_put_pixel(x, y, color, FP_SEG(&put_pixel_screen));
}

unsigned get_pixel_raw(unsigned x, unsigned y)
{
  return call_get_pixel(x, y, FP_SEG(&get_pixel_screen));
}

/*
Puts a pixel to the screen (calls one of the functions above)
*/
void put_pixel(unsigned int x, unsigned int y, unsigned color)
{
  if(!tty)
  if(x < _RES_X && y < _RES_Y)
  {
    if((!draw_window_box && !running_window) || windowpos_visible(running_window, x, y))
    {
      hide_mouse_if_in_box(x, y, 1, 1);
      call_put_pixel(x, y, color, FP_SEG(&put_pixel_screen));
    }
  }
}

void put_pixel_shadow(unsigned int x, unsigned int y, unsigned int color, unsigned int shadow)
{
  put_pixel(x+1, y+1, shadow);
  put_pixel(x, y, color);
}

/*
Reads a pixel from the screen in mode 0x10 or 0x12
*/
unsigned _far get_pixel_ega4(unsigned int x, unsigned int y);
/*{
//  unsigned char __far *dptr=&VGA[y*80+(x>>3)];
  unsigned char __far *dptr = &VGA[mul_80(y) + (x>>3)];
  unsigned char set_bit = 0x80 >> (x&7);
  unsigned char retval=0;
  register unsigned int n = 0x04;

//  if(!ega4_lastoperation)
  {
    outbyte(0x03CE, 0x04);
    ega4_lastoperation=2;    
  }

  while(n--)
  {
    outbyte(0x03CF, n);
    retval <<=1;        
    if((*dptr)&set_bit) retval |= 1;
  }

  return retval;
  
}*/

/*
Reads a pixel from the screen in mode 0x06
*/
unsigned _far get_pixel_cga(unsigned int x, unsigned int y)
{
  unsigned char __far *dptr;
  unsigned char set_bit = 0x80 >> (x&7);
  if(!(y&0x01))
//    dptr = &VGA[(y>>1)*80+(x>>3)];
    dptr = &VGA[mul_80(y>>1) + (x>>3)];
  else
//    dptr = &VGA[0x2000+(y>>1)*80+(x>>3)];
    dptr = &VGA[0x2000+mul_80(y>>1) + (x>>3)];

  if(*dptr & set_bit)
    return 1;
  return 0;
}

/*
Reads a pixel from the screen in mode 0x13
*/
unsigned _far get_pixel_13h(unsigned int x, unsigned int y)
{
  return VGA[(y<<8)+(y<<6)+x];
}

/*
Reads a pixel from the screen in mode 0x0F or 0x11
*/
unsigned _far get_pixel_ega2(unsigned int x, unsigned int y)
{
  unsigned char __far *dptr = &VGA[mul_80(y) + (x>>3)];
  unsigned char set_bit = 0x80 >> (x&7);
  unsigned char get_bits = 0;

  if(*dptr & set_bit)
    return 0x01;
  else return 0;
 

}

/*
Reads a pixel from the screen
Returns: color of the pixel
*/
unsigned char get_pixel(unsigned int x, unsigned int y)
{
  hide_mouse_if_in_box(x, y, 1, 1);
  return call_get_pixel(x, y, FP_SEG(&get_pixel_screen));
}

/*
Draws the window borders to the screen when moving or resizing the window

Phase == 0 when starting to move or resize a window
Phase == 1 when moving a window, 3 when resizing a window
Phase == 2 when done
*/
void draw_move_borders(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char phase)
{
  static unsigned char *bgpixels=0;
  static unsigned int oldx, oldy;
  static unsigned int oldwidth, oldheight;
  register unsigned int n;
  register unsigned int m=0;
  running_window = 0;
  if((phase == 2) && bgpixels)
  {
    free(bgpixels);
    bgpixels=0;
    return;
  }
  drawresize:
  if(!phase && (bgpixels = malloc((((width+2)<<1) + ((height+2)<<1)+1) * sizeof(unsigned char))))
  {
    draw:
    for(n=width+2;n--;m++)
      bgpixels[m] = get_pixel(x-1+n, y-1);
    for(n=width+2;n--;m++)
      bgpixels[m] = get_pixel(x-1+n, y+height);
    for(n=height+2;n--;m++)
      bgpixels[m] = get_pixel(x-1, y-1+n);
    for(n=height+3;n--;m++)
      bgpixels[m] = get_pixel(x+width, y-1+n);
    oldx = x;
    oldy = y;
    oldwidth = width;
    oldheight = height;
    draw_borders(x, y, width, height, 0);
    return;
  }
  if(bgpixels)
  {
    for(n=oldwidth+2;n--;m++)
      put_pixel(oldx-1+n, oldy-1, bgpixels[m]);
    for(n=oldwidth+2;n--;m++)
      put_pixel(oldx-1+n, oldy+oldheight, bgpixels[m]);
    for(n=oldheight+2;n--;m++)
      put_pixel(oldx-1, oldy-1+n, bgpixels[m]);
    for(n=oldheight+3;n--;m++)
      put_pixel(oldx+oldwidth, oldy-1+n, bgpixels[m]);
    m=0;
    if(phase==3)
    {
      free(bgpixels);
      phase=0;
      goto drawresize;
    }
    goto draw;
  }
}

/*
Fills the screen with zeros
*/
void clear_screen(unsigned cnt)
{
  while(cnt--)
  {
    VGA[cnt] = 0;
  }
}


/*
Waits for the CRT beam retrace
*/
/*void wait_refresh(void)
{
  __asm
  {
//    push dx
//    push ax
    mov dx, 0x03da
    jump1:    
    in al, dx
    test al, 0x08
    jnz jump1
    jump2:
    in al, dx
    test al, 0x08
    jz jump2
//    pop ax
//    pop dx
  }
}*/

inline void draw_rectangle_safetycheck(int *x0, int *y0, unsigned int *width, unsigned int *height)
{
  if(*y0 < 0)
  {
    if(*height > abs(*y0))
    {
      *height += *y0;
      *y0 = 0;
    }
    else return;
  }
  if(*x0 < 0)
  {
    if(*width > abs(*x0))
    {
      *width += *x0;
      *x0 = 0;
    }
    else return;
  }
  
  if(*y0 > _RES_Y || *x0 > _RES_X)
  {
    *y0 = 0;
    *x0 = 0;
    *height=0;
    *width=0;
  }
  if(*y0+*height > _RES_Y)
    *height -= (*y0+*height) - _RES_Y;
  if(*x0+*width > _RES_X)
    *width -= (*x0+*width) - _RES_X;

}

inline unsigned char __far *cgapointer(int x, int y)
{
  if(!(y&0x01))
    return VGA + mul_80(y>>1)+(x>>3);
  else
    return VGA + 0x2000+mul_80(y>>1)+(x>>3);
}

void _far draw_rectangle_cga(int x0, int y0, unsigned int width, unsigned int height, unsigned color)
{
  register unsigned int m = height;
  unsigned int l = (((x0+width)-((x0+width)&7)) - (x0)>>3);
  unsigned int n;
  unsigned char __far *vgap;
  register unsigned char set_bits;
  int sc = x0;

  if(x0&7)
  {
    set_bits = 0xFF>>(x0&7);
    if(color)
      while(m--)
      {
        *cgapointer(x0, y0++) |= set_bits;
      }
    else
      while(m--)
      {
        *cgapointer(x0, y0++) &= ~set_bits;      
      }    
    m = height;
    y0-=height;
    sc+=8;
  }

  if(color) set_bits = 0xFF;
  else set_bits = 0x00;
  while(m--)
  {
    vgap = cgapointer(sc, y0++);
/*    for(n=l;n--;)
      *vgap++ = set_bits;*/
    memset(vgap, set_bits, l);
  }

  if((x0+width)&7)
  {
    set_bits = ~(0xFF>>((x0+width)&7));
    m=height;
    y0--;
    sc = (x0+width);
    if(color)
      while(m--)
      {
        *cgapointer(sc, y0--) |= set_bits;
      }
    else
      while(m--)
      {
        *cgapointer(sc, y0--) &= ~set_bits;      
      }    
  }    
}

void _far draw_rectangle_ega2(int x0, int y0, unsigned int width, unsigned int height, unsigned color)
{
  register unsigned int m = height;
  unsigned int l = (((x0+width)-((x0+width)&7)) - (x0))>>3;
  unsigned int n;
  unsigned char __far *vgap = VGA + mul_80(y0) + (x0>>3);
  register unsigned char set_bits;

  if(x0&7)
  {
    set_bits = 0xFF>>(x0&7);
    if(color)
      while(m--)
      {
        *vgap |= set_bits;
        vgap += _RES_X>>3;
      }
    else
      while(m--)
      {
        *vgap &= ~set_bits;      
        vgap += _RES_X>>3;
      }    
    vgap = VGA + mul_80(y0) + (x0>>3) + 1;
    m = height;
  }

  if(color) set_bits = 0xFF;
  else set_bits = 0x00;
  while(m--)
  {
/*    for(n=l;n--;)
      *vgap++ = set_bits;
    vgap += (_RES_X>>3) - l;*/
    memset(vgap, set_bits, l);
    vgap += (_RES_X>>3);
  }

  if((x0+width)&7)
  {
    set_bits = ~(0xFF>>((x0+width)&7));
    vgap = VGA + mul_80(y0) + ((x0+width)>>3);
    m=height;
    if(color)
      while(m--)
      {
        *vgap |= set_bits;
        vgap += _RES_X>>3;
      }
    else
      while(m--)
      {
        *vgap &= ~set_bits;      
        vgap += _RES_X>>3;
      }    
  }    
  
}

/*
Draws a rectangle to the screen in modes 0x10 and 0x12
*/

void _far draw_rectangle_ega4(int x0, int y0, unsigned int width, unsigned int height, unsigned color)
{
//  unsigned int n = (width+((x0+7)&7))/8-1; 
  register unsigned int m = height;
//  unsigned int l = width/8;
  unsigned int l = (((x0+width)-((x0+width)&7)) - (x0))>>3;
//  unsigned int n;
//  unsigned int l;
//  unsigned char __far *vgap = dest + (y0)*80 + (x0+7)/8;
  unsigned char __far *vgap = VGA + mul_80(y0) + (x0>>3);


  sync_pixels_ega4();    
  

  if(ega4_lastoperation)
  {
    outbyte(0x03CE, 0x05);
    outbyte(0x03CF, 0x02);

    outbyte(0x03CE, 0x08);
    ega4_lastoperation=0;    
  }

  // left side
  if(x0&7)
  {
    outbyte(0x03CF, 0xFF>>(x0&7));
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

  outbyte(0x03CF, 0xFF);
  while(m--)
  {
/*      for(n=l;n--;)
        *vgap++ = color;
      vgap += (_RES_X>>3) - l;*/
    memset(vgap, color, l);
    vgap += (_RES_X>>3);
  }
  
  // right side
  if((x0+width)&7)
  {
    outbyte(0x03CF, ~(0xFF>>((x0+width)&7)));
    vgap = VGA + mul_80(y0) + ((x0+width)>>3);
    for(m=height;m--;)
    {
      dummy_read(*vgap);
      *vgap = color;
      vgap += _RES_X>>3;
    }
  }  
  
}

void _far draw_rectangle_px(int x0, int y0, unsigned int width, unsigned int height, unsigned color)
{
  register unsigned int n;  
  while(height--)
  {
    for(n=width;n--;)
      put_pixel(x0+n, y0+height, color);
  }
}

inline int draw_rectangle_draw_window_check(int *x0, int *y0, int *width, int *height)
{
  if(!draw_window_box) return 0;
  if(*x0 < draw_window_box->x-1)
  {
    *width -= draw_window_box->x-1-*x0;
    *x0 = draw_window_box->x-1;
  }
  if(*y0 < draw_window_box->y-1)
  {
    *height -= draw_window_box->y-1-*y0;
    *y0 = draw_window_box->y-1;
  }
  if(*x0 + *width > draw_window_box->x+draw_window_box->width+1)
    *width = (draw_window_box->x+draw_window_box->width+1) - *x0;
  if(*width <= 0) return -1;
  if(*y0 + *height > draw_window_box->y+draw_window_box->height+3+_FONT_HEIGHT)
    *height = (draw_window_box->y+draw_window_box->height+3+_FONT_HEIGHT) - *y0;
  if(*height <= 0) return -1;
  return 1;
}

inline unsigned screen_find_visibility_block(struct prog_window *window, int y0, int x0, int x1)
{
  unsigned hop = 16;
  unsigned retval = 0;

  while(1)
  {
    while(windowpos_visible(window, x0+retval, y0))
    {
      retval+=hop;
      if(x0+retval > x1)
        return x1-x0;
    }
    hop>>=1;
    if(!hop || !retval) return retval;
    while(!windowpos_visible(window, x0+retval, y0))
    {
      retval-=hop;
    }
    hop>>=1;    
  }
}

inline unsigned screen_find_visibility(struct prog_window *window, int y0, int x0, int x1)
{
  unsigned hop = 16;
  unsigned retval = 0;

  while(1)
  {
    while(!windowpos_visible(window, x0+retval, y0))
    {
      retval+=hop;
      if(x0+retval > x1)
        return x1-x0;
    }
    hop>>=1;
    if(!hop || !retval) return retval;
    while(/*x0+retval < _RES_X && */windowpos_visible(window, x0+retval, y0))
    {
      retval-=hop;
    }
    hop>>=1;    
  }
}

/*
Draws a rectangle to the screen
*/
void draw_rectangle(int x0, int y0, unsigned int width, unsigned int height, unsigned color)
{
  unsigned xw, x, c, x1;

  if(tty) return;

  draw_rectangle_safetycheck(&x0, &y0, &width, &height);

  if(width < 8)
  {
    draw_px:
    draw_rectangle_px(x0, y0, width, height, color);
    return;
  }

  switch(draw_rectangle_draw_window_check((int*)&x0, (int*)&y0, (int*)&width, (int*)&height))
  {
    case -1:
      return;
    case 1:
      if(width < 8) goto draw_px;
      goto draw_nocond;
  }
  
  
//  todo: fiksumpi n„kyvyystarkistus
  if(!screen_redraw && running_window && windows[active_window] != running_window)
  {
    if(running_window != (void*)-1 && (running_window->minimized || running_window->hidden
       || (!windows[active_window]->hidden
       && (windows[active_window]->fullscreen || windows[active_window]->maximized)))) return;

//    hide_mouse_if_in_box(x0, y0, width, height);

    x1 = x0 + width;       
    while(height--)
    {
      x=x0;
      while(x < x1)
      {
        xw = screen_find_visibility_block(running_window, y0, x, x1);
        if(xw >= 8)
        {
          hide_mouse_if_in_box(x, y0, xw, 1);
          call_draw_rectangle(x, y0, xw, 1, color, FP_SEG(&draw_rectangle_screen));
        }
        else
          draw_rectangle_px(x, y0, xw, 1, color);
        x += xw;        
        x += screen_find_visibility(running_window, y0, x, x1);
      }
      y0++;
    }
    return;
  }

//  draw_rectangle_screen(x0, y0, width, height, color);
  draw_nocond:
  hide_mouse_if_in_box(x0, y0, width, height);
  call_draw_rectangle(x0, y0, width, height, color, FP_SEG(&draw_rectangle_screen));
}

/*
Fills the screen with argument color in modes 0x10 and 0x12
*/
void _far fill_screen_ega4(unsigned color)
{
//  unsigned char __far *vgap = VGA + ((_RES_X)>>3) * (_RES_Y);
  unsigned int cnt = ((_RES_X)>>3) * (_RES_Y)+1;

  sync_pixels_ega4();
  
  outbyte(0x03CE, 0x05);
  outbyte(0x03CF, 0x02);

  outbyte(0x03CE, 0x08);
  outbyte(0x03CF, 0xFF);  

//  while(cnt--)
//    *vgap-- = color;
  memset(VGA, color, cnt);
}

/*
Fills the screen with color in modes 0x0F and 0x11
*/
void _far fill_screen_ega2(unsigned color)
{
//  unsigned char __far *vgap = VGA + ((_RES_X)>>3) * (_RES_Y);
  unsigned int cnt = ((_RES_X)>>3) * (_RES_Y)+1;  
  if(color)
//    while(cnt--)
//      *vgap-- = 0xFF;
    memset(VGA, 0xFF, cnt);
  else
//    while(cnt--)
//      *vgap-- = 0x00;
    memset(VGA, 0x00, cnt);
}

/*
Fills the screen with color in mode 0x06
*/
void _far fill_screen_cga(unsigned color)
{
//  unsigned char __far *vgap = VGA + ((_RES_X)>>3) * ((_RES_Y)>>1);
//  unsigned int cnt = ((_RES_X)>>3) * ((_RES_Y)>>1) +1;
  unsigned int cnt = ((_RES_X)>>3) * ((_RES_Y));
  memset(VGA, color, cnt);
//  while(cnt--)
//    *vgap-- = color;
//  vgap = VGA + 0x2000 + ((_RES_X)>>3) * ((_RES_Y)>>1);
//  cnt = ((_RES_X)>>3) * ((_RES_Y)>>1) +1;      
//  while(vgap >= VGA + 0x2000)
//  while(cnt--)
//    *vgap-- = color;
}

void _far fill_screen_px(unsigned color)
{
  unsigned y = _RES_Y;
  register unsigned x;

  while(y--)
  {
    x = _RES_X;
    while(x--)
      put_pixel(x, y, color);
  }
}

/*
  Draws a line to the screen
*/
void draw_line(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, unsigned char color)
{
  int dx=x1-x0;
  int dy=y1-y0;
  register unsigned int n,a=0;
  unsigned int absdx,absdy;

  if(tty) return;

  absdx=abs(dx);
  absdy=abs(dy);

  put_pixel(x0,y0,color);
  if(absdx>absdy)
    for(n=absdx;n--;a+=absdy)
    {
      if(a>=absdx)
      {
        if(dy>0)y0++;
        else y0--;
        a-=absdx;
      }
      if(dx>0)x0++;
      else x0--;
      put_pixel(x0,y0,color);      
    }
  else if(absdx<absdy)
    for(n=absdy;n--;a+=absdx)
    {
      if(a>=absdy)
      {
        if(dx>0)x0++;
        else x0--;
        a-=absdy;
      }
      if(dy>0)y0++;
      else y0--;
      put_pixel(x0,y0,color);            
    }
  else
    for(n=absdx;n--;)
    {
      if(dx>0)x0++;
      else x0--;
      if(dy>0)y0++;
      else y0--;
      put_pixel(x0,y0,color);
    }
}

void __far (*driver_videomode)(void) = 0;

void set_video_mode(int mode);
#pragma aux set_video_mode = \
  "mov ah, 0x00" \
  "int 0x10" \
  parm [ax];

/*unsigned char get_video_mode(void);
#pragma aux get_video_mode = \
  "mov ah, 0x0F" \
  "int 0x10" \
  value [al];*/

/*
Changes the video mode to mode
*/
void video_mode(unsigned char mode)
{
  set_video_mode(mode);
  
  if(videodriver >= 0)
  {
    old_put_pixel = put_pixel_screen;
    old_get_pixel = get_pixel_screen;
    old_draw_rectangle = draw_rectangle_screen;
    old_fill_screen = fill_screen_color;
    old_xres = _RES_X;
    old_yres = _RES_Y;
  }
    
  _RES_X=640;
  _RES_Y=200;
  VPUCON_LINES = 25;
  VGA=(unsigned char __far *)0xA0000000L;      
  switch((_VIDEO_MODE = mode))
  {
    case 0x13:
     _RES_X=320;
     put_pixel_screen = put_pixel_13h;
     get_pixel_screen = get_pixel_13h;
     draw_rectangle_screen = draw_rectangle_px;
     fill_screen_color = fill_screen_px;
     update_screen = 0;
    return;
    case 0x06:
      VGA=(unsigned char __far *)0xB8000000L;
      put_pixel_screen = put_pixel_cga;
      get_pixel_screen = get_pixel_cga;
      draw_rectangle_screen = draw_rectangle_cga;
      fill_screen_color = fill_screen_cga;
      update_screen = 0;
      VPUCON_LINES = 15; // korjaa bugi konsolin skrollauksessa ennen tämän muuttamista
    return;
    case 0x0F:    case 0x10:
      _RES_Y=350;
      if(_VIDEO_MODE == 0x0F)
        goto ega2;
      else
        goto ega4;
    case 0x11:    case 0x12:
      _RES_Y=480;
      if(_VIDEO_MODE == 0x11)
        goto ega2;
      else
        goto ega4;
  }
  ega2:
    put_pixel_screen = put_pixel_ega2;
    get_pixel_screen = get_pixel_ega2;
    draw_rectangle_screen = draw_rectangle_ega2;
    fill_screen_color = fill_screen_ega2;
    update_screen = 0;
  return;
  ega4:
    put_pixel_screen = put_pixel_ega4;
    get_pixel_screen = get_pixel_ega4;
    draw_rectangle_screen = draw_rectangle_ega4;
    fill_screen_color = fill_screen_ega4;
    update_screen = sync_pixels_ega4;
}

void restore_videomode(void)
{
  if(tty)
  {
    set_video_mode(0x03);
    return;
  }
  if(videodriver >= 0)
  {
    if(driver_videomode)
    {
      driver_videomode();
      _VIDEO_MODE = video;
    }
    else
    {
/*      __asm
      {
        mov ah, 0x00
        mov al, video
        int 0x10
      }*/
      set_video_mode(video);
      _VIDEO_MODE = video;      
    }    
    put_pixel_screen = old_put_pixel;
    get_pixel_screen = old_get_pixel;
    draw_rectangle_screen = old_draw_rectangle;
    _RES_X = old_xres;
    _RES_Y = old_yres;
    fill_screen_color = old_fill_screen;
  }
  else
    video_mode(video);

  ega4_lastoperation = -1;
}

  
/*
Updates the display memory from a buffer pointed by *VGABUFF  
*/
/*void update_screen(void)
{
  unsigned long n=64000;

  VGA+=n;
  VGABUFF+=n;

  while(n--)
    *VGA-- = *VGABUFF--;    
}*/

/*
Draws rectangular borders to the screen
*/
void draw_borders(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char type)
{
  unsigned char bordercolor_top = _BORDERCOLOR_SHADOW;
  unsigned char bordercolor_bottom = _BORDERCOLOR_SHADOW;
  unsigned char bordercolor_left = _BORDERCOLOR_SHADOW;
  unsigned char bordercolor_right = _BORDERCOLOR_SHADOW;
  switch (type)
  {
    // outset
    case 0:
      bordercolor_top = _BORDERCOLOR_LIGHT;
      bordercolor_left = _BORDERCOLOR_LIGHT;
      bordercolor_right = _BORDERCOLOR_SHADOW;
      bordercolor_bottom = _BORDERCOLOR_SHADOW;
    break;
    // inset
    case 1:
      bordercolor_top = _BORDERCOLOR_SHADOW;
      bordercolor_left = _BORDERCOLOR_SHADOW;
      bordercolor_right = _BORDERCOLOR_LIGHT;
      bordercolor_bottom = _BORDERCOLOR_LIGHT;
    default:
    break;
    // normal light border
    case 2:
      bordercolor_top = _BORDERCOLOR_LIGHT;
      bordercolor_left = _BORDERCOLOR_LIGHT;
      bordercolor_right = _BORDERCOLOR_LIGHT;
      bordercolor_bottom = _BORDERCOLOR_LIGHT;      
    break;
    // normal dark border
    case 3:
      bordercolor_top = _BORDERCOLOR_SHADOW;
      bordercolor_left = _BORDERCOLOR_SHADOW;    
      bordercolor_right = _BORDERCOLOR_SHADOW;
      bordercolor_bottom = _BORDERCOLOR_SHADOW;
    break;
    // desktop bgcolor border
    case 4:
      bordercolor_top = _DESKTOP_BGCOLOR;
      bordercolor_left = _DESKTOP_BGCOLOR;    
      bordercolor_right = _DESKTOP_BGCOLOR;
      bordercolor_bottom = _DESKTOP_BGCOLOR;
    break;
  }
  // draw top border
  draw_line(x-1, y-1, x+width, y-1, bordercolor_top);
  // left border
  draw_line(x-1, y-1, x-1, y+height, bordercolor_left);
  // right border
  draw_line(x+width, y-1, x+width, y+height, bordercolor_right);
  // bottom border
  draw_line(x-1, y+height, x+width, y+height, bordercolor_bottom);
}

void draw_circle(unsigned x, unsigned y, unsigned r, unsigned color)
{
  unsigned cx = 0;
  unsigned cy = r;
  int d = 3-(r<<1);
  do
  {
    put_pixel(x+cx, y+cy, color);
    put_pixel(x-cx, y+cy, color);
    put_pixel(x+cx, y-cy, color);
    put_pixel(x-cx, y-cy, color);
    put_pixel(x+cy, y+cx, color);
    put_pixel(x-cy, y+cx, color);
    put_pixel(x+cy, y-cx, color);
    put_pixel(x-cy, y-cx, color);

    cx++;
    if(d > 0)
    {
      cy--;
      d = d + ((cx-cy)<<2)+10;
    }
    else
      d = d + (cx<<2) + 6;
  }
  while(cx <= cy);
}

void draw_circle_border(unsigned x, unsigned y, unsigned r, unsigned char type)
{
  unsigned cx = 0;
  unsigned cy = ++r;
  int d = 3-(r<<1);
  unsigned char bordercolor_1 = _BORDERCOLOR_SHADOW;
  unsigned char bordercolor_2 = _BORDERCOLOR_SHADOW;

  switch (type)
  {
    // outset
    case 0:
      bordercolor_1 = _BORDERCOLOR_LIGHT;
      bordercolor_2 = _BORDERCOLOR_SHADOW;
    break;
    // inset
    case 1:
      bordercolor_1 = _BORDERCOLOR_SHADOW;
      bordercolor_2 = _BORDERCOLOR_LIGHT;
    default:
    break;
    // normal light border
    case 2:
      bordercolor_1 = _BORDERCOLOR_LIGHT;
      bordercolor_2 = _BORDERCOLOR_LIGHT;      
    break;
    // normal dark border
    case 3:
      bordercolor_1 = _BORDERCOLOR_SHADOW;
      bordercolor_2 = _BORDERCOLOR_SHADOW;
    break;
    // desktop bgcolor border
    case 4:
      bordercolor_1 = _DESKTOP_BGCOLOR;
      bordercolor_2 = _DESKTOP_BGCOLOR;
    break;
  }
 
  do
  {
    put_pixel(x+cx, y+cy, bordercolor_2);
    put_pixel(x-cx, y+cy, bordercolor_2);
    put_pixel(x+cx, y-cy, bordercolor_1);
    put_pixel(x-cx, y-cy, bordercolor_1);
    put_pixel(x+cy, y+cx, bordercolor_2);
    put_pixel(x-cy, y+cx, bordercolor_1);
    put_pixel(x+cy, y-cx, bordercolor_2);
    put_pixel(x-cy, y-cx, bordercolor_1);

    cx++;
    if(d > 0)
    {
      cy--;
      d = d + ((cx-cy)<<2)+10;
    }
    else
      d = d + (cx<<2) + 6;
  }
  while(cx <= cy);
}


void draw_filled_circle(unsigned x, unsigned y, unsigned r, unsigned color)
{
  unsigned cx = 0;
  unsigned cy = r;
  int d = 3-(r<<1);
  do
  {
    draw_line(x-cx, y+cy, x+cx, y+cy, color);
    draw_line(x-cx, y-cy, x+cx, y-cy, color);
    draw_line(x-cy, y+cx, x+cy, y+cx, color);
    draw_line(x-cy, y-cx, x+cy, y-cx, color);

    cx++;
    if(d > 0)
    {
      cy--;
      d = d + ((cx-cy)<<2)+10;
    }
    else
      d = d + (cx<<2) + 6;
  }
  while(cx <= cy);
}
