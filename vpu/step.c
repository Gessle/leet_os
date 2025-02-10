#define IMM_FLAG 0x80
#define IMM_W_FLAG 0x40

#define DATA_FLAG 0x10
#define STK_FLAG 0x20
#define DATA_WORD_FLAG 0x20

#include "ringcopy.c"
#include "sigdef.c"
#include "debug.c"
#include "unixtime.c"
#include "jump.c"
#include "mov.c"
#include "printstr.c"
#include "math_imm.c"
#include "incdec.c"
#include "stack.c"
#include "addsub.c"
#include "comp.c"
#include "segment.c"
#include "muldiv.c"
#include "printint.c"
#include "bitwise.c"
#include "callret.c"
#include "intiret.c"
#include "vpustr.c"
#include "inout.c"
//#include "wait.c"
#include "setflags.c"
#include "signconv.c"
#include "getstr.c"
#include "stacks.c"
#include "vpunop.c"
#include "vpuwin.c"
#include "vpuerrno.c"
#include "signals.c"
#include "memprot.c"
#include "int2vpu.c"
//#include "vpuerrno.c"
#include "krnlparm.c"
#include "vpuenvs.c"
#include "syscall.c"
#include "block.c"

#include "functbl.c"
#include "wait.c"

#define instruction_count (sizeof(instruction_func_ptr) / sizeof(&vpu_instr_wait))

static void vpu_console_instr(struct vpu *vpu, unsigned char *instr)
{
  unsigned char old_ip = vpu->ip;
  unsigned flags;

  vpu->retwait = 0;  

  vpu->code = calloc(vpu->code_count, sizeof(char*));
  if(!vpu->code)
  {
    putstr(outofmemory);
    vpu->code = vpu->realcode;
    return;
  }

  vpu->code[vpu->code_segment] = instr;
  vpu->ip = 1;

  flags = *instr;
  *instr &= 0x3F;

  if(debug)
    vpu_step_debuginfo(vpu, *instr);
  (*instruction_func_ptr[*instr])(vpu, flags);

  free(vpu->code);
  vpu->code = vpu->realcode;

  if(*instr == OPCODE_CALL)
    vpu->stack[vpu->sp+2] = old_ip;

  if(!instr_in_array(*instr, JUMP_INSTR))
    vpu->ip = old_ip;
  else
  {
    ((struct console_struct*)running_console->function_status)->runmode = 2;
    vpu->wait = 0;
    vpu->retwait = 1;
  }
}

inline void vpu_block(struct vpu *vpu)
{
  vpu->ip--;
  vpu->blocking = 1;
}

/*
  Returns -1 if no mutex set, else returns child ID that has mutex lock.
*/
inline int vpu_child_has_mutex(struct vpu *vpu)
{
  unsigned n = vpu->child_count;

  while(n--)
  {
    if(vpu->child[n]->is_clone && vpu->child[n]->mutex_lock)
      break;
  }
  
  return n;
}

inline int vpu_is_exiting(struct vpu *vpu)
{
  if(vpu->exiting)
  {
    if(!diskio_in_progress)
    {
      if(!vpu->zombie)
        free_vpu_resources(vpu);
      if(send_termination_signal(vpu))
        return 1;
    }
    return 2;    
  }
  return 0;
}

inline unsigned vpu_turns(struct vpu *vpu)
{
  unsigned retval = vpu->priority;
  if(vpu_has_active_window(vpu))
    retval <<= 2;
  return retval;
}

static int vpu_alive(struct vpu *vpu)
{
  unsigned n;

  if(!vpu->exiting && !vpu->is_clone)
    return 1;

  for(n=vpu->child_count;n--;)
    if(vpu_alive(vpu->child[n]))
      return 1;
  return 0;
}

inline void vpu_next_cr(struct vpu *vpu)
{
  if(((unsigned)vpu->cr > vpu->child_count) || (vpu->exiting && (vpu->cr < 0)))
    vpu->cr = vpu->child_count;
  vpu->cr--;
}

inline int vpu_child_alive(struct vpu *vpu)
{
  int n = vpu->cr;

  while(n >= 0)
    if(vpu_alive(vpu->child[n]))
      break;
    else n--;
  if(n == -1)
  {
    n = vpu->child_count-1;
    while(n > vpu->cr)
      if(vpu_alive(vpu->child[n]))
        break;
      else n--;
    if(n == vpu->cr) return -1;
  }
  return n;
}

