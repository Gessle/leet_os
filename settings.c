#define SETTING_LEN 13
static void load_settings(void)
{
#ifdef MAIN
  char wallpaper_path[50];
#endif
//  unsigned char *screensaverp = screensaver_program;
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
    &_ATEXT_BGCOLOR
  };
  unsigned int *settings16[] =
  {
    &screensaver_delay
  };
  unsigned char *settingss[] =
  {
    screensaver_program,
    wallpaper
  };
  unsigned int n,m;
  int c;
  char setting[SETTING_LEN];
  FILE *fp = fopen("setup.ini", "r");
  if(fp)
  {
    for(n=0;n<sizeof(settings8)/sizeof(unsigned char *);n++)
    {
      for(m=0;(c = fgetc(fp)) != '\n' && !feof(fp) && m<SETTING_LEN-1;m++)
        setting[m]=c;
      setting[m]=0;
      if(!(!n && videodriver >= 0))  
        *settings8[n] = atoi(setting);
    }
    for(n=0;n<sizeof(settings16)/sizeof(unsigned int *);n++)
    {
      for(m=0;(c = fgetc(fp)) != '\n' && !feof(fp) && m<SETTING_LEN-1;m++)
        setting[m]=c;
      setting[m]=0;
      *settings16[n] = atoi(setting);
    }    
    for(n=0;n<sizeof(settingss)/sizeof(unsigned char *);n++)
    {
      for(m=0;(c = fgetc(fp)) != '\n' && !feof(fp) && m<SETTING_LEN-1;m++)
        settingss[n][m]=c;
      settingss[n][m]=0;
    }
    
    fclose(fp);
  }


#ifdef MAIN
  sprintf(wallpaper_path, "DESKTOP\\%s", wallpaper);
  if(load_xpm_bitmap(wallpaper_path, &wallpaper_struct))
  {
    wallpaper_struct.bitmap = 0;
  }
#endif

  if(videodriver < 0)
  {
    if(video > 0x13)
    {
      system_font_file[6] = 'V';
      monospace_font_file[6] = 'V';
      _FONT_HEIGHT = 16;
      video -= 0x20;
    }
    else
    {
      system_font_file[6] = 'C';
      monospace_font_file[6] = 'C';    
      _FONT_HEIGHT = 8;    
    }
  }

}
