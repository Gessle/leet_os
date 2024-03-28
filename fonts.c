#define MONOSPACE_WIDTH 8
#define WHITESPACE_WIDTH 4

/*
  This function loads fonts
  Arguments: path to the font file as string,
  pointer to the pointer where the memory for the font data is allocated
*/

void load_system_font(char *font_file, unsigned char **font_array)
{
  FILE *fp;
  unsigned int font_file_len=0;
  if(!(fp = fopen(font_file, "rb")))
    return;

  fseek(fp, 0, SEEK_END);
  font_file_len = ftell(fp);
  rewind(fp);

  if(!(*font_array = calloc(font_file_len, sizeof(char))))
  {
    nomemory();
    return;
  }

  fread(*font_array, sizeof(char), font_file_len, fp);

  fclose(fp);
}

unsigned string_width(unsigned char *str, unsigned font_height, unsigned count, unsigned monospace)
{
  unsigned retval = 0;
  unsigned n;
  register unsigned w;
  register unsigned h;
  unsigned char *cp;
  unsigned char *font;
  unsigned cw;
  unsigned wadd;

  if(!monospace) font = system_font;
  else// font = monospace_font;
  {
    if(strlen(str) < count)
      return strlen(str)*MONOSPACE_WIDTH;
    else return count*MONOSPACE_WIDTH;
  }

  while(count--)
  {
    if(*str == ' ')
      retval += WHITESPACE_WIDTH;
    else if((n = *str - 33) < 222)
    {
      cp = font;
      while(1)
      {
        cw = *cp++;
        if(!n--) break;
        cp += cw << (font_height >> 4);
      }
      switch(cw)
      {
        case 2:
          cp++;
        case 1:
          retval += cw+1;
          cp++;
          break;
        case 4:
          for(h=0,w=4,wadd=0;h<font_height>>2;h++,cp+=2)
            if(*(unsigned short*)cp & 0x1111)
            {
              wadd++;
              break;
            }
          retval += w + !!wadd;          
          break;
        case 8:
          for(h=0,w=0;h<font_height>>1;h++,cp+=2)
            w |= *(unsigned short*)cp;
          if(w & 0x0101)
            retval += 9;
          else if(w & 0x0202)
            retval += 8;
          else if(w & 0x0404)
            retval += 7;
          else retval += 6;
        break;
      }
    }
    if(!*str++) break;
  }

  return retval;
}

/*
  This function draws a single character to screen
  Arguments: charnum = the character to be drawn, 
  x and y coordinates on the screen,
  color, font height and pointer to the font data
  
  Returns: width of the drawn char in pixels
*/  

