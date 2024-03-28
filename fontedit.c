#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <i86.h>

#define EDITFIELD_X 20
#define EDITFIELD_Y 20

#define _far __near

unsigned char __far *VGA=(unsigned char __far *)0xA0000000L;
//unsigned char __far *VGABUFF;

inline unsigned char windowpos_visible(struct prog_window *a, int b, int c) { return 1; }

#include "globals.c"
//#include "graphics.c"
#include "graphic2.c"

//#define put_pixel(x, y, c) put_pixel_13h(x, y, c)

unsigned char draw_char(unsigned char charnum, unsigned int charx, unsigned int chary, unsigned char color, unsigned char font_height, unsigned char *cp)
{
  unsigned char n,m;
  unsigned char c;
  unsigned char cw, cl;
  unsigned int x = charx;
  unsigned int y = chary;
  
  for(n=0;n<222;n++,x=charx,y=chary)
  {
    // cw = width of the character (the first byte of every char)
    // can be 1, 2, 4 or 8 (pixels)
    cw = *cp++;
    // cl = length of the character (calculated from cw and font height)
    cl = (font_height * cw) / 8;

    // read character data
    if(n==charnum)
    {
      for(m=cl;m;m--,y++,x=charx)
      {
        c = *cp++;
        switch(cw)
        {
          case 8:
            // first bit from the left
            if((c & 0x80) == 0x80)
              put_pixel(x, y, color);
            x++;
            // and so on
            if((c & 0x40) == 0x40)
              put_pixel(x, y, color);
            x++;
            if((c & 0x20) == 0x20)
              put_pixel(x, y, color);
            x++;
            if((c & 0x10) == 0x10)
              put_pixel(x, y, color);
            x++;
            if((c & 0x08) == 0x08)
              put_pixel(x, y, color);
            x++;
            if((c & 0x04) == 0x04)
              put_pixel(x, y, color);
            x++;
            if((c & 0x02) == 0x02)
              put_pixel(x, y, color);
            x++;
            if((c & 0x01) == 0x01)
              put_pixel(x, y, color);              
          break;
          case 4:
            // first bit from the left
            if((c & 0x80) == 0x80)
              put_pixel(x, y, color);
            x++;
            // and so on
            if((c & 0x40) == 0x40)
              put_pixel(x, y, color);
            x++;
            if((c & 0x20) == 0x20)
              put_pixel(x, y, color);
            x++;
            if((c & 0x10) == 0x10)
              put_pixel(x, y, color);
            x=charx; y++;
            // and the row increments              
            if((c & 0x08) == 0x08)
              put_pixel(x, y, color);
            x++;
            if((c & 0x04) == 0x04)
              put_pixel(x, y, color);
            x++;
            if((c & 0x02) == 0x02)
              put_pixel(x, y, color);
            x++;
            if((c & 0x01) == 0x01)
              put_pixel(x, y, color);                            
          break;
          case 2:
            if((c & 0x80) == 0x80)
              put_pixel(x, y, color);
            x++;
            if((c & 0x40) == 0x40)
              put_pixel(x, y, color);
            x--; y++;
            if((c & 0x20) == 0x20)
              put_pixel(x, y, color);
            x++;
            if((c & 0x10) == 0x10)
              put_pixel(x, y, color);
            x--; y++;
            if((c & 0x08) == 0x08)
              put_pixel(x, y, color);
            x++;
            if((c & 0x04) == 0x04)
              put_pixel(x, y, color);
            x--; y++;
            if((c & 0x02) == 0x02)
              put_pixel(x, y, color);
            x++;
            if((c & 0x01) == 0x01)
              put_pixel(x, y, color);                                                    
          break;
          case 1:
            if((c & 0x80) == 0x80)
              put_pixel(x, y, color);
            y++;
            if((c & 0x40) == 0x40)
              put_pixel(x, y, color);
            y++;
            if((c & 0x20) == 0x20)
              put_pixel(x, y, color);
            y++;
            if((c & 0x10) == 0x10)
              put_pixel(x, y, color);
            y++;
            if((c & 0x08) == 0x08)
              put_pixel(x, y, color);
            y++;
            if((c & 0x04) == 0x04)
              put_pixel(x, y, color);
            y++;
            if((c & 0x02) == 0x02)
              put_pixel(x, y, color);
            y++;
            if((c & 0x01) == 0x01)
              put_pixel(x, y, color);                                                    
          break;
        }
      }
      return cw;
    }
    else
      cp+=cl;
  }
  return 0;
}

