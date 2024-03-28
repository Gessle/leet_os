#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <i86.h>

#define EDITFIELD_X 20
#define EDITFIELD_Y 20
#define BGCOLOR 0x14
#define DRAWCOLOR 0x0F
#define SELECTCOLOR 0x02

#define _far __near

unsigned char __far *VGA=(unsigned char __far *)0xA0000000L;
//unsigned char __far *VGABUFF;

inline unsigned char windowpos_visible(struct prog_window *a, int b, int c) { return 1; }

#include "globals.c"
//#include "graphics.c"
#include "graphic2.c"

//#define put_pixel(x, y, c) put_pixel_13h(x, y, c)
//#define get_pixel(x, y) get_pixel_13h(x, y)

void draw_icon(unsigned int x, unsigned int y, unsigned char color, unsigned char *icon)
{
  unsigned int n=12;
  unsigned int m;
  unsigned int xc;

  while(n--)
  {
    for(m=1,xc=x;m--;)
    {
      if(*icon & 0x80)
        put_pixel(xc, y, color);
      xc++;
      if(*icon & 0x40)
        put_pixel(xc, y, color);
      xc++;
      if(*icon & 0x20)
        put_pixel(xc, y, color);
      xc++;
      if(*icon & 0x10)
        put_pixel(xc, y, color);
      xc++;
      if(*icon & 0x08)
        put_pixel(xc, y, color);
      xc++;
      if(*icon & 0x04)
        put_pixel(xc, y, color);
      xc++;
      if(*icon & 0x02)
        put_pixel(xc, y, color);
      xc++;
      if(*icon & 0x01)
        put_pixel(xc, y, color);
      xc++;
      icon++;
    }
    y++;
  }
}

unsigned char *load_icon(char *icon_file)
{
  FILE *fp;
  unsigned char *retval = malloc(12 * sizeof(char));

  if((fp = fopen(icon_file, "rb")))
  {
    fread(retval, sizeof(char), 12, fp);
    fclose(fp);
    return retval;
  }
  else
    return 0;
}

void draw_bg(void)
{
  unsigned int n=12;
  unsigned int m;
  
  while(n--)
    for(m=8;m--;)
    {
      put_pixel(EDITFIELD_Y+m, EDITFIELD_X+n, BGCOLOR);
    }
    
}

void togglecolor(unsigned char x, unsigned char y, unsigned char *selection_color)
{
/*  if(VGA[(EDITFIELD_Y+y)<<8+(EDITFIELD_Y+y)<<6+x] == BGCOLOR)
    VGA[(EDITFIELD_Y+y)<<8+(EDITFIELD_Y+y)<<6+x] = DRAWCOLOR;
  else if(VGA[(EDITFIELD_Y+y)<<8+(EDITFIELD_Y+y)<<6+x] == DRAWCOLOR)
    VGA[(EDITFIELD_Y+y)<<8+(EDITFIELD_Y+y)<<6+x] = BGCOLOR;*/
  if(*selection_color == BGCOLOR)
    *selection_color = DRAWCOLOR;
  else if(*selection_color == DRAWCOLOR)
    *selection_color = BGCOLOR;
}

void main(int argc, char *argv[])
{
  unsigned char *icon_data;
  int c;
  unsigned char save_byte = 0;
  unsigned int n, m;
  unsigned int x, y;
  unsigned char selection_x = 0;
  unsigned char selection_y = 0;
  unsigned char selection_color;
  FILE *fp;

  video_mode(0x13);
  
  if(argc==2)
  {
    clear_screen(64000);
    draw_bg();
    
    if((icon_data = load_icon(argv[1])))
      draw_icon(EDITFIELD_X, EDITFIELD_Y, DRAWCOLOR, icon_data);

    selection_color = get_pixel(EDITFIELD_X, EDITFIELD_Y);    

    while(1)
    {
      c = getch();
      switch(c)
      {
        case 27:
          goto end;
        break;
        case 115:
          put_pixel(EDITFIELD_X+selection_x, EDITFIELD_Y+selection_y, selection_color);                      
          goto save;
        break;
        case 0:
          switch (getch())
          {
            case 75:
              put_pixel(EDITFIELD_X+selection_x, EDITFIELD_Y+selection_y, selection_color);            
              if(selection_x) selection_x--;
              selection_color = get_pixel(EDITFIELD_X+selection_x, EDITFIELD_Y+selection_y);
            break;
            case 77:
              put_pixel(EDITFIELD_X+selection_x, EDITFIELD_Y+selection_y, selection_color);                        
              if(selection_x<7) selection_x++;
              selection_color = get_pixel(EDITFIELD_X+selection_x, EDITFIELD_Y+selection_y);              
            break;
            case 72:
              put_pixel(EDITFIELD_X+selection_x, EDITFIELD_Y+selection_y, selection_color);                        
              if(selection_y) selection_y--;
              selection_color = get_pixel(EDITFIELD_X+selection_x, EDITFIELD_Y+selection_y);              
            break;
            case 80:
              put_pixel(EDITFIELD_X+selection_x, EDITFIELD_Y+selection_y, selection_color);                        
              if(selection_y<11)selection_y++;
              selection_color = get_pixel(EDITFIELD_X+selection_x, EDITFIELD_Y+selection_y);              
            break;
          }
          put_pixel(EDITFIELD_X+selection_x, EDITFIELD_Y+selection_y, SELECTCOLOR);          
        break;
        case 32:
          togglecolor(selection_x, selection_y, &selection_color);
        break;
      }
      //put_pixel(EDITFIELD_X+selection_x, EDITFIELD_Y+selection_y, SELECTCOLOR);
    }

    save:
    
    fp = fopen(argv[1], "wb");
    for(n=12,y=EDITFIELD_Y,x=EDITFIELD_X;n--;y++)
      for(m=1,x=EDITFIELD_X, save_byte=0;m--;save_byte=0)
      {
//        if(VGA[y<<8+y<<6+x++] == DRAWCOLOR)
        if(get_pixel(x++, y) == DRAWCOLOR)
          save_byte = save_byte | 0x80;
        if(get_pixel(x++, y) == DRAWCOLOR)
          save_byte = save_byte | 0x40;
        if(get_pixel(x++, y) == DRAWCOLOR)
          save_byte = save_byte | 0x20;
        if(get_pixel(x++, y) == DRAWCOLOR)
          save_byte = save_byte | 0x10;
        if(get_pixel(x++, y) == DRAWCOLOR)
          save_byte = save_byte | 0x08;
        if(get_pixel(x++, y) == DRAWCOLOR)
          save_byte = save_byte | 0x04;
        if(get_pixel(x++, y) == DRAWCOLOR)
          save_byte = save_byte | 0x02;
        if(get_pixel(x++, y) == DRAWCOLOR)
          save_byte = save_byte | 0x01;
        fputc(save_byte, fp);
      }
    fclose(fp);


    end:
//    if(icon_data)
      free(icon_data);
  }
  video_mode(0x03);
}
