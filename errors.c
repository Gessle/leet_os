char errortitle[] = "Error";
char outofmemory[] = "Out of memory.";
char outofmemoryprompt[] = "Actions:\nPress C to terminate, or K to kill the \
currently executing VPU console.";
char outofmemoryprompt2[] = "Press any other key to continue or wait";
char outofmemoryprompt3[] = "seconds..";

void nomemory(void)
{
  int c = 0;
  struct console_struct *console_status;
  struct vpu *running_vpu;
  unsigned timeout = 10;

  if(!running_console || running_console && !((struct console_struct *)running_console->function_status)->vcpu->exiting)
  {
    running_window = 0;
    console_status = running_console->function_status;
    running_vpu = console_status->vcpu;
    while(running_vpu->cr != -1)
      running_vpu = running_vpu->child[running_vpu->cr];
    video_mode(0x03);
    puts(outofmemory);      
    puts(outofmemoryprompt);
    while(!c)
    {
      if(!kbhit())
      {
        printf("\r%s %u %s", outofmemoryprompt2, timeout, outofmemoryprompt3);
        fflush(stdout);
        sleep(1);
        if(!timeout--) goto cont;
      }
      else c = getch();
    }
    if(running_console && ((struct console_struct *)running_console->function_status)->vcpu)            
      switch(c)
      {
        case 67: case 99:
          send_vpu_signal(running_vpu, SIGTERM);
          break;
        case 75: case 107:
          running_vpu->wait = 1;
          console_status->vcpu->exiting = 1;
          console_status->console_kill = 1;          
      }
    cont:
    restore_videomode();
    draw_screen();          
    new_tty = 0;
  }
}

void show_error_code(int error_code)
{
  char *errormsg = malloc(100 * sizeof(char));
  if(!errormsg)
  {
    nomemory();
    return;
  }
  sprintf(errormsg, "Error code: 0x%X (%i)", error_code, error_code);
  dialog(20, 20, 150, 100, 0, 1, 1, errormsg, errortitle, "This dialog shows error codes.");
  free(errormsg);
}

void show_error_msg(char *errormsg)
{
/*    video_mode(0x03);
    puts(errormsg);
    getch();
    restore_videomode();;*/
  dialog(20, 20, 250, 50, 0, 0, 0, errormsg, errortitle, 0);
}




















void show_error_console()
{
  struct prog_window *window;
  struct window_element *textbox;
  struct window_element *vscroll;
  struct window_element *hscroll;
  char title[] = "Error console";
  unsigned n;

  for(n=window_count;n--;)
    if(windows[n]->error_console)
      break;
  if(n == -1)
  {
    if(!(window = new_window(-1, -1, 300, 150, title, 1, 1, 0, 0)))
      return;
    if(!(textbox = window_ml_textbox(window, 1, 1, 280, 130, errorlog, ERRORLOG_SIZE))
       || !window_scrollbar(window, 281, 1, 0, 130-26, -1, 0, (int*)&textbox->scroll_y)
       || !window_scrollbar(window, 1, 131, 1, 280-26, -1, 0, (int*)&textbox->scroll_x))
    {
      del_window(window);
      return;
    }
    window->error_console = 1;
    draw_window(window, 1, 1);
  }
  else
  {
    if(!windows[n]->minimized)
      set_active_window(n, 1);
  }
}

void log_error(char *error)
{
  unsigned n = strlen(errorlog);
  unsigned i = strlen(error);
  if(n + i + 2 >= ERRORLOG_SIZE)
  {
    n -= i+1;
    memmove(errorlog, &errorlog[i], n);
  }
  errorlog[n++] = '\n';
  strcpy(&errorlog[n], error);
}
