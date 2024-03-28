#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>

unsigned char __far *VGA=(unsigned char __far *)0xB8000000L;

#include "globals.c"
//#include "graphics.c"
#include "settings.c"
#include "textmode.c"

#define _SCRSAVER_DIR "SCRNSVRS/*.*"
#define _WALLPAPERS "DESKTOP/*.XPM"
#define _DEMO_WIDTH 35
#define _DEMO_HEIGHT 20
#define _DEMO_X 40
#define _DEMO_Y 1


void print_demo(void)
{
  unsigned row, col;

  // print desktop background
  for(row=_DEMO_HEIGHT;row;row--)
    for(col=_DEMO_WIDTH;col;col--)
      put_pixel_tm(_DEMO_X+col, _DEMO_Y+row, _DESKTOP_BGCOLOR, VGA);

  // print inactive title bar
  for(col=_DEMO_WIDTH-10;col;col--)
    put_pixel_tm(_DEMO_X+5+col, _DEMO_Y+6, _TITLE_COLOR_IA, VGA);
//  printtxt(_DEMO_Y+10, _DEMO_X+6, "-", (0x00)|(_WINDOW_BGCOLOR<<4), VGA);
  printtxt(_DEMO_Y+6, _DEMO_X+8, "Inactive window", (_TITLE_TEXTCOLOR)|(_TITLE_COLOR_IA<<4), VGA);
  // print inactive window's  body
  for(row=5;row;row--)
    for(col=_DEMO_WIDTH-10;col;col--)
      put_pixel_tm(_DEMO_X+5+col, _DEMO_Y+6+row, _WINDOW_BGCOLOR, VGA);

  // print active title bar
  for(col=_DEMO_WIDTH-10;col;col--)
    put_pixel_tm(_DEMO_X+7+col, _DEMO_Y+7, _TITLE_COLOR, VGA);
//  printtxt(_DEMO_Y+11, _DEMO_X+8, "-", (0x00)|(_WINDOW_BGCOLOR<<4), VGA);
  printtxt(_DEMO_Y+7, _DEMO_X+10, "Active window", (_TITLE_TEXTCOLOR)|(_TITLE_COLOR<<4), VGA);
  // print active window's  body
  for(row=10;row;row--)
    for(col=_DEMO_WIDTH-10;col;col--)
      put_pixel_tm(_DEMO_X+7+col, _DEMO_Y+7+row, _WINDOW_BGCOLOR, VGA);
  printtxt(_DEMO_Y+9, _DEMO_X+20, "Normal text", (_WINDOW_BASICTEXTCOLOR)|(_WINDOW_BGCOLOR<<4), VGA);

  printtxt(_DEMO_Y+11, _DEMO_X+20, "Textbox   ", (_TEXTBOX_TEXTCOLOR)|(_TEXTBOX_BGCOLOR<<4), VGA);


  // print active icon
  for(row=3;row;row--)
    for(col=6;col;col--)
      if((row+col)&1)
        put_pixel_tm(_DEMO_X+2+col, _DEMO_Y+1+row, _ICON_ACOLOR, VGA);
  printtxt(_DEMO_Y+4, _DEMO_X+2, "Active icon", (_ICON_ACOLOR)|(_DESKTOP_BGCOLOR<<4), VGA);

  // print inactive icon
  for(row=3;row;row--)
    for(col=6;col;col--)
      if(!((row+col)&1))
        put_pixel_tm(_DEMO_X+15+col, _DEMO_Y+1+row, _ICON_ICOLOR, VGA);
  printtxt(_DEMO_Y+4, _DEMO_X+15, "Inactive icon", (_ICON_ICOLOR)|(_DESKTOP_BGCOLOR<<4), VGA);
}

