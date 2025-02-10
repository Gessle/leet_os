#define MAX_SOCKETS 0x40
//#define MAX_OPEN_PORTS 0x3

/*
  TCP/IP stack driver

  Works as a stack of "devices" that can be written and read from.

  A new socket must be opened by using command opcodes.

  This can be used as an example driver.
*/

#include <stdlib.h>
#include <stdio.h>
#include <i86.h>
#include <dos.h>

void debug_int(void);
#pragma aux debug_int = \
  "int 3";

int socket_read_asm(unsigned socket, char __far *buff, unsigned count);
#pragma aux socket_read_asm = \
  "mov ax, 2" \
  "int 0x61" \
  parm [bx] [es dx] [cx] value [ax];

#pragma aux socket_read parm [ax] [dx bx] [cx] value [ax];
static int __far socket_read(unsigned socket, char __far *buff, unsigned count)
{
  return socket_read_asm(socket, buff, count);
}

int socket_write_asm(unsigned socket, char __far *buf, unsigned);
#pragma aux socket_write_asm = \
  "mov ax, 3" \
  "int 0x61" \
  parm [bx] [es dx] [cx] value [ax];

#pragma aux socket_write parm [ax] [dx bx] [cx] value [ax];
static int __far socket_write(unsigned socket, char __far *buff, unsigned count)
{
  return socket_write_asm(socket, buff, count);
}

int socket_close_asm(unsigned socket);
#pragma aux socket_close_asm = \
  "mov ax, 4" \
  "int 0x61" \
  parm [bx] value [ax]; 

unsigned __based (__segname("_CODE")) open_sockets[MAX_SOCKETS];
unsigned __based (__segname("_CODE")) open_socket_count;

int socket_exists(unsigned socket);
#pragma aux socket_exists = \
  "mov ax, 7" \
  "int 0x61" \
  parm [bx] value [ax];

static int socket_is_open(unsigned socket)
{
  int n = open_socket_count;

  while(n--)
  {
    if(open_sockets[n] == socket)
      if(socket_exists(socket))
        break;
      else // socket is closed by the remote end
      {
        open_socket_count--;
        while(n < open_socket_count)
        {
          open_sockets[n] = open_sockets[n+1];
          n++;
        }
      }
  }
  return n;
}

#pragma aux socket_close parm [ax] value [ax];
static int __far socket_close(unsigned socket)
{
  int i;
  if((i = socket_is_open(socket)) >= 0)
  {
    open_socket_count--;
    while(i < open_socket_count)
    {
      open_sockets[i] = open_sockets[i+1];
      i++;
    }
  }
  return socket_close_asm(socket);
}

#pragma aux socket_open parm [ax] [bx] value [ax];
static int __far socket_open(unsigned mode, unsigned socket)
{
  if(socket_is_open(socket) >= 0 || open_socket_count == MAX_SOCKETS-1)
    return -1;
  
  open_sockets[open_socket_count++] = socket;
  
  return 0;
}

int connect(unsigned port, unsigned char __far *dns, unsigned udp);
#pragma aux connect = \
  "mov ax, 1" \
  "int 0x61" \
  parm [bx] [ds si] [cx] value [ax];

int service(void);
#pragma aux service = \
  "mov ax, 0" \
  "int 0x61" \
  value [ax];


int open_port_asm(unsigned port, unsigned udp);
#pragma aux open_port_asm = \
  "mov ax, 5" \
  "int 0x61" \
  parm [bx] [cx] value [ax];
  
static int open_port(unsigned port, unsigned udp)
{
  return open_port_asm(port, udp);
}

int get_error(void);
#pragma aux get_error = \
  "mov ax, 6" \
  "int 0x61" \
  value [ax];
static int __far call_get_error(void)
{
  return get_error();
}

int get_socket_status(unsigned dev_id);
#pragma aux get_socket_status = \
  "mov ax, 8" \
  "int 0x61" \
  parm [bx] value [ax];

void set_idle_int(void);
#pragma aux set_idle_int = \
  "mov ax, 25" \
  "int 0x61";

