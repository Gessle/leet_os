//#include <stdlib.h>
//#include <stdio.h>
//#include <time.h>
//#include <conio.h>

unsigned char __far *VGA = (unsigned char __far *)0xB8000000L;  

unsigned int abs(int num)
{
  return ((num<0)?(-num):(num));
}

struct screensaver_star
{
  unsigned int x, y;
  int dx, dy;
  unsigned int addx, addy;
};

void video_mode(void)
{
  __asm
  {
    mov ah, 0x00
    mov al, 0x04
    int 0x10
  }
}

void wait_refresh(void)
{
  __asm
  {
    mov dx, 0x03da
    jump1:    
    in al, dx
    test al, 0x08
    jnz jump1
    jump2:
    in al, dx
    test al, 0x08
    jz jump2
  }
}

void put_pixel(unsigned int x, unsigned int y, unsigned char color, unsigned char __far *dest)
{
//  dest[(y<<8)+(y<<6)+x] = color;
  unsigned char __far *dptr;
  unsigned char set_bit = 0xC0 >> ((x%4)<<1);

  if(!(y&0x01))
  {
    dptr = &dest[(y>>1)*80+(x>>2)];

    if(!color)
      *dptr &= ~set_bit;
    else
      *dptr |= set_bit;
  }
  else
  {
    dptr = &dest[0x2000+(y>>1)*80+(x>>2)];
    if(!color)
      *dptr &= ~set_bit;
    else
      *dptr |= set_bit;
  }
     
}

unsigned int _DOSRAND_SEED;

unsigned int rand(void)
{
  static unsigned a, b, c;

  a++;
  b+=a+11;
  c-=b-19;
//  static unsigned int seed = 5654;
//  return (seed = 23 * seed % 26479);
  return a+b+c + (_DOSRAND_SEED = 23 * _DOSRAND_SEED % 26479);
}


void seed_dosrand(void)
{
  unsigned char time_ch, time_cl, time_dh, time_dl;
  __asm
  {
    mov ah, 0x2c
    int 0x21
    mov time_ch, ch
    add time_cl, cl
    add time_dh, dh
    add time_dl, dl
  }
  _DOSRAND_SEED = (time_ch + time_cl + time_dh) * (time_dl + 15);
}


void main(void)
{
//  struct screensaver_star *stars = calloc(100, sizeof(struct screensaver_star));
  struct screensaver_star stars[100];
  unsigned int n;
  unsigned int m = 500;
  unsigned view = 0;
  unsigned char ikb;
  unsigned char scancode;

  seed_dosrand();

 __asm
  {
    mov ah, 0x0c
    mov al, 0
    int 21h
    in al, 0x60
    mov ikb, al
    mov scancode, al    
  }
  
  video_mode();
//  srand(time(0));

  for(n=100;n--;)
  {
    stars[n].x = 160;
    stars[n].y = 100;
    stars[n].dx = rand() % 160;
    stars[n].dy = rand() % 100;
    if(rand() & 0x01)
      stars[n].dx = -stars[n].dx;
    if(rand() & 0x01)
      stars[n].dy = -stars[n].dy;
  }


  while(scancode == ikb)
//  while(1)
  {
    __asm
    {
      in al, 0x60
      mov scancode, al
      mov ah, 0x0c
      mov al, 0
      int 21h          
    }
    if(!m--) view=1;
    for(n=100;n--;)
    {
      stars[n].addx += abs(stars[n].dx);
      stars[n].addy += abs(stars[n].dy);
      if(stars[n].addx >= 160)
      {
        if(view)
          put_pixel(stars[n].x, stars[n].y, 0x00, VGA);
        if(stars[n].dx>0)
          stars[n].x++;
        else
          stars[n].x--;
        stars[n].addx = 0;
      }
      if(stars[n].addy >= 100)
      {
        if(view)
          put_pixel(stars[n].x, stars[n].y, 0x00, VGA);        
        if(stars[n].dy>0)
          stars[n].y++;
        else
          stars[n].y--;
        stars[n].addy = 0;
      }
      
      if(stars[n].x >= 320 || stars[n].y >= 200)
      {
        stars[n].x = 160;
        stars[n].y = 100;
        stars[n].dx = rand() % 160;
        stars[n].dy = rand() % 100;
        if(rand() & 0x01)  
          stars[n].dx = -stars[n].dx;
        if(rand() & 0x01)
          stars[n].dy = -stars[n].dy;
      }
      else
        if(view)
          if(abs(stars[n].x-160) > 5 && abs(stars[n].y-100) > 5)
            put_pixel(stars[n].x, stars[n].y, 0x0F, VGA);
    }
    if(view)
      wait_refresh();
  }
//  free(stars);
//  getch();
}
