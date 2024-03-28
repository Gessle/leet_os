static void save_privileges(struct prog_window *window)
{
  DEFAULT_PRIVS = 0;

  if(window->window_grid[1]->selectc)
    DEFAULT_PRIVS |= VPU_PRIV_FS_READ;
  if(window->window_grid[2]->selectc)
    DEFAULT_PRIVS |= VPU_PRIV_FS_WRITE;
  if(window->window_grid[3]->selectc)
    DEFAULT_PRIVS |= VPU_PRIV_RFS_ACCESS;
  if(window->window_grid[4]->selectc)
    DEFAULT_PRIVS |= VPU_PRIV_SIGNALS;
  if(window->window_grid[5]->selectc)
    DEFAULT_PRIVS |= VPU_PRIV_MODULES;
  if(window->window_grid[6]->selectc)
    DEFAULT_PRIVS |= VPU_PRIV_SYSTEM;
  if(window->window_grid[7]->selectc)
    DEFAULT_PRIVS |= VPU_PRIV_ROOT;        
}

static unsigned privileges_program(struct prog_window *window, unsigned args, int *keypress)
{
  unsigned n = 0;
  if(keypress)
  {
    switch(*keypress)
    {
      case 'S': case 's':
        args = 0;
      break;
      case 'R': case 'r':
        n = 1;
      break;
      case 'M': case 'm':
        n = 2;
      break;
      case 'O': case 'o':
        n = 3;
      break;
      case 'P': case 'p':
        n = 4;
      break;
      case 'U': case 'u':
        n = 5;
      break;
      case 'L': case 'l':
        n = 6;
      break;
      case 'T': case 't':
        n = 7;
      break;
    }
    if(n)
    {
      window->window_grid[n]->selectc = ~window->window_grid[n]->selectc;
      window_drawelement(window, n);
    }
    *keypress = -1;
  }

  if(args == WM_WINPROGACT)
  {
    save_privileges(window);
  }
  
  return ~WM_WINPROGACT;
}

static struct prog_window *init_run_priv_window(void)
{
  struct prog_window *retval = new_window(-1, -1, 300, 160, "Default privileges", 1, 1, "ICONS/RUNPRMPT.ICO", &privileges_program);
  struct window_element *element;

  if(!retval)
  {
    nomemory:
    if(retval) del_window(retval);
    nomemory();
    return 0;
  }
  if(!(window_button(retval, 20, 20, 50, _FONT_HEIGHT+2, "\021Save", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  if(!(element = window_checkbox(retval, 100, 20, "\021Read filesystem", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  element->selectc = 0-!!(DEFAULT_PRIVS & VPU_PRIV_FS_READ);
  if(!(element = window_checkbox(retval, 100, 40, "\021Modify filesystem", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  element->selectc = 0-!!(DEFAULT_PRIVS & VPU_PRIV_FS_WRITE);    
  if(!(element = window_checkbox(retval, 100, 60, "R\021oot filesystem access", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  element->selectc = 0-!!(DEFAULT_PRIVS & VPU_PRIV_RFS_ACCESS);    
  if(!(element = window_checkbox(retval, 100, 80, "\021Process signaling", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  element->selectc = 0-!!(DEFAULT_PRIVS & VPU_PRIV_SIGNALS);    
  if(!(element = window_checkbox(retval, 100, 100, "\021Use modules", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  element->selectc = 0-!!(DEFAULT_PRIVS & VPU_PRIV_MODULES);    
  if(!(element = window_checkbox(retval, 100, 120, "\021Load modules and run DOS programs", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  element->selectc = 0-!!(DEFAULT_PRIVS & VPU_PRIV_SYSTEM);    
  if(!(element = window_checkbox(retval, 100, 140, "Roo\021t privileges", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  element->selectc = 0-!!(DEFAULT_PRIVS & VPU_PRIV_ROOT);    
  if(!(window_text(retval, 20, 5, "Select program start-up privileges:", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;      

  draw_window(retval, 1, 1);
  retval->initialized = 1;

  return retval;
}

/*
  This is the main function for the run dialog
*/

static unsigned rundialog_program(struct prog_window *window, unsigned args, int *keypress)
{
  int c;
  unsigned char *textbox_content = window->window_grid[0]->text;
  unsigned char *argbox_content = window->window_grid[1]->text;
  unsigned char *wdbox_content = window->window_grid[2]->text;

  if(keypress)
  {
    switch(*keypress)
    {
/*      case 112: case 80:
        args=0;
        break;
      case 97: case 65:
        args=1;
        break;
      case 119: case 87:
        args=2;
        break;*/
      case 114: case 82:
        args=(3 | WM_WINPROGACT);
        break;
      case 99: case 67:
        args=(4 | WM_WINPROGACT);
        break;
      case 'V': case 'v':
        args = (5 | WM_WINPROGACT);
        break;
    }
    *keypress = -1;
  }

  if(args & WM_WINPROGACT)
  {
    args &= ~WM_WINPROGACT;
    if(args == 5)
      init_run_priv_window();

    else if(args <= 4)
    {
      run_program(wdbox_content, textbox_content, argbox_content, 1, DEFAULT_PRIVS);

      if(args==4)
      {
        // exit "application"
//        del_window(window);
//        draw_mouse_cursor(1);
//        return -101;            
        return 2;
      }
    }
  }
  return ~WM_WINPROGACT;    
}

/*
  This function creates new window for the run dialog
  Returns: Pointer to the window struct
*/
static struct prog_window *init_run_window(char *program)
{
  struct prog_window *retval = new_window(-1, -1, 300, 100, "Run program", 1, 1, "ICONS/RUNPRMPT.ICO", &rundialog_program);

  if(!retval)
  {
    nomemory:
    if(retval) del_window(retval);
    nomemory();
    return 0;
  }

  if(!(window_textbox(retval, 110, 5, 180, program, 81)))
    goto nomemory;
  if(!(window_textbox(retval, 110, 5+_FONT_HEIGHT+8, 180, "", 128)))
    goto nomemory;
  if(!(window_textbox(retval, 110, 5+2*(_FONT_HEIGHT+8), 180, "", 68)))
    goto nomemory;
  if(!(window_button(retval, 10, 5+3*(_FONT_HEIGHT+8), 50, _FONT_HEIGHT+2, "\021Run", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  if(!(window_button(retval, 70, 5+3*(_FONT_HEIGHT+8), 100, _FONT_HEIGHT+2, "Run & \021close", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  if(!(window_button(retval, 180, 5+3*(_FONT_HEIGHT+8), 80, _FONT_HEIGHT+2, "Pri\021vileges", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  if(!(window_text(retval, 5, 5, "Program path", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  if(!(window_text(retval, 5, 5+_FONT_HEIGHT+8, "Arguments", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;
  if(!(window_text(retval, 5, 5+2*(_FONT_HEIGHT+8), "Working directory", _WINDOW_BASICTEXTCOLOR)))
    goto nomemory;

  draw_window(retval, 1, 1);
  retval->initialized = 1;
  
  return retval;
}
