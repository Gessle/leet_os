#define VPU_REGISTER_COUNT 8
#define VPU_PORT_COUNT 0xFF
#define MAX_FILES 60
#define MAX_DIRS 10
#define MAX_WINDOWS 5
#define WINKEYPOLLRATE 5000
#define DEFAULT_PRIORITY 100

typedef unsigned long posixtime;

//typedef SDL_Renderer VPU_Window;

struct vpu_hw_int
{
  unsigned char hw_int;
  unsigned short vpu_int;
  struct vpu *vpu;
  void (__interrupt __far *prev_int)();
  void (__interrupt __far *handler)();
};


struct VPU_Window
{
  unsigned char __far *renderer;
  struct prog_window *window;
  unsigned short keypress_port;
  unsigned short keypress_int;
  unsigned long buffer_size;
  unsigned element_count;
};

struct file_struct
{
  unsigned long *file_pos;
  unsigned short filepos_segment;
  unsigned short filepos_offset;
  unsigned short flags;
  unsigned char is_ioport;
  int module;
  unsigned device;
  struct file_struct *dup;
  struct file_struct *orig;
  struct file_struct *pipe_to;
  struct file_struct *pipe_from;
  unsigned char *buff;
  unsigned bufwpos;
  unsigned bufrpos;
  int handle;
  unsigned open:1;
};

struct dir_struct
{
  struct find_t fileinfo;
  unsigned error;
  struct dir_struct *dup;
  struct dir_struct *orig;
};

/*struct vpu_wregs
{
  unsigned short *a;
  unsigned short *b;
  unsigned short *c;
  unsigned short *d;
};

struct vpu_bregs
{
  unsigned char *ah, *al;
  unsigned char *bh, *bl;
  unsigned char *ch, *cl;
  unsigned char *dh, *dl;
};*/

struct vpu_flags
{
  unsigned dsegv:1;
  unsigned datav:1;
  unsigned csegv:1;
  unsigned codev:1;
  unsigned stackv:1;
  unsigned overflow:1;
  unsigned sign:1;
  unsigned carry:1;    
  unsigned char fill;
};

/*struct vpu_privs
{
  unsigned fs_read:1; // read from filesystem
  unsigned fs_write:1; // modify filesystem
  unsigned rfs_access:1; // without this can access only files and directories under the initial working directory
  unsigned signals:1; // signaling with other processes
  unsigned modules:1; // use modules
  unsigned system:1; // load modules, spawn DOS programs
  
};*/

#include "privdef.c"

struct vpu_interrupt
{
  unsigned short segment;
  unsigned short address;
};

struct dynsegment
{
  unsigned short length;
  unsigned short segment;
};


struct vpu
{
//  unsigned priority;
//  struct vpu_wregs wregs;
//  struct vpu_bregs bregs;

//  unsigned char privileges;

  unsigned short regs[VPU_REGISTER_COUNT];
  unsigned short code_segment;
  unsigned short ip;
  unsigned short data_segment;
  unsigned short sp;
  // here needs to be 4 "safe" words that the program can modify without
  // crashing the system, so that the register count can be ANDed with 0xF
  // for memory safety
  unsigned short psp, ssp;
  short compreg;
  struct vpu_flags flags;

  unsigned priority;
  unsigned char privileges;

  volatile unsigned short signal;
  unsigned short interrupt_wait;

  unsigned char *byteregs;

  unsigned short *stack;
  unsigned short *primary_stack;
  unsigned short *secondary_stack;
  unsigned short stack_size;

  unsigned char **realcode;

  unsigned char **code;
  unsigned short code_size;
  unsigned short code_count;

  unsigned char **data;
  unsigned short data_size;
  unsigned short data_count;
  
  unsigned short dataseg_table_len;    
  
  unsigned short dynsegment_count;
  struct dynsegment *dynsegments;

  unsigned short *ports;
  unsigned char port_count;

  struct vpu_interrupt *interrupts;
  unsigned short interrupt_count;

  struct file_struct **file_struct;

  struct dir_struct **dir_handles;

  char **envs;
  unsigned env_count;

  struct VPU_Window windows[MAX_WINDOWS];

  char cwd[81];
  char iwd[81];

  int error_code;

  unsigned exit_status_seg;
  unsigned exit_status_off;
  struct vpu *parent;
  unsigned termination_signal;
  struct vpu **child;
  unsigned child_count;
  int cr;
  unsigned argc;
  char **argv;
  unsigned pid;
  volatile unsigned sys_wait:1;
  unsigned is_clone:1;
  unsigned mutex_lock:1;
  unsigned exiting:1;
  unsigned interrupt_run:1;
  unsigned pause:1;
  volatile unsigned wait:1;
  unsigned retwait:1;
  unsigned zombie:1;
  unsigned blocking:1;
  volatile unsigned interrupt_redirect:1;
  volatile unsigned yield:1;
  volatile unsigned yield_hwint:1;
  posixtime sleep;
};

struct unixtime
{
  posixtime time;
  unsigned long nanoseconds;
};