inline struct vpu *next_vpu(struct vpu *vpu)
{
  int mutex;
  int rc;

  if(vpu->mutex_lock)
    vpu->cr = -1;
  else if((mutex = vpu_child_has_mutex(vpu)) != -1)
    vpu->cr = mutex;
  else
    vpu_next_cr(vpu);
  if(rc = vpu_is_exiting(vpu))
  {
    vpu->cr = vpu_child_alive(vpu);
    if(vpu->cr == -1)
    {
      if(rc == 2 && vpu->parent && !diskio_in_progress)
        delete_vpu(vpu);
      return 0; // no processes alive in this branch
    }
  }

  if(vpu->cr == -1) return vpu;
  else return vpu->child[vpu->cr];
}

static int vpu_step(struct vpu *vpu);

inline struct vpu *vpu_recursive(struct vpu *vpu)
{
  struct vpu *retval;
  struct vpu *prev_vpu = 0;

  while(retval = next_vpu(vpu))
  {
    if(retval == vpu)
      return retval;
    prev_vpu = vpu;
    vpu = retval;
  }
  return prev_vpu;
}

static void vpu_step_debugoutput(struct vpu *vpu)
{
  sprintf(message, "PID: %u, Instruction pointer: 0x%.4X,\
 Instruction opcode: 0x%.2X", vpu->pid, vpu->ip, vpu->code[vpu->code_segment][vpu->ip]);
  output_debug_info(message);
//  if((vpu->code[vpu->code_segment][vpu->ip] & 0x3F) < instruction_count)
    vpu_step_debuginfo(vpu, vpu->code[vpu->code_segment][vpu->ip] & 0x3F);
}

unsigned char vpu_run(struct vpu *con_vpu)
{
  unsigned rc;
  unsigned start_time = clock_ticks;
  struct vpu *vpu;
  struct vpu *vpu2 = 0;
  unsigned steps;

  do
  {
    if(((struct console_struct *)running_console->function_status)->console_kill)
      return 2;  
    if(!con_vpu->interrupt_redirect)
    {
      if(!(vpu = vpu_recursive(con_vpu)))
        return 2;
    }
    else
    {
      con_vpu->interrupt_redirect = 0;
      vpu = con_vpu;
      while(vpu->cr != -1)
        vpu = vpu->child[vpu->cr];
    }

    if(vpu2 == vpu) break;

    if(process_is_stopped(vpu) || vpu->sys_wait) continue;

    steps = vpu_turns(vpu);

    if(!debug)
      while(steps-- && !(rc = vpu_step(vpu)));
    else
    {
      vpu_step_debugoutput(vpu);
      vpu_step(vpu);
    }
    if(rc == 4)
      return 0;
    vpu_getwindowkeys(vpu);

    if(!vpu2) vpu2 = vpu;
  }
  while(start_time == clock_ticks);

  return 0;
}

static int vpu_step(struct vpu *vpu)
{
  unsigned char instruction;
  unsigned char flags;
  unsigned ssteps;
  int rc;

  if(vpu->signal)
  {
    if(vpu->blocking) vpu->error_code = EINTR;
    if(process_vpu_signal(vpu))
      return 3;
  }

  vpu->blocking = 0;  

  if(vpu->interrupt_wait && !vpu->interrupt_run && ((struct console_struct *)running_console->function_status)->runmode)
  {
    if(vpu->interrupt_wait < vpu->interrupt_count)
    {
      vpu->wait = 0;      
      vpu_instr_hardint(vpu, vpu->interrupt_wait);
    }
    return 3;
  }

  if(vpu->wait)
  {
    if(vpu->yield)
    {
      vpu->wait = 0;
      vpu->yield = 0;
      if(vpu->yield_hwint)
      {
        vpu->yield_hwint = 0;
        return 4;
      }
    }
    return 3;
  }

  for(ssteps = 64;ssteps--;)
  {
    if(vpu->ip >= vpu->code_size)
    {
      send_vpu_signal(vpu, SIGSEGV);
      sprintf(message, "PID %u: Code offset violation!", vpu->pid);
      putstr(message);
      return 0;
    }
//    else instruction = flags = vpu->code[vpu->code_segment][vpu->ip++];
    else instruction = flags = vpu_next_code_byte(vpu);
    instruction &= 0x3F;
    flags &= 0xC0;

/*    if(instruction >= instruction_count)
    {
      send_vpu_signal(vpu, SIGILL);
      rc = 3;
    }
    else*/
    {
      rc = (*instruction_func_ptr[instruction])(vpu, flags);
      if(rc) break;
    }
  }

  return rc;
}

