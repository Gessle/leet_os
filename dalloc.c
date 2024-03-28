#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>

// Maximum number of memory allocation to keep track of
#define DALLOC_TABLE_SIZE 6000
// Maximum length of a function name
#define FUNC_NAME_LEN 64
// Callstack length. Increase this if you get the "Call stack overflow" error.
#define DEBUG_CALLSTACK_LEN 128
// How long call trace is shown when exiting.
#define CALL_TRACE_LEN 12
// A file that contains list of function names that are not pushed to the call trace.
#define FUNCTION_BLACKLIST_FILE "blacklst.prf"
// Comment this over if you don't want a call trace.
#define TRACE_CALLS
// Comment this over if you don't want serial logging.
//#define SERIAL_DEBUG

// Serial port settings

// 2304 == 50 baud
// 1040 == 110 baud
// 768 == 150 baud
// 384 == 300 baud
// 192 == 600 baud
// 96 == 1200 baud
// 48 == 2400 baud
// 24 == 4800 baud
// 12 == 9600 baud
// 6 == 19200 baud
// 3 == 38400 baud
// 2 == 57600 baud
// 1 == 115200 baud
#define BAUD_DIVISOR 1
// Parity: 0 = no parity, 1 = odd parity, 3 = even parity
#define PARITY 1
// Stop bits
#define STOP 1

// What COM port to use. 0 == COM1, 1 == COM2 ...
#define COM_PORT 0

// use RTS/CTS flow control
#define FLOW_CONTROL

const unsigned short __far *calltrace_serialbase = (unsigned short __far*)0x00400000L;
void make_calltrace_serialport_settings(void)
{
  unsigned mode = 0x3;
  mode |= STOP << 2;
  mode |= PARITY << 4;
  
  // enable DLAB
  outp(calltrace_serialbase[COM_PORT]+3, 0x80);
  // set baud rate
  outp(calltrace_serialbase[COM_PORT]+0, BAUD_DIVISOR);
  outp(calltrace_serialbase[COM_PORT]+1, BAUD_DIVISOR>>8);
  // set data bits, parity and stop bits
  outp(calltrace_serialbase[COM_PORT]+3, mode);
//  // enable FIFO
//  outp(serialbase[port]+2, 0xC7);
//  outp(serialbase[port]+2, 0x07);
  // disable FIFO
  outp(calltrace_serialbase[COM_PORT]+2, 0x00);
  // set interrupt when receiving  
//  outp(serialbase[port]+1, 0x01);
  // set RTS/DSR, IRQ enable
//  outp(serialbase[port]+4, 0x0B);
}

unsigned char debug_callstack_p = 0;

inline unsigned char calltrace_serial_flow_control_send(void)
{
    // RTS/CTS
    // check CTS line
  if(!(inp(calltrace_serialbase[COM_PORT]+6) & 0x10))    return 1;      
  return 0;
}


unsigned char calltrace_serial_write(unsigned char chr)
{
  unsigned char overruns = 0;

#ifdef FLOW_CONTROL
  while(calltrace_serial_flow_control_send());
#endif

  // wait until bit 5 clears  
  while(!(inp(calltrace_serialbase[COM_PORT]+5) & 0x20));

  retry:
  if(overruns++) delay(1);
  if(overruns == 0xFF) return 1;
  outp(calltrace_serialbase[COM_PORT], chr);
  if(inp(calltrace_serialbase[COM_PORT]+5) & 0x02)
    goto retry;  
  
  return 0;
}

void calltrace_serial_output(const char __far *function_name, unsigned len)
{
  unsigned n;
  static char com_port_set = 0;
  if(!com_port_set)
  {
    make_calltrace_serialport_settings();
    com_port_set = 1;
  }

  for(n=debug_callstack_p;n;n--)
    calltrace_serial_write(' ');    

  do
    calltrace_serial_write(function_name[n]);
  while(n++ < len);
}


struct debug_callstack_function
{
  char name[FUNC_NAME_LEN+1];
  void __far *code_ptr;
  unsigned short caller_bp;
  unsigned short this_bp;
};
struct debug_callstack_function call_stack[DEBUG_CALLSTACK_LEN];

void video_mode(unsigned char mode);

struct debug_memory_allocation
{
  void __far *ptr_to_block;
  unsigned size;
  unsigned freed:1;
  unsigned staticarr:1;
};

struct debug_memory_allocation *debug_memory_allocs;
unsigned dalloc_count = 0;
unsigned dalloc_giveup = 0;
unsigned vague_memsets = 0;
unsigned vague_memcpys = 0;
unsigned vague_strcpys = 0;
unsigned vague_strncpys = 0;

char call_trace[CALL_TRACE_LEN][FUNC_NAME_LEN];
unsigned call_trace_index = 0;
char **call_trace_blacklist = 0;
unsigned call_trace_blacklist_len = 0;

