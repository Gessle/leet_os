#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <i86.h>
//#include <process.h>

#include "..\extdlc.c"

#include "drvdef.c"

#include "drvstrc.c"

void putstr(unsigned char *string);

unsigned dosexec(char __far *path, char *arg);
void restore_videomode(void);
extern void __far (*driver_videomode)(void);

struct driver *drivers;

extern char *message;

unsigned driver_count = 0;

extern int videodriver;

/*unsigned char device_exists(unsigned driver, unsigned device)
{
  if(device < drivers[driver].dev_count)
    return 1;
  return 0;
}*/

int driver_id(char *name)
{
  int n = driver_count;

  while(n--)
  {
    if(!strcmp(drivers[n].dev_name, name))
      break;
  }

  return n;
}

void set_dosidle_handler();
void reset_dosidle_handler();

inline void drivercall_entry(void)
{
  reset_dosidle_handler();
}

inline void drivercall_ret(void)
{
  set_dosidle_handler();
}

int __far (*driver_func_ptr)();

int call_cmd_func_devnum(unsigned device, void __far *cmd, unsigned func_seg);
#pragma aux call_cmd_func_devnum = \
  "call dword ptr es:driver_func_ptr" \
  parm [bx] [dx ax] [es] value [ax];
int call_cmd_func_sdev(void __far *cmd, unsigned func_seg);
#pragma aux call_cmd_func_sdev = \
  "call dword ptr es:driver_func_ptr" \
  parm [dx ax] [es] value [ax];  
int dev_cmd(unsigned driver, int device, void __far *cmd)
{
  int rc;
  if(drivers[driver].cmd_func)
  {
    drivercall_entry();
    driver_func_ptr = drivers[driver].cmd_func;    
    if(drivers[driver].dev_count >= 1)
      rc = call_cmd_func_devnum(device, cmd, FP_SEG(&driver_func_ptr));
    else if(drivers[driver].dev_count == -1)      
      rc = call_cmd_func_sdev(cmd, FP_SEG(&driver_func_ptr));
    drivercall_ret();
    return rc;
  }
  return -1;
}

int call_read_func_devnum(unsigned device, unsigned func_seg, char *buff, unsigned count);
#pragma aux call_read_func_devnum = \
  "call dword ptr es:driver_func_ptr" \
  parm [ax] [es] [dx bx] [cx] value [ax];
int call_read_func_sdev(unsigned func_seg, char *buff, unsigned count);
#pragma aux call_read_func_sdev = \
  "call dword ptr es:driver_func_ptr" \
  parm [es] [dx bx] [cx] value [ax];  
//int dev_read(unsigned driver, int device)
int dev_read(unsigned driver, int device, char *buff, unsigned count)
{
  int rc;
  if(drivers[driver].read_func)
  {
    drivercall_entry();
    driver_func_ptr = drivers[driver].read_func;        
    if(drivers[driver].dev_count >= 1)    
      rc = call_read_func_devnum(device, FP_SEG(&driver_func_ptr), buff, count);
    else if(drivers[driver].dev_count == -1)
      rc = call_read_func_sdev(FP_SEG(&driver_func_ptr), buff, count);
    drivercall_ret();
    return rc;
  }
  return -1;
}

int call_write_func_devnum(unsigned device, char *buff, unsigned count, unsigned func_seg);
#pragma aux call_write_func_devnum = \
  "call dword ptr es:driver_func_ptr" \
  parm [ax] [dx bx] [cx] [es] value [ax];
int call_write_func_sdev(char *buff, unsigned count, unsigned func_seg);
#pragma aux call_write_func_sdev = \
  "call dword ptr es:driver_func_ptr" \
  parm [dx bx] [cx] [es] value [ax];  
//int dev_write(unsigned driver, int device, unsigned char byte)
int dev_write(unsigned driver, int device, char *buff, unsigned count)
{
  int rc;
  if(drivers[driver].write_func)
  {
    drivercall_entry();
    driver_func_ptr = drivers[driver].write_func;            
    if(drivers[driver].dev_count >= 1)
      rc = call_write_func_devnum(device, buff, count, FP_SEG(&driver_func_ptr));
    else if(drivers[driver].dev_count == -1)      
      rc = call_write_func_sdev(buff, count, FP_SEG(&driver_func_ptr));
    drivercall_ret();
    return rc;
  }
  return -1;
}

int call_open_func_devnum(int device, unsigned mode, unsigned func_seg);
#pragma aux call_open_func_devnum = \
  "call dword ptr es:driver_func_ptr" \
  parm [bx] [ax] [es] value [ax];
int call_open_func_sdev(unsigned mode, unsigned func_seg);
#pragma aux call_open_func_sdev = \
  "call dword ptr es:driver_func_ptr" \
  parm [ax] [es] value [ax];    
int dev_open(unsigned driver, int device, unsigned mode)
{
  int rc;
  if(drivers[driver].open_func)  
  {
    drivercall_entry();
    driver_func_ptr = drivers[driver].open_func;
    if(drivers[driver].dev_count >= 1)    
      rc = call_open_func_devnum(device, mode, FP_SEG(&driver_func_ptr));
    else if(drivers[driver].dev_count == -1)      
      rc = call_open_func_sdev(mode, FP_SEG(&driver_func_ptr));
    drivercall_ret();
    return rc;
  }
  return -1;
}

int call_close_func_devnum(int device, unsigned func_seg);
#pragma aux call_close_func_devnum = \
  "call dword ptr es:driver_func_ptr" \
  parm [ax] [es] value [ax];
int call_close_func_sdev(unsigned func_seg);
#pragma aux call_close_func_sdev = \
  "call dword ptr es:driver_func_ptr" \
  parm [es] value [ax];
