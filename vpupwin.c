static int vpuprog_window_close(struct prog_window *window)
{
  struct vpu_progwindow *status = window->function_status;
  unsigned n;

  if(SIGWINRESIZE < status->vcpu->interrupt_count)
  {
    if(send_vpu_signal(status->vcpu, SIGWINRESIZE))
      return -1;
    for(n=MAX_WINDOWS;n--;)
      if(status->vcpu->windows[n].window == window)
        break;
    status->vcpu->ports[status->vcpu->windows[n].keypress_port] = 0;
    return 1;
  }
  return 0;
}

static unsigned vpuprog_window_resize(struct prog_window *window)
{
  switch(vpuprog_window_close(window))
  {
    case 0: case -1:
//      draw_window(window, 0, 0);
      window->redraw = 4;
      break;
    case 1:
      draw_window(window, 1, 2);
      break;
  }
  return ~WM_WINPROGACT;
}

static unsigned vpuwindow_program(struct prog_window *window, unsigned args, int *keypress)
{
  struct vpu_progwindow *status = window->function_status;
  unsigned n,m;

  if(keypress)
  {
    status->keybuff[0] = *keypress;
    if(!*keypress)
      status->keybuff[1] = getch();      
    *keypress = -1;
  }
  if(args & WM_WINPROGACT)
  {
    args &= ~WM_WINPROGACT;

    m = window->hitbox[args].element;
    
    status->keybuff[0] = 0x0100 + args;
    if(window->window_grid[m]->type == 2)
    {
      strcpy((char*)window->window_grid[m]->connect_value, window->window_grid[m]->text);
    }
  }
  
  return ~WM_WINPROGACT;
}

struct prog_window *init_vpuprog_window(unsigned x, unsigned y, unsigned char *title, unsigned mouse_int, unsigned mouse_port, void *vpu)
{
  struct prog_window *retval;
  struct vpu_progwindow *status;

  if(!(retval = new_window(-1, -1, x, y, title, 1, 1, 0, &vpuwindow_program)))
  {
    return 0;
  }

  retval->function_status = calloc(1, sizeof(struct vpu_progwindow));
  status = retval->function_status;
  status->keybuff[0] = -1;
  status->keybuff[1] = -1;  
  status->vcpu = vpu;
  status->mouse_int = mouse_int;
  status->mouse_port = mouse_port;
  retval->on_close = vpuprog_window_close;
  retval->on_resize = vpuprog_window_resize;
  retval->initialized = 1;

  return retval;
  
}