#pragma aux tcp_cmd parm [bx] [dx ax] value [ax];
static int __far tcp_cmd(unsigned dev_id, unsigned char __far *cmd)
{
  unsigned short port;
  unsigned char __far *dns;
  
  switch(cmd[0])
  {
    case 0:
      return service();
    case 1: // connect by tcp socket
      port = *(unsigned short __far*)&cmd[1];
      dns = &cmd[3];
      return connect(port, dns, 0);
    case 2: // connect by udp socket
      port = *(unsigned short __far*)&cmd[1];
      dns = &cmd[3];
      return connect(port, dns, 1);
    case 3: // open tcp port
      port = *(unsigned short __far*)&cmd[1];
      return open_port(port, 0);
    case 4: // open udp port
      port = *(unsigned short __far*)&cmd[1];
      return open_port(port, 1);
    case 5: // get socket status
      return get_socket_status(dev_id);
  }
  return -1;
}

unsigned __based(__segname("_CODE")) psp_seg;
void __far * __based(__segname("_CODE")) int21hdl;

unsigned my_psp(void);
#pragma aux my_psp = \
  "mov ah, 0x51" \
  "int 0x21" \
  value [bx] modify [ax];

#pragma aux unload aborts;
static void __far unload(void)
{
  __asm
  {
    mov ax, 26 ; disable idle handler
    int 0x61

    pop ax ; IP
    pop es ; CS
    pushf  ; emulate interrupt call
    push es
    push ax
    cli

    mov es, cs:psp_seg
    mov ah, 0x49
    jmp dword ptr cs:[int21hdl]
  }
}

int dos_close(unsigned handle);
#pragma aux dos_close = \
  "mov ah, 0x3E" \
  "int 0x21" \
  "jc end" \
  "xor ax, ax" \
  "end:" \
  parm [bx] value [ax];

void dos_free(unsigned seg);
#pragma aux dos_free = \
  "mov ah, 0x49" \
  "mov es, bx" \
  "int 0x21" \
  parm [bx] modify [es ax];

// Main function: Install the driver. Above functions are kept in memory.
int main(int argc, char **argv)
{
  // Interrupt to use when calling the driver installer function
  unsigned char driver_install_int;
  // -1-terminating table of functions that this driver provides.
  int __far (__far *function_table[])() =
  {
    (void __far *)tcp_cmd, //                cmd function.
    (void __far *)socket_read, //        read function
    (void __far *)socket_write, //       write function
    (void __far *)socket_open, //       open function 
    (void __far *)socket_close,    // close function
    0, // lseek func
    0, // fsync func
    (void __far *)call_get_error, // error func
    (void __far *)unload, // unload
    (void __far *)-1, // end of function table    
    (void __far *)128, // length of command function argument array.
    (void __far *)0x7FFF // count of devices.
  };
  union REGS regs;
  unsigned n;
  unsigned char __far *psp_ptr;
  
  if(argc != 2)
    return 1;
  if(!_dos_getvect(0x61))
    return 2;

  regs.w.ax = ((unsigned long)function_table) >> 16; // segment of the pointer to the table
  regs.w.bx = (unsigned short)function_table; // offset
  regs.w.cx = 0; // driver type = 0 (no display driver or any special driver)

  driver_install_int = strtoul(argv[1], 0, 0x10); // Interrupt vector that points to the driver installer function
  if(driver_install_int == 0x61)
    return 3;
  int86(driver_install_int, &regs, &regs); // Call the driver installer
  if(regs.w.ax) // if AX != 0, some error happened
  {
    puts("Error while loading TCPIP module. Press any key");
    getchar();
  }
  else
  {
    for(n=0;!dos_close(n);n++);

    psp_ptr = MK_FP(my_psp(), 0);
    n = *(unsigned __far*)&psp_ptr[0x2C];
    dos_free(n);

    psp_seg = FP_SEG(psp_ptr);

    int21hdl = _dos_getvect(0x21);
    set_idle_int();
    _dos_keep(0, ((_FP_OFF( main ) + 0xF) >> 4) + 0x10 ); // exit and leave functions before main() in memory    
  }
  return 1;
}