void dalloc_abortmsg(char *msg);

int load_call_trace_blacklist(void)
{
  FILE *fp = fopen(FUNCTION_BLACKLIST_FILE, "r");
  if(!fp) return 0;
  call_trace_blacklist = calloc(2, sizeof(char*));
  if(!call_trace_blacklist) goto error;

  do
  {
    call_trace_blacklist = realloc(call_trace_blacklist, (call_trace_blacklist_len +1) * sizeof(char*));
    if(!call_trace_blacklist) goto error;
    call_trace_blacklist[call_trace_blacklist_len] = malloc((FUNC_NAME_LEN+1) * sizeof(char));
    if(!call_trace_blacklist[call_trace_blacklist_len])
    {
      error:
      dalloc_abortmsg("Out of memory while allocating function blacklist table!");
    }
    
  }
  while(fgets(call_trace_blacklist[call_trace_blacklist_len++], FUNC_NAME_LEN+1, fp));
  fclose(fp);
  return 1;
}

int function_name_is_blacklisted(const char __far *function_name, unsigned len)
{
  unsigned n;

  if(!call_trace_blacklist)
    load_call_trace_blacklist();

  n = call_trace_blacklist_len;

  while(n--)
  {
    if(!strncmp(call_trace_blacklist[n], function_name, len))
      return 1;
  }
  return 0;
}


void push_call_trace(const char __far *function, unsigned fname_len)
{
#ifdef SERIAL_DEBUG
  char message[FUNC_NAME_LEN+20];
  sprintf(message, "Call: %.*Fs\n", fname_len, function);
//  calltrace_serial_output(function, fname_len);
  calltrace_serial_output(message, strlen(message));
#endif
  
  memset(call_trace[call_trace_index], 0, FUNC_NAME_LEN);
  strncpy(call_trace[call_trace_index], function, fname_len);
  if(++call_trace_index == CALL_TRACE_LEN)
    call_trace_index = 0;
}

void show_call_trace(void)
{
  unsigned n = call_trace_index+1;
  unsigned functions_on_row = 0;

  puts("Call trace:");

  do
  {
    if(n == CALL_TRACE_LEN)
      n = 0;
    if(*call_trace[n])
    {
      if(!(++functions_on_row % 5))
        puts(call_trace[n]);
      else
        printf("%s, ", call_trace[n]);
    }
  }
  while(n++ != call_trace_index);  
}


void show_call_stack(void)
{
  unsigned n = 0;
  printf("Call stack length: %u\n", debug_callstack_p);

  puts("Call stack:");  
  while(n < debug_callstack_p)
  {
    printf("%p -> %s (caller BP: 0x%.4X, function BP: 0x%.4X)\n", call_stack[n].code_ptr, call_stack[n].name,
      call_stack[n].caller_bp, call_stack[n].this_bp);
    n++;
  }
}

void debug_exit(int status)
{
  unsigned n = dalloc_count;
  unsigned leaked = 0;

  if(dalloc_giveup)
    puts("Too much allocs (or just leaks)");

  else
  {
    printf("Allocations: %u\n", dalloc_count);    
  
    while(n--)
      if(!debug_memory_allocs[n].freed)
        leaked++;
    
    printf("Leaks: %u\n", leaked);
  }

  printf("Vague memsets: %u\n", vague_memsets);
  printf("Vague memcpys: %u\n", vague_memcpys);
  printf("Vague strcpys: %u\n", vague_strcpys);
  printf("Vague strncpys: %u\n", vague_strncpys);      

  show_call_stack();
#ifdef TRACE_CALLS
  show_call_trace();
#endif  
  exit(status);
}

#define exit(x) debug_exit(x)

void show_error_msg(char *errormsg);

void dalloc_abortmsg(char *msg)
{
  if(dalloc_giveup) return;
  getch();
  video_mode(0x03);
  puts(msg);
  exit(0);
}

unsigned init_dallocs(void)
{
  debug_memory_allocs = calloc(DALLOC_TABLE_SIZE, sizeof(struct debug_memory_allocation));
  if(!debug_memory_allocs)
    dalloc_abortmsg("Out of memory");
  return 1;
}

struct debug_memory_allocation *new_dalloc(void)
{
  unsigned n;
  if(!dalloc_count) init_dallocs();

  if(dalloc_giveup)
    return debug_memory_allocs;

  if(dalloc_count == DALLOC_TABLE_SIZE)
  {
    for(n=0;n<DALLOC_TABLE_SIZE;n++)
      if(debug_memory_allocs[n].freed)
        while(n<DALLOC_TABLE_SIZE-1)
        {
          debug_memory_allocs[n] = debug_memory_allocs[n+1];
          n++;
        }
    if(n == DALLOC_TABLE_SIZE)
    {
      dalloc_giveup = 1;
      n=0;
    }
    return &debug_memory_allocs[n];
  }
  
