/*
Prints text to the screen in text mode
Arguments: Screen row, screen column, pointer to the char string to print, 
color of text, pointer to the video memory or buffer
*/
void printtxt(unsigned char row, unsigned char col, unsigned char *str, unsigned char color, unsigned char __far *dest)
{
  unsigned char __far *destptr = (char __far*)&dest[(row*COLS+col)<<1];
  while(*str)
  {
    *destptr++=*str++;
    *destptr++=color;
  }
}
void printtxt_len(unsigned char row, unsigned char col, unsigned char *str, unsigned char color, unsigned char __far *dest, unsigned length)
{
  unsigned char __far *destptr = (char __far*)&dest[(row*COLS+col)<<1];
  while(*str && length--)
  {
    *destptr++=*str++;
    *destptr++=color;
  }
}

void clear_screen_tm(unsigned char __far *dest)
{
  unsigned cnt = 2000;
  while(cnt--)
  {
    *dest++ = 0;
    *dest++ = 0x07;
  }
}

void put_pixel_tm(unsigned x, unsigned y, unsigned char color, unsigned char __far *dest)
{
  dest[(y*80+x)<<1] = 0;
  dest[((y*80+x)<<1)+1] = color<<4;
}

void edit_line(unsigned x, unsigned y, char *str, unsigned len, unsigned viewlen)
{
  unsigned caret_pos = 0;
  int c;

  while(str[caret_pos])
    caret_pos++;

  if(caret_pos>viewlen)
    printtxt(y, x, &str[caret_pos-viewlen], (_TEXTBOX_TEXTCOLOR)|(_TEXTBOX_ABGCOLOR<<4), VGA);
  else
    printtxt(y, x, str, (_TEXTBOX_TEXTCOLOR)|(_TEXTBOX_ABGCOLOR<<4), VGA);    

  while((c = getch()) != 27 && c != 13)
  {
    if(c == 8)
    {
      if(caret_pos)
      {
        if(--caret_pos<viewlen)
          put_pixel_tm(x+caret_pos, y, _TEXTBOX_BGCOLOR, VGA);      
        str[caret_pos] = 0;
      }
    }
    else if(caret_pos<len)
    {
      str[caret_pos++] = c;
      str[caret_pos]=0;
    }
    if(caret_pos>viewlen)
      printtxt(y, x, &str[caret_pos-viewlen], (_TEXTBOX_TEXTCOLOR)|(_TEXTBOX_ABGCOLOR<<4), VGA);
    else
      printtxt(y, x, str, (_TEXTBOX_TEXTCOLOR)|(_TEXTBOX_ABGCOLOR<<4), VGA);    
  }
  printtxt_len(y, x, str, (_TEXTBOX_TEXTCOLOR)|(_TEXTBOX_BGCOLOR<<4), VGA, viewlen);  
}

void disable_blink(void)
{
//  unsigned char *mode_control_register = (unsigned char*)0x03D8;



  __asm
  {
    // VGA AND EGA
    mov ax, 0x1003
    mov bl, 0x00
    int 0x10
    
    // CGA
    mov ax, 0x40
    mov es, ax
    mov dx, es:[0x63]
    add dx, 4

    mov al, es:[0x65]
    and al, 0xDF
    out dx, al
    mov es:[0x65], al
  }
}