int dev_close(unsigned driver, int device)
{
  int rc;
  if(drivers[driver].close_func)
  {
    drivercall_entry();
    driver_func_ptr = drivers[driver].close_func;                    
    if(drivers[driver].dev_count >= 1)    
      rc = call_close_func_devnum(device, FP_SEG(&driver_func_ptr));
    else if(drivers[driver].dev_count == -1)      
      rc = call_close_func_sdev(FP_SEG(&driver_func_ptr));
    drivercall_ret();
    return rc;
  }
  return -1;
}

int call_lseek_func_devnum(int device, int offset, unsigned func_seg);
#pragma aux call_lseek_func_devnum = \
  "call dword ptr es:driver_func_ptr" \
  parm [bx] [ax] [es] value [ax];
int call_lseek_func_sdev(int offset, unsigned func_seg);
#pragma aux call_lseek_func_sdev = \
  "call dword ptr es:driver_func_ptr" \
  parm [ax] [es] value [ax];  
int dev_lseek(unsigned driver, int device, int offset)
{
  int rc;
  if(drivers[driver].lseek_func)
  {
    drivercall_entry();
    driver_func_ptr = drivers[driver].lseek_func;                        
    if(drivers[driver].dev_count >= 1)    
      rc = call_lseek_func_devnum(device, offset, FP_SEG(&driver_func_ptr));
    else if(drivers[driver].dev_count == -1)      
      rc = call_lseek_func_sdev(offset, FP_SEG(&driver_func_ptr));
    drivercall_ret();
    return rc;
  }
  return -1;
}

int call_fsync_func_devnum(int device, unsigned func_seg);
#pragma aux call_fsync_func_devnum =  \
  "call dword ptr es:driver_func_ptr" \
  parm [ax] [es] value [ax];
int call_fsync_func_sdev(unsigned func_seg);
#pragma aux call_fsync_func_sdev =  \
  "call dword ptr es:driver_func_ptr" \
  parm [es] value [ax];
int dev_fsync(unsigned driver, int device)
{
  int rc;
  if(drivers[driver].fsync_func)
  {
    drivercall_entry();
    driver_func_ptr = drivers[driver].fsync_func;                            
    if(drivers[driver].dev_count >= 1)    
      rc = call_fsync_func_devnum(device, FP_SEG(&driver_func_ptr));
    else if(drivers[driver].dev_count == -1)      
      rc = call_fsync_func_sdev(FP_SEG(&driver_func_ptr));
    drivercall_ret();
    return rc;
  }
  return -1;
}

int call_error_func_devnum(int device, unsigned func_seg);
#pragma aux call_error_func_devnum = \
  "call dword ptr es:driver_func_ptr" \
  parm [ax] [es] value [ax];
int call_error_func_sdev(unsigned func_seg);
#pragma aux call_error_func_sdev = \
  "call dword ptr es:driver_func_ptr" \
  parm [es] value [ax];
int dev_geterror(unsigned driver, int device)
{
  int rc;
  if(drivers[driver].error_func)
  {
    drivercall_entry();
    driver_func_ptr = drivers[driver].error_func;
    if(drivers[driver].dev_count >= 1)
      rc = call_error_func_devnum(device, FP_SEG(&driver_func_ptr));
    else if(drivers[driver].dev_count == -1)
      rc = call_error_func_sdev(FP_SEG(&driver_func_ptr));
    drivercall_ret();
    return rc;
  }
  return -1;
}

int unload_driver(unsigned driver)
{
  if(drivers[driver].unload_func)
  {
    (*drivers[driver].unload_func)();
    return 0;
  }
  return 1;
}

void unload_drivers(void)
{
  unsigned n = driver_count;
  while(n--)
    unload_driver(n);
}

int unload_module(char *modname)
{
  unsigned n = driver_count;
  int rc;

  while(n--)
  {
    if(!strcmp(drivers[n].dev_name, modname))
    {
      rc = unload_driver(n);
      driver_count--;

      for(;n < driver_count;n++)
        drivers[n] = drivers[n+1];

      return rc;
    }
  }
  return n;
}
 
void video_mode(unsigned char mode);
extern unsigned char video;
void draw_screen(void);

extern unsigned char _VIDEO_MODE;



unsigned short get_memory_allocation_strategy(void);
#pragma aux get_memory_allocation_strategy = \
"mov al, 0" \
"int 0x58" \
value [ ax ];

void set_memory_allocation_strategy(unsigned char strategy);
#pragma aux set_memory_allocation_strategy = \
"mov al, 1" \
"int 0x58" \
parm [ bx ];


void driver_installer();

int load_driver(char *program, char *modname)
{
  unsigned n = 0;
  unsigned old_driver_count = driver_count;
  char args[3];
  unsigned short memory_allocation_strategy = get_memory_allocation_strategy();
  // find free interrupt
  while(_dos_getvect(++n))
  {
    if(!n)
      return -1;
  }

  sprintf(args, "%X", n);

  reset_dosidle_handler();    
  _dos_setvect(n, driver_installer);
  set_memory_allocation_strategy(0x82);  
  dosexec(program, args);
  set_memory_allocation_strategy(memory_allocation_strategy);
  set_dosidle_handler();

//  draw_screen();  
  _dos_setvect(n, 0);

  if(old_driver_count != (volatile unsigned)driver_count)
  {
    strncpy(drivers[old_driver_count].dev_name, modname, 5);
    drivers[old_driver_count].dev_name[5] = 0;
    return 0;
  }
  else
  {
    restore_videomode();
    draw_screen();
    return -1;
  }
}
