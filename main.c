#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <errno.h>
#include <direct.h>
#include <dos.h>
#include <malloc.h>
#include <io.h>
#include <fcntl.h>

#include "extdlc.c"
#include "defines.c"
#include "pragmas.c"
#include "drvmod\drvstrc.c"
#include "constrct.c"
#include "structs.c"
#include "globals.c"
#include "pm.c"
#include "vpu\vpustrct.c"
#include "settings.c"
#include "path.c"
#include "str.c"
#include "dosexec.c"
#include "sound.c"
#include "drawchk.c"
#include "fonts.c"
#include "menus.c"
#include "keyboard.c"
#include "desktop.c"
#include "mouse.c"
#include "timer.c"
#include "windows.c"
#include "text.c"
#include "errors.c"
#include "mltext.c"
#include "run.c"
#include "ttys.c"
#include "startup.c"
#include "envs.c"

#include "runprmpt.c"
#include "dostime.c"
#include "clock.c"
#include "fileman.c"
#include "cpanel.c"
#include "scrnsvr.c"
#include "arrwndws.c"
#include "vpucon.c"
#include "vpuprog.c"
#include "mtask.c"

#define MAX_WINDOWS 5

inline void start_program(void)
{
  struct unixtime time;

  puts("Loading settings...");
  load_settings();

  puts("Loading system font...");
  load_system_font(system_font_file, &system_font);
  load_system_font(monospace_font_file, &monospace_font);


  puts("Setting interrupts...");
  set_int24();
  set_timer_rtn();
  set_keyboard_handler();

  puts("Setting environment variables...");
  update_envvars();

  init_power_manager();

  puts("Preparing desktop...");
  init_desktop();

  puts("Loading desktop shortcuts...");
  load_desktop_shortcuts();

  puts("Preparing variables...");
  filename_clipboard.pending_operation = 0;
  screensaver_timer = 0;
  time = unixtime(0);
  starttime_low = time.time;
  starttime_high = time.time >> 16;
  clipboard.content = 0;
  errorlog = calloc(ERRORLOG_SIZE, sizeof(char));

  puts("Preparing virtual terminals...");
  ttys = calloc(TTY_COUNT, sizeof(struct vt));

  puts("Switching to graphical display mode...");
  video_mode(video);  

  mouse = init_mouse();

  init_clock_window();

  getcwd(cwd, MAX_PATH);
  _dos_getdrive(&cwdrive);

  save_dosidle_handler();

  run_startup_programs();
}

inline void end_program(void)
{
  restore_cpu_clock();
  unload_drivers();
  reset_dosidle_handler();
  reset_vpu_interrupt_handlers();  
  video_mode(0x03);
  reset_timer_rtn();
  reset_int24();
  reset_keyboard_handler();
}

static void call_drv_screenupdate(void)
{
  if(update_screen)
  {
    if(active_window < 0 || !windows[active_window]->fullscreen || windows[active_window]->scrmode == video)
      update_screen();
  }
}

static void mouse_set_normal_cursor(void)
{
  if(mouse_cursor == busy_cursor)
  {
    hide_mouse_cursor();
    mouse_cursor = normal_cursor;
    redraw_mouse_cursor();
  }
}

static unsigned call_mouse_handler(unsigned a)
{
  unsigned retval = ~WM_WINPROGACT;

  if(tty) return retval;

  if(menu_selected >= 0)
  {
    mouse_handler(0);
    return a;
  }
  if(mouse)
  {    
    mouse_set_normal_cursor();
    if(active_window >= 0)
      retval = mouse_handler(windows[active_window]);
    else
      retval = mouse_handler(0);
  }
  if(retval == ~WM_WINPROGACT) return a;
  return retval;
}

inline void start_arg1_prog(unsigned argc, char **argv)
{
  char argstr[256];
  char *ptr = argstr;
  unsigned n = 0;

  while(++n < argc)
  {
    strcpy(ptr, argv[n]);
    ptr += strlen(ptr);
    *ptr++ = ' ';
  }
  *ptr = 0;
  run_program(0, argv[0], argstr, 0, DEFAULT_PRIVS);
}