unsigned draw_char(unsigned char chr, unsigned x, unsigned y,  unsigned color, unsigned font_height, unsigned char *font)
{
  register unsigned cx = x;
  register unsigned cy = y;
  unsigned n;
  unsigned cw;
  unsigned t;
  unsigned addw = 0;

  chr -= 33;

  if(chr >= 222) return 0;
  while(1)
  {
    cw = *font++;
    if(!chr--) break;
    font += cw << (font_height >> 4);
  }

  switch(cw)
  {
    case 1:
      for(n=font_height;n--;cy++)
      {
        if(*font & (0x01 << (n&7)))
          put_pixel(cx, cy, color);
        if(!(n & 7))
          font++;
      }
      return 2;
    case 2:
      for(n=font_height>>3;n--;font+=2,cy++)
      {
        t = *(unsigned short*)font;
        if(t & 0x0080) put_pixel(cx, cy, color);
        if(t & 0x0040) put_pixel(cx+1, cy, color);
        cy++;
        if(t & 0x0020) put_pixel(cx, cy, color);
        if(t & 0x0010) put_pixel(cx+1, cy, color);
        cy++;
        if(t & 0x0008) put_pixel(cx, cy, color);
        if(t & 0x0004) put_pixel(cx+1, cy, color);
        cy++;
        if(t & 0x0002) put_pixel(cx, cy, color);
        if(t & 0x0001) put_pixel(cx+1, cy, color);
        cy++;
        if(t & 0x8000) put_pixel(cx, cy, color);
        if(t & 0x4000) put_pixel(cx+1, cy, color);
        cy++;
        if(t & 0x2000) put_pixel(cx, cy, color);
        if(t & 0x1000) put_pixel(cx+1, cy, color);
        cy++;
        if(t & 0x0800) put_pixel(cx, cy, color);
        if(t & 0x0400) put_pixel(cx+1, cy, color);
        cy++;
        if(t & 0x0200) put_pixel(cx, cy, color);
        if(t & 0x0100) put_pixel(cx+1, cy, color);
      }
      return 3;
    case 4:
      for(n=font_height>>2;n--;font+=2,cy++)
      {
        t = *(unsigned short*)font;
        if(t & 0x80) put_pixel(cx, cy, color);
        if(t & 0x40) put_pixel(cx+1, cy, color);
        if(t & 0x20) put_pixel(cx+2, cy, color);
        if(t & 0x10)
        {
          put_pixel(cx+3, cy, color);
          addw++;
        }
        cy++;
        if(t & 0x08) put_pixel(cx, cy, color);
        if(t & 0x04) put_pixel(cx+1, cy, color);
        if(t & 0x02) put_pixel(cx+2, cy, color);
        if(t & 0x01)
        {
          put_pixel(cx+3, cy, color);
          addw++;
        }
        cy++;
        if(t & 0x8000) put_pixel(cx, cy, color);
        if(t & 0x4000) put_pixel(cx+1, cy, color);
        if(t & 0x2000) put_pixel(cx+2, cy, color);
        if(t & 0x1000)
        {
          put_pixel(cx+3, cy, color);
          addw++;
        }
        cy++;
        if(t & 0x0800) put_pixel(cx, cy, color);
        if(t & 0x0400) put_pixel(cx+1, cy, color);
        if(t & 0x0200) put_pixel(cx+2, cy, color);
        if(t & 0x0100)
        {
          put_pixel(cx+3, cy, color);
          addw++;
        }
      }
      return 4 + !!addw;
    case 8:
      for(n=font_height>>1;n--;cy++,font+=2)
      {
        t = *(unsigned short*)font;
        if(t & 0x80) put_pixel(cx, cy, color);
        if(t & 0x40) put_pixel(cx+1, cy, color);
        if(t & 0x20) put_pixel(cx+2, cy, color);
        if(t & 0x10) put_pixel(cx+3, cy, color);

        if(t & 0x08) put_pixel(cx+4, cy, color);
        if(t & 0x04)
        {
          put_pixel(cx+5, cy, color);
          if (addw < 1) addw = 1;
        }
        if(t & 0x02)
        {
          put_pixel(cx+6, cy, color);
          if(addw < 2) addw = 2;
        }
        if(t & 0x01)
        {
          put_pixel(cx+7, cy, color);
          if(addw < 3) addw = 3;
        }
        cy++;
        if(t & 0x8000) put_pixel(cx, cy, color);
        if(t & 0x4000) put_pixel(cx+1, cy, color);
        if(t & 0x2000) put_pixel(cx+2, cy, color);
        if(t & 0x1000) put_pixel(cx+3, cy, color);
        if(t & 0x0800) put_pixel(cx+4, cy, color);
        if(t & 0x0400)
        {
          put_pixel(cx+5, cy, color);
          if(addw < 1) addw = 1;
        }
        if(t & 0x0200)
        {
          put_pixel(cx+6, cy, color);
          if(addw < 2) addw = 2;
        }
        if(t & 0x0100)
        {
          put_pixel(cx+7, cy, color);
          if(addw < 3) addw = 3;
        }
      }
      return 6+addw;
  }
  return 0;
}