  return &debug_memory_allocs[dalloc_count++];
}

void *debug_malloc(unsigned count)
{
  struct debug_memory_allocation *new_malloc = new_dalloc();

  new_malloc->ptr_to_block = malloc(count);
  new_malloc->size = count;
  return new_malloc->ptr_to_block;
}

void *debug_calloc(unsigned count, unsigned bsize)
{
  struct debug_memory_allocation *new_calloc = new_dalloc();
  
  new_calloc->ptr_to_block = calloc(count, bsize);
  new_calloc->size = count*bsize;  
  return new_calloc->ptr_to_block;
}

void *debug_realloc(void __far *ptr, unsigned size)
{
  struct debug_memory_allocation *new_realloc = new_dalloc();
  unsigned n = dalloc_count;

  if(!ptr)
    dalloc_abortmsg("Tried to reallocate a null pointer!");
  
  while(n--)
    if(debug_memory_allocs[n].ptr_to_block == ptr)
      break;

  if(n == -1 && !dalloc_giveup)
    dalloc_abortmsg("Invalid realloc!");

  else if(n != -1 && !dalloc_giveup)
  {
    if(debug_memory_allocs[n].freed)
      dalloc_abortmsg("Tried to reallocate an already freed memory block!");
    if(debug_memory_allocs[n].size == size)
    {
      show_error_msg("Realloc with no size chg!");
    }
    if(!size)
    {
      show_error_msg("Tried to reallocate null bytes!");
      debug_memory_allocs[n].freed = 1;        
      return realloc(ptr, 0);
    }  
    if(debug_memory_allocs[n].staticarr)
      dalloc_abortmsg("Tried to resize static array!");    

    new_realloc->ptr_to_block = realloc(ptr, size);

    if(!new_realloc->ptr_to_block && size < debug_memory_allocs[n].size)
    {
      show_error_msg("Realloc returned 0 when trying to shrink array...");
      debug_memory_allocs[n].freed = 1;        
      return 0;
    }
  
    new_realloc->size = size;

    if(new_realloc->ptr_to_block)
      debug_memory_allocs[n].freed = 1;  
  
    return new_realloc->ptr_to_block;
  }

  return realloc(ptr, size);
}

void debug_free(void __far *ptr)
{
  unsigned n = dalloc_count;
  
  while(n--)
    if(debug_memory_allocs[n].ptr_to_block == ptr)
      break;

  if(ptr)
  {    
    if(n == -1 && !dalloc_giveup)
    {
      dalloc_abortmsg("Invalid free!");
    }
    else if(n != -1)
    {
      if(debug_memory_allocs[n].freed && !dalloc_giveup)
      {
        dalloc_abortmsg("Double free!");
      }
      if(debug_memory_allocs[n].staticarr)
      {
        dalloc_abortmsg("Tried to free static array!");
      }
      debug_memory_allocs[n].freed = 1;      
    }
  }
  free(ptr);      
}

void *debug_salloc(unsigned count)
{
  struct debug_memory_allocation *new_salloc = new_dalloc();
  
  new_salloc->ptr_to_block = malloc(count);
  new_salloc->size = count;
  new_salloc->staticarr = 1;
  return new_salloc->ptr_to_block;
}

unsigned pointer_in_allocated_block(const void __far *ptr)
{
  unsigned n = dalloc_count;

  while(n--)
  {
    if(ptr >= debug_memory_allocs[n].ptr_to_block && (unsigned long)ptr < (unsigned long)debug_memory_allocs[n].ptr_to_block + debug_memory_allocs[n].size)
      if(!debug_memory_allocs[n].freed)
        break;
  }
  return n;
}

unsigned pointer_in_block(const void __far *ptr, unsigned block)
{
  if(ptr >= debug_memory_allocs[block].ptr_to_block && (unsigned long)ptr < (unsigned long)debug_memory_allocs[block].ptr_to_block + debug_memory_allocs[block].size)
    return 1;
  return 0;
}

void *debug_memset(void __far *dest, int c, unsigned count)
{
  unsigned n;
  char __far *dstptr = dest;

  if(FP_SEG(dest) == 0xA000) // destination is in display memory
    goto noerror;

  if((n = pointer_in_allocated_block(dest)) != -1)
  {
    if(!pointer_in_block((char*)dest + count-1, n))
      dalloc_abortmsg("Invalid memset!");
  }
  else
    vague_memsets++;
  noerror:return memset(dest, c, count);
/*  while(count--) *dstptr++ = c;

  return dest;*/
}

