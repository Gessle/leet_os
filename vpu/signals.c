static int core_debug(struct console_struct *console);

/*
  Sends a signal to a process.
  Returns 1 if the signal could not be send. Otherwise returns 0.
*/
int send_vpu_signal(struct vpu *vpu, unsigned short signal)
{
  if(!vpu->signal || signal == SIGKILL || signal == SIGSTOP || signal == SIGCONT)
    vpu->signal = signal;
  else return 1;
  if(debug)
  {
    sprintf(message, "Signal %u to process %u", signal, vpu->pid);
    output_debug_info(message);
  }
  if(vpu->pause)
  {
    vpu->pause = 0;
    vpu->error_code = EINTR;
  }
  return 0;
}

static void set_exiting(struct vpu *vpu)
{
//  unsigned n = vpu->child_count;

/*  while(n--)
    if(vpu->child[n]->is_clone)
      vpu->child[n]->exiting = 1;*/

  vpu->exiting = 1;
//  vpu->mutex = 0;
}

/*
  Return values: 0 = normal operation
                 1 = switch to next task
*/
static int process_vpu_signal(struct vpu *vpu)
{
  unsigned n;
  struct vpu *parent;
  unsigned signal = vpu->signal;

  vpu->signal = 0;

  switch(signal)
  {
    case SIGHIDECON: case SIGSHOWCON:
      for(n=window_count;n--;)
        if(windows[n]->function_pointer == vpuconsole_program
        && ((struct console_struct *)windows[n]->function_status)->vcpu == vpu)
        {
          windows[n]->hidden = SIGSHOWCON - signal;
          set_active_window(n, 0);
          draw_window_box = windows[n];
          draw_screen();
          draw_window_box = 0;
          break;
        }
    break;
    case SIGMUTLOCK:
      vpu->mutex_lock = 1;
    break;
    case SIGMUTUNLOCK:
      for(n=vpu_count;n--;)
        if(vpus[n]->data == vpu->data)
          if(vpus[n]->mutex_lock)
          {
            vpus[n]->mutex_lock = 0;
            break;
          }
    break;
    default:
      switch(signal)
      {
        // default action == exit
        case SIGHUP: case SIGINT: case SIGPIPE: case SIGALRM: case SIGTERM:
        case SIGUSR1: case SIGUSR2: case SIGPOLL: case SIGVTALRM: case SIGPROF:
          if(vpu->interrupt_count > signal && vpu->interrupts[signal].address)
            goto sdefault;
        case SIGKILL:skill:
          parent = vpu;
          while(parent->is_clone ? (parent = parent->parent) : 0);
          set_exiting(parent);
          return 1;
        // default action == stop
        case SIGTSTP: case SIGTTIN: case SIGTTOU:
          if(vpu->interrupt_count > signal && vpu->interrupts[signal].address)
            goto sdefault;
        case SIGSTOP:
          vpu->pause = 1;
          return 1;
        case SIGCONT:
          vpu->pause = 0;
          vpu->sleep = 0;
          break;
        // default action == core
        case SIGQUIT: case SIGILL: case SIGTRAP: case SIGABRT: case SIGEMT: case SIGFPE:
        case SIGBUS: case SIGSEGV: case SIGSYS: case SIGXCPU: case SIGXFSZ:
          if(vpu->interrupt_count <= signal || !vpu->interrupts[signal].address)
          {
            sprintf(message, "Guru meditation: %u, 0x%.4X:0x%.4X, %u!", vpu->pid, vpu->code_segment, vpu->ip, signal);
            if(debugfp)
              fprintf(debugfp, "%s\n", message);
            putstr(message);    
            log_error(message);
            // show core dump... if out of memory, kill process
            if(core_debug(running_console->function_status))
              goto skill;
            else
              vpu->wait=1;
          }
          goto sdefault;
        // default action == ignore
        default:sdefault:
          vpu->interrupt_wait = signal;
          break;
      }
  }
  return 0;
}