void readchardata(unsigned char ch, unsigned char *chardata)
{
  unsigned int n, m;
  unsigned int x = EDITFIELD_X;
  unsigned int y = EDITFIELD_Y;
  for(n=0; n<ch; n++, y++, x = EDITFIELD_X)
    for(m=0;m<8;m++, x++)
      chardata[n*8+m] = VGA[y*320+x];
    
}

void printchardata(unsigned char ch, unsigned char *chardata)
{
  unsigned int n, m;
  unsigned int x = EDITFIELD_X;
  unsigned int y = EDITFIELD_Y;
  for(n=0;n<ch;n++, y++, x = EDITFIELD_X)
    for(m=0;m<8;m++, x++)
      if(chardata[n*8+m])
        VGA[y*320+x] = 0x02;
}

void main(int argc, char *argv[])
{
  unsigned char font_height;
  unsigned char *font_file;
  unsigned char *font_array;
  unsigned int n,m;
  unsigned int c;
  unsigned int charnum = 0;
  unsigned int selpx_x = 0;
  unsigned int selpx_y = 0;
  unsigned char *chardata[255];
  unsigned char cw;
  unsigned char cs;
  FILE *fp;
  
  video_mode(0x13);
  
  if(argc==3)
  {
    font_height = atoi(argv[1]);
    font_file = argv[2];

    font_array = calloc(font_height*255+1, sizeof(char));
    for(n=0;n<255;n++)
      chardata[n] = calloc(8*font_height, sizeof(char));        

    // if font file does exist
    if(fp = fopen(font_file, "rb"))
    {
      //fgets(font_array, font_height*255, fp);
      fread(font_array, sizeof(char), font_height*255, fp);
      fclose(fp);

      for(n=0;n<222;n++)
      {
        clear_screen(64000);        
        cw = draw_char(n, EDITFIELD_X, EDITFIELD_Y, 0x0F, font_height, font_array);
        readchardata(font_height, chardata[n]);
      }
      
    }

    while(1)
    {
      c = getch();
//      clear_screen(VGA, 64000);
      for(n=0;n<font_height;n++)
        for(m=0;m<8;m++)
          put_pixel(EDITFIELD_X+m, EDITFIELD_Y+n, 0x14);
      
      if(c == 27) goto end;      
      if(c == 43)
      {
        if(charnum<222) charnum++;
        printf("Charnum: %u, char: %c \r", charnum, charnum+33);                    
      }
      if(c == 45)
      {
        if(charnum) charnum--;
        printf("Charnum: %u, char: %c \r", charnum, charnum+33);            
      }
      if(c == 32)
      {
        if(chardata[charnum][selpx_y*8+selpx_x]) chardata[charnum][selpx_y*8+selpx_x]=0x00;
        else chardata[charnum][selpx_y*8+selpx_x] = 0x0F;
      }
      if(c == 115) break;
      if(c == 0)
      {
        c = getch();
        if(c == 72 && selpx_y) selpx_y--;
        if(c == 80 && selpx_y < font_height-1) selpx_y++;
        if(c == 75 && selpx_x) selpx_x--;
        if(c == 77 && selpx_x < 7) selpx_x++;
      }
      printf("Charnum: %u, char: %c \r", charnum, charnum+33);            
      printchardata(font_height, chardata[charnum]);
      put_pixel(EDITFIELD_X + selpx_x, EDITFIELD_Y + selpx_y, 0x01);
    }
    
    // save the font
    fp = fopen(font_file, "wb");
    for(c=0;c<222;c++)
    {
      // determine the width of each character
      cw=8;
      for(n=0;n<font_height;n++)
        for(m=4;m<8;m++)
          if(chardata[c][n*8+m])
            goto savechar;
      cw=4;
      for(n=0;n<font_height;n++)
        for(m=2;m<4;m++)
          if(chardata[c][n*8+m])
            goto savechar;
      cw=2;
      for(n=0;n<font_height;n++)
        if(chardata[c][n*8+1])
          goto savechar;
      cw=1;

      savechar:
      // put the width information first
      fputc(cw, fp);
      // then the character bitmap
      switch(cw)
      {
        case 8:
          for(n=0,cs=0;n<font_height;n++,cs=0)
          {
            if(chardata[c][n*8])
              cs = cs | 0x80;
            if(chardata[c][n*8+1])
              cs = cs | 0x40;
            if(chardata[c][n*8+2])
              cs = cs | 0x20;
            if(chardata[c][n*8+3])
              cs = cs | 0x10;
            if(chardata[c][n*8+4])
              cs = cs | 0x08;
            if(chardata[c][n*8+5])
              cs = cs | 0x04;
            if(chardata[c][n*8+6])
              cs = cs | 0x02;
            if(chardata[c][n*8+7])
              cs = cs | 0x01;
            fputc(cs, fp);
          }
        break;
        case 4:
          for(n=0,cs=0;n<font_height;n++,cs=0)
          {
            if(chardata[c][n*8])
              cs = cs | 0x80;
            if(chardata[c][n*8+1])
              cs = cs | 0x40;
            if(chardata[c][n*8+2])
              cs = cs | 0x20;
            if(chardata[c][n*8+3])
              cs = cs | 0x10;
            if(chardata[c][(++n)*8])
              cs = cs | 0x08;
            if(chardata[c][n*8+1])
              cs = cs | 0x04;
            if(chardata[c][n*8+2])
              cs = cs | 0x02;
            if(chardata[c][n*8+3])
              cs = cs | 0x01;
            fputc(cs, fp);            
          }
        break;
        case 2:
          for(n=0,cs=0;n<font_height;n++,cs=0)
          {
            if(chardata[c][n*8])
              cs = cs | 0x80;
            if(chardata[c][n*8+1])
              cs = cs | 0x40;
            if(chardata[c][(++n)*8])
              cs = cs | 0x20;
            if(chardata[c][n*8+1])
              cs = cs | 0x10;
            if(chardata[c][(++n)*8])
              cs = cs | 0x08;
            if(chardata[c][n*8+1])
              cs = cs | 0x04;
            if(chardata[c][(++n)*8])
              cs = cs | 0x02;
            if(chardata[c][n*8+1])
              cs = cs | 0x01;
            fputc(cs, fp);
          }
        break;
        case 1:
          for(n=0,cs=0;n<font_height;n++,cs=0)
          {
            if(chardata[c][n*8])
              cs = cs | 0x80;
            if(chardata[c][(++n)*8])
              cs = cs | 0x40;
            if(chardata[c][(++n)*8])
              cs = cs | 0x20;
            if(chardata[c][(++n)*8])
              cs = cs | 0x10;
            if(chardata[c][(++n)*8])
              cs = cs | 0x08;
            if(chardata[c][(++n)*8])
              cs = cs | 0x04;
            if(chardata[c][(++n)*8])
              cs = cs | 0x02;
            if(chardata[c][(++n)*8])
              cs = cs | 0x01;
            fputc(cs, fp);
          }
        break;
      }
    }
    fclose(fp);

    end:
    free(font_array);
    for(n=0;n<255;n++)
      free(chardata[n]);
  }
  video_mode(0x03);
}
