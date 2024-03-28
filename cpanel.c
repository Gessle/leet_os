static void reload_shortcuts(void)
{
    while(shortcut_count)
    {
      free(desktop_shortcuts[--shortcut_count].icon);
      free(desktop_shortcuts[shortcut_count].icon_path);
    }
    free(desktop_shortcuts);
    if(!load_desktop_shortcuts())
    {
      video_mode(0x03);
      printf("Critical out of memory error!");
      abort();
    }

}

static unsigned newsc_program(struct prog_window *window, unsigned args, int *keypress)
{
  if(keypress)
  {
    switch(*keypress)
    {
/*      case 83: case 115:
        args=0;
        break;*/
      case 82: case 114:
        args=WM_WINPROGACT;
        break;
    }
    *keypress = -1;
  }
    
/*  if(!args)
  {
    if(edit_text(window, 1) == 13)
      args=1;
  }*/
  if(args == WM_WINPROGACT || args == (WM_WINPROGACT | 1))
  {
    run_program(0, "SCEDITOR.COM", window->window_grid[1]->text, 0, 0xFFFF);
    
    reload_shortcuts();

/*    del_window(window);
    draw_mouse_cursor(1);
    return -101;*/
    return 2;
  }

  
  return ~WM_WINPROGACT; 
}

static struct prog_window *newsc_window(void)
{
  struct prog_window *retval = new_window(-1, -1, 250, 60, "New shortcut", 1, 1, "ICONS/SCEDITOR.ICO", &newsc_program);

  if(!retval)
  {
    nomemory:
    if(retval) del_window(retval);
    nomemory();
    return 0;
  }

  if(!(window_text(retval, 5, 5, "Shortcut filename:", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;

  if(!(window_textbox(retval, 110, 5, 100, "DESKTOP\\", 81)))
    goto nomemory;    

  if(!(window_button(retval, 10, 5+(_FONT_HEIGHT+8), 150, _FONT_HEIGHT+2, "\021Run shortcut editor", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;

  draw_window(retval, 1, 1);
  retval->initialized = 1;
  
  return retval;
}

static void reload_config(void)
{
  free(wallpaper_struct.bitmap);
  load_settings();
  free(system_font);
  load_system_font(system_font_file, &system_font);
  free(monospace_font);
  load_system_font(monospace_font_file, &monospace_font);
  restore_videomode();
  draw_screen();
//  draw_mouse_cursor(1);  
}

inline unsigned cpanel_getkb(int *key, unsigned args)
{
  switch (*key)
  {
    case 99: case 67:
      return 0 | WM_WINPROGACT;
    case 102: case 70:
      return 1 | WM_WINPROGACT;
    case 110: case 78:
      return 2 | WM_WINPROGACT;
    case 97: case 65:
      return 3 | WM_WINPROGACT;
    case 115: case 83:
      return 4 | WM_WINPROGACT;
    default:
      return args;
  }
}

static unsigned cpanel_program(struct prog_window *window, unsigned args, int *keypress)
{
  if(keypress)
  {
    args = cpanel_getkb(keypress, args);
    *keypress = -1;
  }

  if(!(args & WM_WINPROGACT))
    return ~WM_WINPROGACT;
  // config
  if(args == WM_WINPROGACT)
  {
    run_program(0, "CONFIG.COM", 0, 0, 0xFFFF);
    reload_config();
  }
  // default programs
  else if(args == (WM_WINPROGACT | 1))
  {
    run_program(0, "ASCSFX.COM", 0, 0, 0xFFFF);
  }
  else if(args == (WM_WINPROGACT | 2))
  {
    newsc_window();
//    draw_mouse_cursor(1);            
  }
  else if(args == (WM_WINPROGACT | 3))
  {
    dialog(20, 20, 250, 50, 0, 0, 0, versioninfo, infotitle, 0);    
  }
  else if(args == (WM_WINPROGACT | 4))
    show_error_console();
  return ~WM_WINPROGACT; 
}

static struct prog_window *init_cpanel_window(void)
{
  struct prog_window *retval = new_window(-1, -1, 250, 70, "Control Panel", 1, 1, "ICONS/CPANEL.ICO", &cpanel_program);

  if(!retval)
  {
    nomemory:
    if(retval) del_window(retval);
    nomemory();
    return 0;
  }

  if(!(window_icon(retval, 10, 5, "\021Configuration", "ICONS/CONFIG.ICO")))
    goto nomemory;
  if(!(window_icon(retval, 90, 5, "\021File types", "ICONS/ASCSFX.ICO")))
    goto nomemory;      
  if(!(window_icon(retval, 170, 5, "\021New shortcut", "ICONS/SCEDITOR.ICO")))
    goto nomemory;      
  if(!(window_button(retval, 10, 50, 100, _FONT_HEIGHT+2, "\021About lEEt/OS", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  if(!(window_button(retval, 120, 50, 100, _FONT_HEIGHT+2, "\021Show error log", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;

  draw_window(retval, 1, 1);
  retval->initialized = 1;
  
  return retval;
}