inline void screensaver_func(void)
{
  static unsigned int screensaver_secs;

  if(screensaver_delay && screensaver_secs != dostime()->s)
  {
    screensaver_timer++;
    screensaver_secs = dostime()->s;
    if(screensaver_timer > screensaver_delay)
      screen_saver();
  }  
}

static void quitprompt(void)
{
  if(!dialog(20, 20, 200, 50, 2, 1, 1, "Exit to dos?", "Exit", "Press [Yes] to exit, [No] to cancel"))
  {
    end_program();
    exit(0);
  }
}

inline unsigned main_kbd_handler(int *c)
{
  unsigned n;
  
  zero_screensaver_timer();

  // if a function key (c==0)
  if(!(*c = getch()))
  {
    switch(*c = getch())
    {
      // alt+tab          
      case KEY_ALTTAB:
        if(active_window>=0 && windows[active_window]->fullscreen)
          minimize_window(windows[active_window]);
        if(window_count)
        {
          for(n=0;windows[window_visibility_order[n]]->hidden;n++);
          if(windows[window_visibility_order[n]]->minimized)
            unminimize_window(window_visibility_order[n]);
          else
            set_active_window(window_visibility_order[n], 1);
        }
      break;
      //alt + M
      case KEY_ALTM:
        if(active_window >= 0)
          menu_selected = 0;
        else
          menu_selected = 1;
      break;
      //alt+f4
      case 107:
        if(active_window >= 0)
        {
          return WM_CLOSEWINDOW;
        }
      break;
      // alt+f3
      case 106:
        return WM_MINIMIZEWINDOW;
      // alt+f2
      case 105:
        return WM_MINIMIZEALL;
      // alt + f5
      case 108:
        return WM_TILE;
      // alt + f6
      case 109:
        return WM_CASCADE;
      default:
        // if no window is active, use arrow keys to select active desktop icon
        if(active_window < 0)
        {
          desktop_iconnav(*c);
          desktop_iconmov(*c);
        }
        // else unget the keypress for the active window to process
        else
        {
          ungetch(*c);
          *c = 0;
        } 
      return ~WM_WINPROGACT;
    }
    *c = -1;
  }
  // if no special key
  else
    switch(*c)
    {
      // tab
      case 9:
        if(active_window >= 0)
          activate_next_element(windows[active_window]);
        break;      
      // enter
      case KEY_ENTER:
      // if no active window, do the currently selected desktop icon's thing
        if(active_window < 0)
          return WM_ICONSELECT;
      break;
      // esc
      case KEY_ESC:
        if(!window_count || windows[active_window]->hidden || windows[active_window]->function_pointer != &vpuconsole_program)
          return WM_QUIT;
      break;
    }
  return ~WM_WINPROGACT;
}