char *video_mode_str(unsigned char mode)
{
  static char *modes[] =
  {
    "CGA Small Fonts",
    "EGA B&W Small Fonts",
    "EGA COLOR Small Fonts",
    "VGA B&W Small Fonts",
    "VGA COLOR Small Fonts",
    "VGA 320x200 Small Fonts",
    "CGA Large Fonts",
    "EGA B&W Large Fonts",
    "EGA COLOR Large Fonts",
    "VGA B&W Large Fonts",
    "VGA COLOR Large Fonts",
    "VGA 320x200 Large Fonts"
  };

  switch (mode)
  {
    case 0x06:
      return modes[0];
    case 0x0F:
      return modes[1];
    case 0x10:
      return modes[2];
    case 0x11:
      return modes[3];
    case 0x12:
      return modes[4];
    case 0x13:
      return modes[5];
    case 0x26:
      return modes[6];
    case 0x2F:
      return modes[7];
    case 0x30:
      return modes[8];
    case 0x31:
      return modes[9];
    case 0x32:
      return modes[10];
    case 0x33:
      return modes[11];
    default:
      return modes[0];
  }
}

  unsigned char *settings8[] =
  {
    &video,
    &_TITLE_COLOR,
    &_TITLE_COLOR_IA,
    &_TITLE_TEXTCOLOR,
    &_WINDOW_BGCOLOR,
    &_MENU_BGCOLOR,
    &_MENU_SELCOLOR,
    &_MENU_LINECOLOR,
    &_MENU_ATEXTCOLOR,
    &_WINDOW_BASICTEXTCOLOR,
    &_BORDERCOLOR_LIGHT,
    &_BORDERCOLOR_SHADOW,
    &_DESKTOP_BGCOLOR,
    &_ICON_ACOLOR,
    &_ICON_ICOLOR,
    &_TEXTBOX_BGCOLOR,
    &_TEXTBOX_ABGCOLOR,
    &_TEXTBOX_TEXTCOLOR,
    &_TEXTBOX_ATEXTCOLOR,
    &_SCROLLBAR_BGCOLOR,
    &_ATEXT_BGCOLOR,
  };


unsigned print_opts(unsigned selection)
{
  unsigned n = 0;
  unsigned char color;
  unsigned char settings_str[SETTING_LEN];
  char *options[] =
  {
    "Screen mode",
    "Active window title color",
    "Passive window title color",
    "Title text color",
    "Window body background color",
    "Menu background color",
    "Menu selection color",
    "Menu separator line color",
    "Menu selection text color",
    "Window basic text color",
    "Light border color",
    "Dark border color",
    "Desktop background color",
    "Active icon color",
    "Passive icon color",
    "Textbox background color",
    "Active textbox background color",
    "Textbox text color",
    "List selection text color",
    "Scrollbar background color",
    "List selection background color",
    "Screensaver delay (0 = no screensaver)",
    "Screensaver program",
    "Desktop wallpaper"
  };

  clear_screen_tm(VGA);
    
  printtxt(0, 5, "lEEt/OS config", 0x1F, VGA);

  do
  {
    if(selection==n) color = 0x2F;
    else color = 0x0F;
    printtxt(1+n, 0, options[n], color, VGA);
    if(!n)
      printtxt(1+n, 15, video_mode_str(video), color, VGA);
      
    else if(n < sizeof(settings8)/sizeof(char*))
    {
      put_pixel_tm(39, 1+n, *settings8[n], VGA);
      put_pixel_tm(38, 1+n, *settings8[n], VGA);        
    }
        
    else
    {
      switch(n - sizeof(settings8)/sizeof(char*))
      {
        case 0:
          sprintf(settings_str, "%i", screensaver_delay);
          break;
        case 1:
          sprintf(settings_str, "%s", screensaver_program);
          break;
        case 2:
          sprintf(settings_str, "%s", wallpaper);        
      }
      printtxt(1+n, 39, settings_str, color, VGA);      
    }
  }
  while(++n < sizeof(options)/sizeof(char*));

  print_demo();
  printtxt(24, 60, "Press S to save", 0x0F, VGA);
  
  return sizeof(options)/sizeof(char*);
}