void *debug_memcpy(void __far *dest, const void __far *src, unsigned count)
{
  unsigned n;
  unsigned char __far *dstptr = (char __far*)dest;
  unsigned char __far *srcptr = (char __far*)src;

  if((n = pointer_in_allocated_block(dest)) != -1)
  {
    if(!pointer_in_block((char*)dest + count-1, n))
      dalloc_abortmsg("Invalid memcpy (destination buffer overflow)");    
  }
  else
    vague_memcpys++;

  if((n = pointer_in_allocated_block(src)) != -1)
  {
    if(!pointer_in_block((char*)src + count-1, n))
      dalloc_abortmsg("Invalid memcpy (source buffer overflow)");    
  }
  else
    vague_memcpys++;
    
  return memcpy(dest, src, count);
/*  while(count--)
    *dstptr++ = *srcptr++;

  return dest;*/
}

char *debug_strcpy(char __far *dest, const char __far *src)
{
  unsigned n;
  char __far *dstptr;

  if((n = pointer_in_allocated_block(dest)) != -1)
  {
    if(!pointer_in_block(dest + strlen(src), n))
      if(pointer_in_block(dest + strlen(src)-1, n))
        dalloc_abortmsg("Invalid strcpy (no space for null char in destination?)");
      else
        dalloc_abortmsg("Invalid strcpy (destination buffer overflow)");
  }
  else
    vague_strcpys++;

  if((n = pointer_in_allocated_block(src)) != -1)
  {
    if(!pointer_in_block(src + strlen(src), n))
      if(pointer_in_block(src + strlen(src)-1, n))
        dalloc_abortmsg("Invalid strcpy (no space for null char in source?)");
      else
        dalloc_abortmsg("Invalid strcpy (source buffer overflow)");
  }
  else
    vague_strcpys++;


  return strcpy(dest, src);
}

char *debug_strncpy(char __far *dest, const char __far *src, unsigned count)
{
  unsigned n;

  if((n = pointer_in_allocated_block(dest)) != -1)
  {
    if(!pointer_in_block(dest + count +1, n) && !pointer_in_block(dest + strlen(src), n))
      if(pointer_in_block(dest + count, n))
        dalloc_abortmsg("Invalid strncpy (no space for null char in destination?)");      
      else    
        dalloc_abortmsg("Invalid strncpy (destination buffer overflow)");
  }
  else
    vague_strncpys++;

  if((n = pointer_in_allocated_block(src)) != -1)
  {
    if(!pointer_in_block(src + count +1, n) && !pointer_in_block(src + strlen(src), n))
      if(pointer_in_block(src + count, n))
        dalloc_abortmsg("Invalid strncpy (no space for null char in source?)");      
      else        
        dalloc_abortmsg("Invalid strncpy (source buffer overflow)");
  }
  else
    vague_strncpys++;

  return strncpy(dest, src, count);
}

inline void debug_callstack_push(unsigned len, const char __far *function, void __far *code_ptr, unsigned short caller_bp, unsigned short this_bp)
{
  strncpy(call_stack[debug_callstack_p].name, function, len);
  call_stack[debug_callstack_p].code_ptr = code_ptr;
  call_stack[debug_callstack_p].caller_bp = caller_bp;
  call_stack[debug_callstack_p].this_bp = this_bp;    
  if(++debug_callstack_p == DEBUG_CALLSTACK_LEN)
    dalloc_abortmsg("Call stack overflow!");
#ifdef TRACE_CALLS
  if(!function_name_is_blacklisted(function, len))
    push_call_trace(function, len);
#endif
}

inline void debug_callstack_pop(void)
{
#ifdef SERIAL_DEBUG
  char message[FUNC_NAME_LEN+20];
  if(!function_name_is_blacklisted(call_stack[debug_callstack_p-1].name, strlen(call_stack[debug_callstack_p-1].name)))
  {
    sprintf(message, "Returns: %s\n", call_stack[debug_callstack_p-1].name);
//    calltrace_serial_output(0, 0);
    calltrace_serial_output(message, strlen(message));
  }
#endif  
  debug_callstack_p--;
  
  memset(call_stack[debug_callstack_p].name, 0, FUNC_NAME_LEN);
}

#include "prolog16/prolog16.c"

#define malloc(size) debug_malloc(size)
#define calloc(count, bsize) debug_calloc(count, bsize)
#define realloc(ptr, size) debug_realloc(ptr, size)
#define free(ptr) debug_free(ptr)
#define memset(str, c, count) debug_memset(str, c, count)
#define memcpy(dest, src, count) debug_memcpy(dest, src, count)
#define strcpy(dest, src) debug_strcpy(dest, src)
#define strncpy(dest, src, count) debug_strncpy(dest, src, count)

//#define str[l] "*str = debug_salloc(l)"