inline void wm_action(unsigned *action)
{
  unsigned n, m;
  
  switch(*action)
  {
    case WM_KBDMOVEWINDOW: window_kbdmove(windows[active_window]); break;
    case WM_CLOSEWINDOW: // window close
      // if the window is vpu console window, then also terminate the virtual machine
      if(windows[active_window]->function_pointer == &vpuconsole_program)
        delete_console(windows[active_window]->function_status);
      // if the window is created by a virtual machine, place a null pointer to the vpu struct
      else if(windows[active_window]->function_pointer == &vpuwindow_program)
        for(m=vpu_count;m--;)
          for(n=MAX_WINDOWS;n--;)
            if(vpus [m]->windows[n].window == windows[active_window])
            {
              vpus [m]->windows[n].window = 0;
              _ffree(vpus[m]->windows[n].renderer);
            }
      del_window(windows[active_window]);
      *action=0;
      break;
    case WM_KBDRESIZEWINDOW: window_kbdresize(windows[active_window]); break;
    case WM_ICONSELECT:
      switch(icon_selected)
      {
        case 4:
          init_vpuconsole_window(0);
          break;
        case 3:
          init_clock_window();
        break;
        case 2:
          init_cpanel_window();
        break;
        case 1:
          init_fileman_window(0);
        break;
        case 0:
          init_run_window(0);
        break;
      }
      for(n=shortcut_count;n--;)
      {
        if(icon_selected == desktop_icon_count+n)
        {
          if(desktop_shortcuts[n].path[strlen(desktop_shortcuts[n].path)-1] == '\\')
            init_fileman_window(desktop_shortcuts[n].path);
          else if(desktop_shortcuts[n].ask_args)
            init_run_window(desktop_shortcuts[n].path);
          else
            run_program(desktop_shortcuts[n].working_dir, desktop_shortcuts[n].path, 0, 0, DEFAULT_PRIVS);
        }
      }
    break;
    case 3:
    if(windows[active_window]->resizable)
    {
      if(windows[active_window]->maximized)
      {
        unmaximize_window(windows[active_window]);
        screen_redraw=1;
      }
      else
        maximize_window(windows[active_window]);
      set_window_resized(windows[active_window]);
      break;          
    }
    case WM_MINIMIZEWINDOW: case WM_MINIMIZEALL: showdesktop:
      if(active_window >= 0)
        minimize_window(windows[active_window]);
      if(*action == WM_MINIMIZEALL)
        if(active_window >= 0)
          goto showdesktop;
        else
          for(n=window_count;n--;)
            if(windows[n]->function_pointer == &clock_program)
              unminimize_window(n);
    break;
    case WM_TILE:
      if(window_count)
        tile_windows();
      screen_redraw = 1;
    break;
    case WM_CASCADE:
      if(window_count)
        cascade_windows();
      screen_redraw = 1;
    break;
    case WM_QUIT:
      quitprompt();
  }
}

static int mouseclick(void)
{
  if(!mouse) return 0;

  get_mouse_movement();
  
  return mouse->left_pressed | mouse->right_pressed;
}

#pragma aux main aborts;
void main(unsigned argc, char **argv)
{
  unsigned a;
  int c = 0;
  int n;
  int t;

  start_program();

  if(argc > 1)
    start_arg1_prog(argc-1, &argv[1]);

  while(1)
  {
    a = ~WM_WINPROGACT;
    screensaver_func();

    if(menu_selected >= 0)
    {
      if(active_window < 0)
      {
        if(a = dropmenu_handler(0))
          goto mtloop;
        else a = ~WM_WINPROGACT;
      }
      else
        if(a = dropmenu_handler(windows[active_window]))
          if(a & WM_WINPROGACT)
          {
            next_window = active_window + 1;
            goto mtloop;
          }
          else goto actions;
        else a = ~WM_WINPROGACT;
    }    

    else if(!tty && active_window >= 0 && windows[active_window]->active_element >= 0)
    {
      switch(windows[active_window]->window_grid[windows[active_window]->active_element]->type)
      {
        case 2:
          a = edit_textbox(windows[active_window], windows[active_window]->active_element);
        break;
        case 8:
          a = edit_ml_textbox(windows[active_window], windows[active_window]->active_element);
      }
      goto mtloop;
    }
    if(!kbhit())
    {
      mtloop:
      a = call_mouse_handler(a);
      if(!(a & WM_WINPROGACT) && a != ~WM_WINPROGACT) goto actions;
      c = -1;
      switch_tty();
    }
    else
    {
      if((a = main_kbd_handler(&c)) > 0 && a != ~WM_WINPROGACT
         && !tty)
        goto actions;
      if(!tty)
      {
        next_window = active_window+1;
        next_tty = -1;
      }
      else
      {
        next_tty = tty;
        next_window = -1;
      }
    }

    a = mtask_win(&c, a, 0);

    draw_mouse_cursor();
    if(all_proc_blocking()) throttle_cpu();
    else restore_cpu_clock();

    if(tty)
    {
      draw_tty();
      continue;
    }

    if(screen_redraw)
    {
      draw_screen();
      screen_redraw=0;
    }

    actions:wm_action(&a);
  }
  end_program();
}


