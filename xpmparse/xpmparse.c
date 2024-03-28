#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../extdlc.c"

void put_pixel(unsigned int x, unsigned int y, unsigned color);

#define INTENSITY_LIMIT 0xd0
#define TRIGGER_LIMIT 0x40

inline int check_xpm_header(FILE *fp)
{
  char buff[] = "! XPM2\n";
  char xpmbuff[20];

  fgets(xpmbuff, 20, fp);
  
  if(strcmp(xpmbuff, buff))
    return 0;
  return 1;
}

/*inline char *skip_word(char *str)
{
  while(*str != ' ')
    str++;
  return str;
}*/

static char *split_str(char *dest, char *str, char delim)
{ 
  while(*str != delim)
  {
    if(dest)
      *dest++ = *str++;
    else
      str++;
    if(!*str)
      return 0;
  }
  if(dest)
    *dest = 0;

  return ++str;
}

#include "xpmstrct.c"

inline int get_xpm_initdata(struct xpm_data *retval, FILE *fp)
{
  char buff[20];
  char *ptr = buff;
  char num[6];

  fgets(buff, sizeof(buff), fp);

  if(!(ptr = split_str(num, ptr, ' ')))
    return -1;
  retval->width = atoi(num);
  if(!(ptr = split_str(num, ptr, ' ')))
    return -1;
  retval->height = atoi(num);
  if(!(ptr = split_str(num, ptr, ' ')))
    return -1;
  retval->colors = atoi(num);
  retval->chars_per_pixel = atoi(ptr);
  if(retval->chars_per_pixel > 5)
    return -1;

  return 0;
}

inline unsigned char parse_hex_color(char *hex_color)
{ 
  unsigned char retval = 0;
  unsigned color;
  char red[3] = { 0, 0, 0 };
  char green[3] = { 0, 0, 0 };
  char blue[3] = { 0, 0, 0 };
  
  strncpy(red, hex_color, 2);
  strncpy(green, hex_color+2, 2);
  strncpy(blue, hex_color+4, 2);
  
  if((color = strtoul(red, 0, 0x10)) > TRIGGER_LIMIT)
  {
    if(color > INTENSITY_LIMIT)
      retval |= 0x88 | 0x44;
    else if(color > TRIGGER_LIMIT*3)
      retval |= 0x80 | 0x44;
    else if(color > TRIGGER_LIMIT*2)
      retval |= 0x44;
    else if(color > TRIGGER_LIMIT)
      retval |= 0x04;
  }
  if((color = strtoul(blue, 0, 0x10)) > TRIGGER_LIMIT)
  {
    if(color > INTENSITY_LIMIT)
      retval |= 0x88 | 0x11;
    else if(color > TRIGGER_LIMIT*3)
      retval |= 0x80 | 0x11;
    else if(color > TRIGGER_LIMIT*2)
      retval |= 0x11;
    else if(color > TRIGGER_LIMIT)
      retval |= 0x01;
  }
  if((color = strtoul(green, 0, 0x10)) > TRIGGER_LIMIT)
  {
    if(color > INTENSITY_LIMIT)
      retval |= 0x88 | 0x22;
    else if(color > TRIGGER_LIMIT*3)
      retval |= 0x80 | 0x22;
    else if(color > TRIGGER_LIMIT*2)
      retval |= 0x22;
    else if(color > TRIGGER_LIMIT)
      retval |= 0x02;
  }

  return retval;
}

inline int get_xpm_colordata(struct xpm_data *retval, FILE *fp)
{
  unsigned n = retval->colors;
  char buff[20];
  char *tok;
  char *ptr;
  
  retval->color_table = calloc(retval->colors, sizeof(struct xpm_color));

  if(!retval->color_table)
    return -1;

  while(n--)
  {
    fgets(buff, 20, fp);
    if(strlen(buff) < retval->chars_per_pixel + 10)
      return -2;
    memcpy(retval->color_table[n].chars, buff, retval->chars_per_pixel);
    tok = buff + retval->chars_per_pixel + 4;
    retval->color_table[n].color = parse_hex_color(tok);
  }

  return 0;
}

inline int xpm_chars2bitmap(FILE *fp, struct xpm_data *xpm_data, struct xpm_bitmap *bitmap)
{
  int n, h;
  register c;
  unsigned char __far *ptr = bitmap->bitmap;
  char color[6];
  unsigned char d;
  register unsigned char e = 0;

  color[xpm_data->chars_per_pixel] = 0;
  
  for(h=xpm_data->height;h--;)
  {
    for(n=0;n<xpm_data->width;n++)
    {
      if(!fread(color, xpm_data->chars_per_pixel, 1, fp))
        return -1;
      for(c=xpm_data->colors;c--;)
        if(!strcmp(xpm_data->color_table[c].chars, color))
          goto color_found;
      if(c==-1)
        return -1;

      color_found:
      d = xpm_data->color_table[c].color;
      if((n ^ h) & 1) d >>= 4;
      else d &= 0x0F;

      if(e)
        *ptr |= d;
      else
        *ptr |= d << 4;

      ptr += e & 1;
      e = ~e;
    }    
    fgetc(fp); // newline char
  }
  return 0;
}

int load_xpm_bitmap(char *filename, struct xpm_bitmap *retval)
{
  FILE *fp = fopen(filename, "r");
  struct xpm_data xpm_data;
  unsigned long alloc_size;

  if(!fp)
    return -1;

  if(!check_xpm_header(fp))
    return -2; // not a XPM2 file

  if(get_xpm_initdata(&xpm_data, fp))
    return -4; // invalid header

  if(get_xpm_colordata(&xpm_data, fp))
    goto nomemory;

  retval->width = xpm_data.width;
  retval->height = xpm_data.height;

  alloc_size = (((unsigned long)xpm_data.width * xpm_data.height) >> 1) + 1;
  if(alloc_size > 0xFFFF)
  {
    nomemory:
    free(xpm_data.color_table);
    fclose(fp);
    return -3;
  }
  retval->bitmap = calloc(alloc_size, 1);

  if(!retval->bitmap)
    goto nomemory;

  xpm_chars2bitmap(fp, &xpm_data, retval);

  free(xpm_data.color_table);
  fclose(fp);
  
  return 0;
}

inline unsigned char bitmap_color(struct xpm_bitmap *bitmap, unsigned long offset)
{
  unsigned char retval = bitmap->bitmap[offset>>1];
  if(offset&0x1)
    return retval & 0x0F;
  return retval >> 4;
}

void draw_xpm_bitmap(struct xpm_bitmap *bitmap, unsigned x0, unsigned y0, unsigned x1, unsigned y1)
{
  unsigned scr_row = y0;
  register unsigned scr_col;
  unsigned long bytedata = 0;
  int hscale = 0;
  int vscale = 0;
  unsigned h = y1-y0;
  unsigned w = x1-x0;

  for(;scr_row<y1;scr_row++)
  {
    vscale += h - bitmap->height;        
    while(vscale >= (signed)bitmap->height)
    {
      vscale -= h;
      bytedata -= bitmap->width;
    }
    while(vscale <= (signed)-bitmap->height)
    {
      vscale += h;
      bytedata += bitmap->width;
    }
    
    for(scr_col=x0;scr_col<x1;scr_col++)
    {
      hscale += w - bitmap->width;       
      while(hscale >= (signed)bitmap->width)
      {
        hscale -= w;
        bytedata--;
      }
      while(hscale <= (signed)-bitmap->width)
      {
        hscale += w;
        bytedata++;
      }
      put_pixel(scr_col, scr_row, bitmap_color(bitmap, bytedata++));     
    }
  }
}
