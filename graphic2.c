void put_pixel(unsigned int x, unsigned int y, unsigned color)
{
  VGA[(y<<8)+(y<<6)+x] = color;
}

unsigned char get_pixel(unsigned int x, unsigned int y)
{
  return VGA[(y<<8)+(y<<6)+x];
}

void video_mode(unsigned char mode);
#pragma aux video_mode = \
  "mov ah, 0x00" \
  "int 0x10" \
  parm [ax];

void clear_screen(unsigned cnt)
{
  while(cnt--)
  {
    VGA[cnt] = 0;
  }
}  