/*unsigned char draw_char(unsigned char charnum, unsigned int charx, unsigned int chary, unsigned char color, unsigned char font_height, unsigned char *cp)
{
  unsigned char n,m;
  unsigned char c;
  unsigned char cw, cl;
  unsigned char retval=0;
  unsigned int x = charx;
  unsigned int y = chary;

  charnum-=33;
  
  for(n=0;n<222;n++,x=charx,y=chary)
  {
    // cw = width of the character (the first byte of every char)
    // can be 1, 2, 4 or 8 (pixels)
    cw = *cp++;

    // cl = length of the character (calculated from cw and font height)
//    cl = (font_height * cw) >> 3;
    cl = (cw << (font_height>>4));

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
            if(c & 0x80)
              put_pixel(x, y, color);
            x++;
            // and so on
            if(c & 0x40)
              put_pixel(x, y, color);
            x++;
            if(c & 0x20)
              put_pixel(x, y, color);
            x++;
            if(c & 0x10)
              put_pixel(x, y, color);
            x++;
            if(c & 0x08)
              put_pixel(x, y, color);
            x++;
            if(c & 0x04)
              put_pixel(x, y, color);
            x++;
            if(c & 0x02)
            {
              put_pixel(x, y, color);
              if(!retval) retval++;
            }
            x++;
            if(c & 0x01)
            {
              put_pixel(x, y, color);
              retval=2;
            }
          break;
          case 4:
            // first bit from the left
            if(c & 0x80)
              put_pixel(x, y, color);
            x++;
            // and so on
            if(c & 0x40)
              put_pixel(x, y, color);
            x++;
            if(c & 0x20)
              put_pixel(x, y, color);
            x++;
            if(c & 0x10)
            {
              put_pixel(x, y, color);
              if(!retval) retval++;
            }
            x=charx; y++;
            // and the row increments              
            if(c & 0x08)
              put_pixel(x, y, color);
            x++;
            if(c & 0x04)
              put_pixel(x, y, color);
            x++;
            if(c & 0x02)
              put_pixel(x, y, color);
            x++;
            if(c & 0x01)
            {
              put_pixel(x, y, color);
              if(!retval) retval++;
            }
          break;
          case 2:
            if(c & 0x80)
              put_pixel(x, y, color);
            x++;
            if(c & 0x40)
              put_pixel(x, y, color);
            x--; y++;
            if(c & 0x20)
              put_pixel(x, y, color);
            x++;
            if(c & 0x10)
              put_pixel(x, y, color);
            x--; y++;
            if(c & 0x08)
              put_pixel(x, y, color);
            x++;
            if(c & 0x04)
              put_pixel(x, y, color);
            x--; y++;
            if(c & 0x02)
              put_pixel(x, y, color);
            x++;
            if(c & 0x01)
              put_pixel(x, y, color);
          break;
          case 1:
            if(c & 0x80)
              put_pixel(x, y, color);
            y++;
            if(c & 0x40)
              put_pixel(x, y, color);
            y++;
            if(c & 0x20)
              put_pixel(x, y, color);
            y++;
            if(c & 0x10)
              put_pixel(x, y, color);
            y++;
            if(c & 0x08)
              put_pixel(x, y, color);
            y++;
            if(c & 0x04)
              put_pixel(x, y, color);
            y++;
            if(c & 0x02)
              put_pixel(x, y, color);
            y++;
            if(c & 0x01)
              put_pixel(x, y, color);
          break;
        }
      }
      if(cw==8)
        return --cw+retval;
      else if (cw==4)
        return cw+retval;
      else if(cw==2)
        return 3;  
      else if(cw==1)
        return 2;
    }
    else
      cp+=cl;
  }
  return 0;
}*/
#define TAB_WIDTH 4
/*
  This function calls the above function in loop to print a string to the screen
  Arguments: x and y coordinates on the screen, 32-bit pointer to the string, 
  text color, font height in pixels, the X coordinate where to stop printing,
  and if monospace != 0 then prints the text with monospace font
*/  

void gprint_text(unsigned int x, unsigned int y, unsigned char __far *text, unsigned char color, unsigned char font_height, unsigned int x2, unsigned monospace)
{
  unsigned int xu;
  if(!text) return;
  do
  {
    if(*text == 32 || *text == 9)
    {
      if(!monospace)
         x+=WHITESPACE_WIDTH;
    }
/*    else if(*text == 9)
    {
      x+=TAB_WIDTH*MONOSPACE_WIDTH;
    }*/
    // underline next char
    else if(*text == 0x11)
    {
      xu = x+draw_char((*++text), x, y, color, font_height, system_font);
      draw_line(x, y+font_height-1, xu-2, y+font_height-1, color);
      x = xu;
    }
    else
    {
      if(!monospace)
        x += draw_char((*text), x, y, color, font_height, system_font);
      else
      {
        draw_char((*text), x, y, color, font_height, monospace_font);
      }
    }
    if(monospace) x += MONOSPACE_WIDTH;
  }
  while(*text++ && x+8<x2);
}

/*
  Returns end positions
*/
struct text_print cprint_text(unsigned int x, unsigned int y, unsigned char *text, unsigned char color, unsigned char font_height, unsigned int x2, unsigned int count, unsigned monospace)
{
  struct text_print retval;
  unsigned int n=0;

  if(!count) count--;

  if(*text)
    do
    {
      if(*text == 32 || *text == 9)
      {
        if(!monospace)
          x+=WHITESPACE_WIDTH; 
      }
/*      else if(*text == 9)
        x += TAB_WIDTH*MONOSPACE_WIDTH;*/
      else
        if(!monospace)
          x += draw_char((*text), x, y, color, font_height, system_font);
        else
          draw_char((*text), x, y, color, font_height, monospace_font);        
      if(monospace) x += MONOSPACE_WIDTH;
    }
    while(*++text && x+8<x2 && ++n < count);

  retval.text = text;
  retval.x = x;

  return retval;
}
