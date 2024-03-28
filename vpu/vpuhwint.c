#include <stdlib.h>
#include <stdio.h>
#include <dos.h>

#include "vpustrct.c"
#include "../constrct.c"
#include "../structs.c"
//#include "../globals.c"

extern unsigned vpu_hwint_count;
extern struct vpu_hw_int *vpu_hardware_int_table;
extern unsigned window_count;
extern struct prog_window **windows;
extern struct prog_window *running_window;
extern signed next_window;
extern struct vt *ttys;
extern volatile unsigned next_tty;
void vpu_sched_yield(struct vpu *vpu);

//void __far *int_to_vpu_next_handler = 0;
//extern void (__far __interrupt *int_to_vpu_next_handler)();
//extern unsigned long int_to_vpu_next_handler;

//char viesti[100];
unsigned vpuconsole_program(struct prog_window *window, unsigned args, int *keypress);

int send_vpu_signal(struct vpu *vpu, unsigned short signal);

void set_vpu_turn(struct vpu *vpu)
{
  struct vpu *parent = vpu;
  unsigned n;
  struct console_struct *console_status;
  struct vpu *running_vpu;

  vpu->pause = 0;

  while(parent->parent)
  {
    parent = parent->parent;
    for(n=parent->child_count;n--;)
      if(parent->child[n] == vpu)
      {
        parent->cr = n;
        break;
      }
  }

  parent->interrupt_redirect = 1;

  vpu->cr = -1;

  if(running_window && running_window->function_pointer == vpuconsole_program)
  {
    console_status = running_window->function_status;
    running_vpu = console_status->vcpu;
    while(running_vpu->cr != -1)
      running_vpu = running_vpu->child[running_vpu->cr];
    if(running_vpu != vpu)
    {
      vpu_sched_yield(running_vpu);
      running_vpu->yield_hwint = 1;
    }
  }

  for(n=window_count;n--;)
    if(windows[n]->function_pointer == vpuconsole_program &&
      ((struct console_struct*)windows[n]->function_status)->orig_vpu == parent)
    {
      next_window = n+1;
      return;
    }  
  for(n=TTY_COUNT;n--;)
    if(((struct console_struct*)ttys[n].con.function_status)->orig_vpu == parent)
    {
      next_tty = n+1;
      return;
    }
}

void int_to_vpu(unsigned char interrupt_triggered);
#pragma aux int_to_vpu parm [ax]
void int_to_vpu(unsigned char interrupt_triggered)
{
  unsigned n = vpu_hwint_count;

  while(n--)
    if(vpu_hardware_int_table[n].hw_int == interrupt_triggered)
      break;

  send_vpu_signal(vpu_hardware_int_table[n].vpu, vpu_hardware_int_table[n].vpu_int);
  set_vpu_turn(vpu_hardware_int_table[n].vpu);
}
