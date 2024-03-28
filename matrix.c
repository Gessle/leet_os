#define COLS 80

void printchar(unsigned char row, unsigned char col, unsigned char str, unsigned char color, unsigned char __far *dest)
{
  unsigned char __far *destptr = (char __far*)&dest[(row*COLS+col)*2];
  *destptr++=str;
  *destptr=color;
}

unsigned int abs(int num)
{
  return ((num<0)?(-num):(num));
}

void video_mode(unsigned char mode)
{
  __asm
  {
    mov ah, 0x00
    mov al, mode
    int 0x10
  }
}

void wait_refresh(void)
{
  __asm
  {
    push dx
    mov dx, 0x03da
    jump1:    
    in al, dx
    test al, 0x08
    jnz jump1
    jump2:
    in al, dx
    test al, 0x08
    jz jump2
    pop dx
  }
}

unsigned long _DOSRAND_SEED;

unsigned int rand(void)
{
//  static unsigned int seed = 5654;
//  return (seed = 23 * seed % 26479);
  return (_DOSRAND_SEED = 23 * _DOSRAND_SEED % 26479);
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

void scroll_up(unsigned char __far *vga)
{
  unsigned int n;
  for(n=0;n<4000-160;n++)
  {
    vga[n]=vga[n+160];
  }
}

unsigned char colors[] =
{
  0x2A, 0x0A, 0x02, 0x20, 0x2A, 0x22
};

unsigned char randcolor(void)
{
  unsigned char color = colors[rand()%sizeof(colors)];
  if(rand()&1) color &= 0x0F;
  return color;
}

void main()
{
  unsigned char __far *vgatxtbuff = (unsigned char __far*)0xB0008000L;
  unsigned char n,m,c,r;
  unsigned char w=0;
  unsigned char l[40];
  unsigned char ikb;
  unsigned char scancode;
  seed_dosrand();
  video_mode(0x03);

  __asm
  {
    mov ah, 0x0c
    mov al, 0
    int 21h
    in al, 0x60
    mov ikb, al
    mov scancode, al    
  }


  for(m=0;m<80;m++)
    printchar(0, m, rand()%255, randcolor(), vgatxtbuff);
  for(n=1;n<25;n++)
    for(m=0;m<80;m++)
    {
      printchar(n, rand()%80, rand()%255, randcolor(), vgatxtbuff); 
    }  
  while(1)
  {
    for(r=rand()%3;r--;)
    {
      for(m=0;m<40;m++)
        l[m]=rand()%80;
      for(n=rand()%25;n<25-rand()%5;n++)
      {
        for(m=0;m<40;m++)
        {
          c=rand()%25;
          if(c!=n) c=vgatxtbuff[w*2+1];
          else
          {           
            vgatxtbuff[w*2+1]=c=randcolor();
          }
          w=l[rand()%20];
          printchar(n, w, rand()%255, vgatxtbuff[w*2+1], vgatxtbuff);
          if(m&1)wait_refresh();
          __asm
          {
            in al, 0x60
            mov scancode, al
            mov ah, 0x0c
            mov al, 0
            int 21h          
          }
          if(scancode != ikb)
            goto end;
        }
      }
    }
    scroll_up(vgatxtbuff);
    for(m=0;m<80;m++)
    {               
      printchar(0, m, rand()%255, randcolor(), vgatxtbuff);
      printchar(24, m, rand()%255, randcolor(), vgatxtbuff);      
    }
  }
  end:;
}