char *file_list(char *dir, unsigned c)
{
  static char retval[13];
  unsigned n=0;

  struct find_t   fileinfo; 
  unsigned        rc;

  rc = _dos_findfirst(dir, _A_NORMAL, &fileinfo);

  while(!rc)
  {
    sprintf(retval, "%s", fileinfo.name);
    if(n++ == c) break;
    rc = _dos_findnext( &fileinfo );      
  }   

  if(rc) return 0;
  return retval;
}

void save_setup(void)
{
  unsigned n,c;
  char setting[13];
  FILE *fp = fopen("setup.ini", "w");
  if(fp)
  {
    for(n=0;n<sizeof(settings8)/sizeof(unsigned char *);n++)
    {
      sprintf(setting, "%i", *settings8[n]);
      for(c=0;setting[c];c++)
        fputc(setting[c], fp);
      fputc('\n', fp);
    }
    sprintf(setting, "%i", screensaver_delay);
    for(c=0;setting[c];c++)
      fputc(setting[c], fp);
    fputc('\n', fp);

    sprintf(setting, "%s", screensaver_program);
    for(c=0;setting[c];c++)
      fputc(setting[c], fp);
    fputc('\n', fp);

    sprintf(setting, "%s", wallpaper);
    for(c=0;setting[c];c++)
      fputc(setting[c], fp);    
  }
  fclose(fp);

  printtxt(24, 60, "Options saved  ", 0x0F, VGA);
}

void main()
{
  int c;
  unsigned selection = 0;
  unsigned opt_count;
  unsigned scrsaver_sel = 0;
  int wallpaper_sel = 0;

  disable_blink();
  load_settings();
  
//  clear_screen_tm(VGA, 2000);
  opt_count = print_opts(selection)-1;  

  while((c = getch()) != 27)
  {
    if(!c)
    {
      c = getch();
      if(c == 72 && selection)
        selection--;
      else if(c == 80 && selection<opt_count)
        selection++;
      else if(c == 73)
        selection = 0;
      else if(c == 81)
        selection = opt_count;
      else
      {
        // video mode option
        if(!selection)
        {
          if(c == 75)
            switch(*settings8[0])
            {
              case 0x26:
                *settings8[0] = 0x13;
                break;
              case 0x2F:
                *settings8[0] = 0x26;
                break;
              case 0x0F:
                *settings8[0] = 0x06;
              case 0x06:
                break;
              default:
               (*settings8[0])--;
            }
          else if(c == 77)
            switch(*settings8[0])
            {
              case 0x06:
                *settings8[0] = 0x0F;
                break;
              case 0x13:
                *settings8[0] = 0x26;
                break;
              case 0x26:
                *settings8[0] = 0x2F;
              case 0x33:
                break;
              default:
                (*settings8[0])++;
            }
        }
        // color options
        else if(selection && selection<opt_count-2)
        {
          if(c == 75 && *settings8[selection])
            (*settings8[selection])--;
          else if(c == 77 && *settings8[selection]<0x0F)
            (*settings8[selection])++;        
        }
        // screen saver delay
        else if(selection == opt_count-2)
        {
          if(c == 75 && screensaver_delay)
            screensaver_delay-=30;
          else if(c == 77)
            screensaver_delay+=30;
        }
        // screensaver
        else if(selection == opt_count-1)
        {
          if(c == 75 && scrsaver_sel)
            scrsaver_sel--;
          else if(c == 77)
            if(!file_list(_SCRSAVER_DIR, ++scrsaver_sel))
              scrsaver_sel--;
          sprintf(screensaver_program, "%s", file_list(_SCRSAVER_DIR, scrsaver_sel));
        }
        // wallpaper
        else
        {
          if(c == 75 && wallpaper_sel > -1)
            wallpaper_sel--;
          else if(c == 77)
            if(!file_list(_WALLPAPERS, ++wallpaper_sel))
              wallpaper_sel--;
          if(wallpaper_sel > -1)
            sprintf(wallpaper, "%s", file_list(_WALLPAPERS, wallpaper_sel));
          else
            *wallpaper = 0;
        }
      }
      print_opts(selection);
    }
    else if(c == 115)
      save_setup();    
  }
  clear_screen_tm(VGA);  
}
