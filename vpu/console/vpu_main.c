#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
//#include <conio.h>
//#include <i86.h>
//#include <time.h>
//#include <process.h>
//#include <errno.h>
#include <sys/types.h>
#include <direct.h>
//#include <dos.h>
#include <malloc.h>
#include <dos.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>

//#include "..\..\dalloc.c"
#include "..\..\pragmas.c"
#include "..\..\extdlc.c"
#include "..\..\xpmparse\xpmstrct.c"
#include "..\..\drvmod\drvstrc.c"

//unsigned int farstrlen(char __far *str);
//#define strlen(str) farstrlen(str)
//#include "string.c"

extern struct vpu_hw_int *vpu_hardware_int_table;
extern unsigned vpu_hwint_count;
static void remove_vpu_hw_int(struct vpu *vpu, unsigned int vpu_int);

extern unsigned *cursor_row;
extern unsigned *cursor_col;
extern unsigned char VPUCON_LINES;
#include "..\..\condef.c"
//#define SCR_ROWS VPUCON_LINES
unsigned char SCR_ROWS;
#define SCR_COLS VPUCON_COLS
int chdir( const char *path ); 
void refresh(void);
void putstr(unsigned char *string);
void printchar(unsigned char chr);
void scroll_up(void);
void clear_row(unsigned row);
void inc_row(void);
void clear_console(void);
void print_text(unsigned row, unsigned col, unsigned char *str);
void move(unsigned row, unsigned col);
void mvaddch(unsigned row, unsigned col, unsigned char chr);
void del_window(struct prog_window *window);
unsigned char vpu_has_active_window(struct vpu *vpu);
int vpuconsole_getch(void);
void vpucon_caret(unsigned row, unsigned col, unsigned char remove);
void nomemory(void);
void wait_refresh(void);
extern struct prog_window *running_console;
extern unsigned steps;
void close_serialport(unsigned char port);
static unsigned vpu_fd_close(struct vpu *vpu, unsigned fd);
static void vpu_dd_close(struct vpu *vpu, unsigned dd);
static int load_program(char *file, struct vpu *vpu);
void show_error_msg(char *errormsg);

#include "../../constrct.c"

FILE *debugfp = 0;

static void output_debug_info(char *str)
{
  if(debugfp)
    fprintf(debugfp, "%s\n", str);
  else
    putstr(str);
}

//#define mvprintw(row, col, str) print_text(row, col, str)

static void mvprintw(unsigned row, unsigned col, unsigned char *str)
{
  unsigned oldcol = *cursor_col;
  unsigned oldrow = *cursor_row;
  print_text(row, col, str);
  move(oldrow, oldcol);
}

/*inline void mvprintw(unsigned row, unsigned col, unsigned char *str)
{
  print_text(row, col, str);
}*/

char message[100];
char hexstr[6];
char *smessage;


void code_editor(struct console_struct *);

unsigned char debug = 0;

unsigned console = 1;

/*struct console_status
{
  unsigned compiled:1;
};

struct console_status console_status;*/

#define COMMAND_LEN VPUCON_COLS

//#include "output.c"


#include "../assemble/globals.c"
#include "../assemble/asbstrct.c"

#include "../vpuglob.c"
#include "../vpu_init.c"

#include "../opcodes.c"
#include "../instr.c"
#include "../assemble/data.c"
#include "../assemble/parsemov.c"
#include "../assemble/basic.c"
#include "../assemble/symbols.c"
#include "../assemble/parshead.c"
#include "../assemble/saveprog.c"

#include "../step.c"

struct text_content
{
  unsigned char **asm_code;
  unsigned count;
};

//struct text_content asm_content;

struct text_content *asm_content;

#include "save.c"

//char history[HISTORY_COUNT][COMMAND_LEN];
//unsigned char history[][COMMAND_LEN];
unsigned char (*history)[COMMAND_LEN];
// ncurses version
// todo: conio version

static void push_history(char *str)
{
  int n = HISTORY_COUNT-1;
  int m;

  while(n--)
  {
    m=0;
    do
    {
      history[n+1][m] = history[n][m];
    }
    while(history[n][m++]);
  }

  m=0;
  do
  {
    history[0][m] = str[m];
  }
  while(str[m++]);
}

static void get_history(unsigned r, char *retval)
{
  unsigned n;

  if(!history[r][0]) return;
  n=0;  
  do
  {
    retval[n] = history[r][n];
  }
  while(history[r][n++]);

  *cursor_col = (*cursor_col = n-1);

//  clear_row(*cursor_row);

//  mvprintw(cursor_row, 0, retval);
}

static char *getcommand(struct console_struct *con_struct, unsigned char *retval)
{
//  unsigned char *retval = con_struct->command;
  int c;
  unsigned s;

  history = con_struct->history;  

  if(con_struct->newcommand)
  {
    retval[0] = 0;
    con_struct->newcommand=0;
    con_struct->history_n = 0;
  }

  if((c = vpuconsole_getch()) == -1)
    goto end;

  if(c != 13)
  {
    if(!c)
      switch(c = vpuconsole_getch())
      {
        case 72: // up
          if(con_struct->history_n < HISTORY_COUNT-1)
            get_history(con_struct->history_n++, retval);
        break;
        case 80: // down
          if(con_struct->history_n > 1)
            get_history(--con_struct->history_n -1, retval);
        break;
        case 75: // left
          if(*cursor_col)
            --*cursor_col;
        break;
        case 77: // right
          if(retval[*cursor_col])
            ++*cursor_col;
        break;
        case 83: // delete
          for(s=*cursor_col;retval[s];s++)
            retval[s] = retval[s+1];
//          mvaddch(*cursor_row, s-1, ' ');
          break;
        }
    else
      switch(c)
      {
        case 8:
          if(*cursor_col)
          {
            for(s=--*cursor_col;retval[s];s++)
              retval[s] = retval[s+1];
//            mvaddch(*cursor_row, s-1, ' ');
            
          }
        break;
        default:
          for(s=0;retval[s];s++);
          if(s<COMMAND_LEN-1 && *cursor_col<COMMAND_LEN-1)
          {
            for(s=COMMAND_LEN-1;s>*cursor_col;s--)
              retval[s] = retval[s-1];
            retval[(*cursor_col)++] = c;
          }
      }
    clear_row(*cursor_row);
    mvprintw(*cursor_row, 0, retval);
    move(*cursor_row, *cursor_col);         
  }
  else //if(c == 13)
  {
    push_history(retval);
    inc_row();
    con_struct->newcommand=1;
    return retval;
  }
  end:return 0;
}

static void show_variable(char *variable)
{
  struct symbol *symbol = get_symbol_struct(variable, 0);
  char message[80];

  if(!symbol)
  {
    sprintf(message, "Undefined symbol %s", variable);
    putstr(message);
    return;
  }
  sprintf(message, "Segment: 0x%.4X, Address: 0x%.4X, Type: 0x%X, Value: ", symbol->segment, symbol->address, symbol->type);
  if(!(symbol->type & 0x04))
    sprintf(message, "%s0x%.2X", message, data[symbol->segment][symbol->address]);
  else
    sprintf(message, "%s0x%.4X", message, *(unsigned short*)&data[symbol->segment][symbol->address]);
  putstr(message);
}

static void show_unsigned_data(char *address, unsigned show_word)
{
  unsigned short segment = parse_number(address);;
  unsigned short offset;
  unsigned short word;
  char message[80];

  address = skip_word(address);
  address = skip_whitespaces(address);
  offset = parse_number(address);

  if(show_word)  
  {
    word = *(unsigned short*)&data[segment][offset];
    sprintf(message, "%.4X:%.4X Dec: %u, hex: 0x%.4X", segment, offset, word, word);
  }

  else
  {
    word = data[segment][offset];
    sprintf(message, "%.4X:%.4X Dec: %u, hex: 0x%.2X", segment, offset, word, word);
  }

  putstr(message);
}

static void list_symbols(void)
{
  unsigned c = symbol_count;
  char message[80];

  sprintf(message, "%u symbols", c);
  putstr(message);  

  while(c--)
  {
    sprintf(message, "%s:", symbols[c].name);
    putstr(message);
    show_variable(symbols[c].name);
  }
}

static void print_status(struct vpu *vpu)
{
  char message[80];

  putstr("Registers:");
  sprintf(message, "AX: 0x%.4X, BX: 0x%.4X, CX: 0x%.4X, DX: 0x%.4X", vpu->regs[0], vpu->regs[1], vpu->regs[2], vpu->regs[3]);
  putstr(message);
  sprintf(message, "EX: 0x%.4X, FX: 0x%.4X, GX: 0x%.4X, HX: 0x%.4X", vpu->regs[4], vpu->regs[5], vpu->regs[6], vpu->regs[7]);
  putstr(message);
  sprintf(message, "CS: 0x%.4X, IP: 0x%.4X, DS: 0x%.4X, SP: 0x%.4X", vpu->code_segment, vpu->ip, vpu->data_segment, vpu->sp);
  putstr(message);
  sprintf(message, "Compare reg: %i", vpu->compreg);
  putstr(message);
  sprintf(message, "Flags: 0x%.2X", *(unsigned short*)&vpu->flags);
  putstr(message);
  sprintf(message, "       (DSEGV: %u, DATAV: %u, CSEGV: %u, CODEV: %u,", vpu->flags.dsegv, vpu->flags.datav, vpu->flags.csegv, vpu->flags.codev);
  putstr(message);
  sprintf(message, "       STACKV: %u, OVERFLOW: %u, SIGN: %u, CARRY: %u)", vpu->flags.stackv, vpu->flags.overflow, vpu->flags.sign, vpu->flags.carry);
  putstr(message);
}

static void reset_vpu(struct vpu *vpu)
{
  unsigned n;
  for(n=VPU_REGISTER_COUNT;n--;)
    vpu->regs[n] = 0;
  vpu->code_segment=0;
  vpu->ip=0;
  vpu->data_segment=0;
  vpu->sp=vpu->ssp=vpu->psp=vpu->stack_size-1;
  vpu->stack=vpu->primary_stack;
  vpu->compreg=0;
//  vpu->flags = (struct vpu_flags) { 0x0000 };
  memset(&vpu->flags, 0, sizeof(struct vpu_flags));
}

static void list_stack(struct vpu *vpu, char *base)
{
  unsigned short c = vpu->stack_size;
  unsigned short list_base;
//  char message[40];
  if(!(*base))
    list_base = vpu->sp;
  else
    list_base = parse_number(base+1);

  while(--c > list_base)
  {
    sprintf(message, "%u: 0x%.4X", c, vpu->stack[c]);
    putstr(message);
  }
}

static char compile_asm(struct vpu *vpu)
{
  struct return_string *retval;
  char response[100];
  unsigned n;
  unsigned char instr_hex[6];
//  char pass=0;
  pass=0;

  build:
  line_num = 0;

  codep_segment = 0;
  codep_offset = 0;  

  putstr("Assembling...");

  while(line_num < asm_content->count)
  {
    while(!asm_content->asm_code[line_num][0])
    {
      if(++line_num == asm_content->count) goto end;
    }
    switch(start_instr(asm_content->asm_code[line_num++], &retval))
    {
      case 0:
        sprintf(response, "Generated %u bytes long instruction: ", retval->length);
        for(n=0;n<retval->length;n++)
        {
          vpu->code[codep_segment][codep_offset++] = retval->str[n];
          sprintf(instr_hex, "0x%.2X ", retval->str[n]);
          strcat(response, instr_hex);
        }
        if(debug)
          putstr(response);
        free(retval);
      break;
      case -1:
        return 1;
    }
    end:;
  }
  if(!pass++)
  {
    if(debug)
      putstr("First pass complete!");
    goto build;
  }
  return 0;
}

unsigned char vpu_run_program(struct console_struct *con_struct)
{
  struct vpu *vpu;
  asm_content = con_struct->asm_content;
  if(!con_struct->compiled)
    if(compile_asm(con_struct->vcpu))
    {
      putstr("Error while assembling");
      return 1;
    }
    else
    {
      con_struct->compiled = 1;
      putstr("Running program...");
    }
  if(!con_struct->vcpu->wait)
  {
    vpu = vpu_recursive(con_struct->vcpu);
    if(vpu)
    {
      if(!process_is_stopped(vpu))
      {
        if(debug)
          vpu_step_debugoutput(vpu);
        vpu_step(vpu);
        vpu_getwindowkeys(vpu);
      }
      return 0;
    }
  }
  return 1;
}

static void show_bytecode(char *arguments, struct vpu *vpu, unsigned char data)
{
  unsigned segment = parse_number(arguments);
  unsigned address;
  unsigned count;
  char hexbyte[50];
  char *symbolname;
  unsigned operands = 0;
  unsigned char n;

  arguments = skip_word(arguments);
  arguments = skip_whitespaces(arguments);
  address = parse_number(arguments);
  arguments = skip_word(arguments);
  arguments = skip_whitespaces(arguments);
  count = parse_number(arguments);

  while(count--)
  {
    if(data && (symbolname = get_addr_symbol(segment, address, 0x01))
      || !data && (symbolname = get_addr_symbol(segment, address, 0x02)))
    {
      sprintf(hexbyte, "%s:", symbolname);
      putstr(hexbyte);
    }
    if(data)
    {
/*      if((symbolname = get_addr_symbol(segment, address)) && !(define_symbol_type(0, symbolname) & 0x06))
      {
        sprintf(hexbyte, "%s:", symbolname);
        putstr(hexbyte);
      }*/
      if(vpu->data[segment][address] >= 0x20 && vpu->data[segment][address] <= 0x7E)
        sprintf(hexbyte, "0x%.4X:0x%.4X 0x%.2X %c", segment, address, vpu->data[segment][address], vpu->data[segment][address]);
      else
        sprintf(hexbyte, "0x%.4X:0x%.4X 0x%.2X", segment, address, vpu->data[segment][address]); 
    }
    else
    {
/*      if((symbolname = get_addr_symbol(segment, address)) && define_symbol_type(0, symbolname) == 0x0A)
      {
        sprintf(hexbyte, "%s:", symbolname);
        putstr(hexbyte);
      }*/
      if(!operands && vpu->code[segment][address] < instruction_count)
      {
        sprintf(hexbyte, "0x%.4X:0x%.4X 0x%.2X %s", segment, address, vpu->code[segment][address], INSTR[vpu->code[segment][address]]);
        if(instr_in_array(vpu->code[segment][address], VINSTR));
        else if(instr_in_array(vpu->code[segment][address], SBYTE_INSTR)) operands = 1;
        else if(instr_in_array(vpu->code[segment][address], DBYTE_INSTR) || instr_in_array(vpu->code[segment][address], SWORD_INSTR)) operands = 2;
        else if(instr_in_array(vpu->code[segment][address], DWORD_INSTR)
          || vpu->code[segment][address] == OPCODE_CALL) operands = 4;
        else if(vpu->code[segment][address] == OPCODE_JUMP)
        {
          if(!vpu->code[segment][address+1]) operands = 3;
          else operands = 4;
        }
        else if(vpu->code[segment][address] == OPCODE_MOV)
        {
          operands = 3;
          n=1;
          do
          {
            switch((vpu->code[segment][address+1] >> (n*4)) & 0x0F)
            {
              case 0x0D: case 0x01: case 0x02: case 0x03: case 0x05: case 6: case 7:
                operands++;
              default: break;
            }
          }
          while(n--);
/*          if(vpu->code[segment][address+1] & 0x80 && vpu->code[segment][address+1] & 0x70 != 0x60) operands++;
          if(vpu->code[segment][address+1] & 0x08 && vpu->code[segment][address+1] & 0x07 != 0x06) operands++;*/
        }
        else if(instr_in_array(vpu->code[segment][address], TWORD_INSTR)) operands = 6;
      }
      else
      {
        sprintf(hexbyte, "0x%.4X:0x%.4X 0x%.2X", segment, address, vpu->code[segment][address]);
        if(operands) operands--;
      }
    }
    address++;
    putstr(hexbyte);
  }
}

static void toggle_debug(char *filename)
{
//  if(!debug) debug++;
//  else debug--;
  debug = ~debug;
  if(filename)
    if(debug)
      debugfp = fopen(filename, "wt");
    else
    {
      fclose(debugfp);
      debugfp = 0;
    }
}

char response_ready[] = "Ready.";

static void reset_console_vpu(void)
{
  struct console_struct *con_struct = running_console->function_status;
  con_struct->vcpu = con_struct->orig_vpu;
}

static inline void set_console_vpu(char *pid)
{
  struct console_struct *con_struct = running_console->function_status;
  unsigned vpu_pid = parse_number(pid);
  if(!(con_struct->vcpu = pid_to_vpu(vpu_pid)))
    reset_console_vpu();  
}

static char *command_handler(char *command, struct vpu *vpu)
{
  struct return_string *retval;

  static char invalidcommand[] = "Unknown command or instruction.";
  static char nofilename[] = "No file name specified.";
  static char response[100];
  char instr_hex[6];
  unsigned n;

  if(!strcmp("exit", command))
    return 0;

  else if(!strcmp("pid", command))
  {
    sprintf(message, "%u", vpu->pid);
    putstr(message);
  }

  else if(!strncmp("pid", command, 3))
    set_console_vpu(&command[4]);

  else if(!strncmp("debug", command, 5))
  {
    if(!command[5])
      toggle_debug(0);
    else
      toggle_debug(&command[6]);
  }

  else if(!strncmp("load", command, 4))
  {
    if(strlen(command) > 5)
      load_asm(&command[5]);
    else return nofilename;
  }

  else if(!strncmp("save", command, 4))
  {
    if(strlen(command) > 5)    
      save_asm(&command[5]);
    else return nofilename;
  }

  else if(!strncmp("code", command, 4))
    show_bytecode(&command[5], vpu, 0);

  else if(!strncmp("data", command, 4))
    show_bytecode(&command[5], vpu, 1);

  else if(!strcmp("step", command))
  {
//    vpu_run_program(vpu, step);
    return (char*)2;
  }

  else if(!strcmp("run", command))
  {
    vpu->code_segment = 0;
    vpu->ip = 0;
    vpu->wait = 0;
    reset_console_vpu(); 
    return (char*)3;
//    vpu_run_program(vpu, begin);
  }

  else if(!strcmp("continue", command))
  {
//    vpu_run_program(vpu, next);
    vpu->wait = 0;
    reset_console_vpu();    
    return (char*)3;
  }

  else if(!strcmp("edit", command))
  {
//    code_editor();
    clear_console();
    refresh();
    return (char*)1;
  }

  else if(!strcmp("lssym", command))
    list_symbols();

  else if(!strcmp("status", command))
    print_status(vpu);

  else if(!strcmp("reset", command))
    reset_vpu(vpu);

  else if(!strncmp("stack", command, 5))
    list_stack(vpu, &command[5]);

  else if(!strncmp("memword", command, 7))
    show_unsigned_data(&command[8], 1);

  else if(!strncmp("membyte", command, 7))
    show_unsigned_data(&command[8], 0);


  else if(command[0] == '$')
    show_variable(&command[1]);

  // if not any internal command
  else switch(start_instr(command, &retval))
  {
    case 0:
      sprintf(response, "Generated %u bytes long instruction: ", retval->length);
      for(n=0;n<retval->length;n++)
      {
//      bcode[codep_offset++] = retval->str[n];
        sprintf(instr_hex, "0x%.2X ", retval->str[n]);
        strcat(response, instr_hex);
      }
      putstr(response);
      sprintf(response, "Executing instruction...");      
      putstr(response);
      vpu_console_instr(vpu, retval->str);
      free(retval);
      return response_ready;
    case -1:
      return invalidcommand;
//    case 1:
//      return response_ready;
  }
  return response_ready;
}

unsigned char commandline(struct console_struct *con_struct)
{
  char *r;

  asm_content = con_struct->asm_content;

//  r = command_handler(getcommand(), vpu);
  if(r = getcommand(con_struct, con_struct->command))
    if(!(r = command_handler(r, con_struct->vcpu)))
      return 0;

  else if(r == (char*)1)
    con_struct->editmode = 1;

  else if(r == (char*)2)
    con_struct->runmode=1; // step
  else if(r == (char*)3)
    con_struct->runmode=2; // run

  else
    putstr(r);

//  move(*cursor_row, 0);
  return 1;
//  exit:puts("Exiting...");
}

static void list_code(unsigned view_row)
{
  unsigned n = view_row+SCR_ROWS-2;

  clear_console();
  refresh();
  do
  {
    if(n < asm_content->count)
      print_text(n-view_row, 0, asm_content->asm_code[n]);
  }
  while(n-- > view_row);
}

static int add_asm_row(unsigned row)
{
  unsigned s=asm_content->count++;
  void *new_pointer;

  new_pointer = realloc(asm_content->asm_code, asm_content->count*sizeof(char*));
  if(!new_pointer) return 1;
  asm_content->asm_code = new_pointer;

  new_pointer = calloc(SCR_COLS+1, sizeof(char));
  if(!new_pointer) return 1;

  for(;s>row;s--)
    asm_content->asm_code[s] = asm_content->asm_code[s-1];
  asm_content->asm_code[s] = new_pointer;

  return 0;
}

static void delete_asm_row(unsigned row)
{
  unsigned s;

  free(asm_content->asm_code[row]);
  asm_content->count--;

  for(s=row;s < asm_content->count;s++)
    asm_content->asm_code[s] = asm_content->asm_code[s+1];

  asm_content->asm_code = realloc(asm_content->asm_code, asm_content->count *sizeof(char*));
}

static void code_editor_draw_screen(unsigned view_row, unsigned caret_row, unsigned caret_col)
{
  char status_line[SCR_COLS+1];

  vpucon_caret(*cursor_row, *cursor_col, 1);

  clear_row(caret_row-view_row);
  print_text(caret_row-view_row, 0, asm_content->asm_code[caret_row]);

  clear_row(SCR_ROWS-1);
  sprintf(status_line, "row: %u, col: %u", caret_row, caret_col);
  print_text(SCR_ROWS-1, 0, status_line);
        
  move(caret_row-view_row, caret_col);
}

void code_editor(struct console_struct *con_struct)
{
  unsigned view_row = con_struct->view_row;
  unsigned caret_row = con_struct->caret_row;
  unsigned caret_col = con_struct->caret_col;
  int c;
  unsigned s;

  asm_content = con_struct->asm_content;

  if(con_struct->editmode == 1)
  {
    list_code(view_row);
    code_editor_draw_screen(view_row, caret_row, caret_col);
    con_struct->editmode=2;
  }

  if((c = vpuconsole_getch()) == -1)
    return;

  if(!c)
    switch((c = vpuconsole_getch()))
    {
      case 75: // left arrow
        if(caret_col) caret_col--;
        break;
      case 71: // home
        caret_col=0;
        break;
      case 77: // right arrow
        if(caret_col<SCR_COLS-1) caret_col++;
        break;
      case 79: // end
        for(caret_col=0;asm_content->asm_code[caret_row][caret_col];caret_col++);
        break;
      case 72: // up arrow
        if(caret_row) caret_row--;
        break;
      case 80: // down arrow
        if(caret_row < asm_content->count-1) caret_row++;
        break;
      case 73: // page up
        for(s=SCR_ROWS;s-- && caret_row;caret_row--);
        break;
      case 81: // page down
        for(s=SCR_ROWS;s-- && caret_row < asm_content->count-1;caret_row++);
        break;
      case 83: // delete
        for(s=caret_col;asm_content->asm_code[caret_row][s];s++)
          asm_content->asm_code[caret_row][s] = asm_content->asm_code[caret_row][s+1];
        if(!asm_content->asm_code[caret_row][0] && caret_row<asm_content->count-1)
        {
          delete_asm_row(caret_row);
          list_code(view_row);
        }
      break;
    }

  else switch(c)
  {
    case 27: // esc
      con_struct->editmode=0;
      clear_console();
      refresh();
      return;

    case 8: // backspace
      if(caret_col)
        for(s=--caret_col;asm_content->asm_code[caret_row][s];s++)
          asm_content->asm_code[caret_row][s] = asm_content->asm_code[caret_row][s+1];
      else if(!asm_content->asm_code[caret_row][0] && caret_row)
      {
        delete_asm_row(caret_row--);
        list_code(view_row);                
      }
    break;
    case 13: // enter
      if(caret_col)
        add_asm_row(++caret_row);
      else
        add_asm_row(caret_row++);
      list_code(view_row);                
      caret_col=0;
      break;
    default:
      for(s=0;asm_content->asm_code[caret_row][s];s++);
      if(s<SCR_COLS-1 && caret_col<SCR_COLS-1)
      {
        for(s=SCR_COLS-1;s>caret_col;s--)
          asm_content->asm_code[caret_row][s] = asm_content->asm_code[caret_row][s-1];
        asm_content->asm_code[caret_row][caret_col++] = c;
        for(s=0;s<caret_col;s++)
          if(!asm_content->asm_code[caret_row][s])
            asm_content->asm_code[caret_row][s] = ' ';
      }
  }
  con_struct->compiled = 0;
/*  while(caret_row < view_row)
    view_row--;*/
/*  while(caret_row > view_row+SCR_ROWS-2)
    view_row++;*/    
  if(caret_row < view_row)
    view_row=caret_row;
  else if(caret_row > view_row+SCR_ROWS-2)
    view_row = caret_row-SCR_ROWS+2;
  else
  {
    code_editor_draw_screen(view_row, caret_row, caret_col);
    goto end;
  }
  list_code(view_row);  
  code_editor_draw_screen(view_row, caret_row, caret_col);
  end:
  con_struct->view_row = view_row;
  con_struct->caret_row = caret_row;
  con_struct->caret_col = caret_col;  
}

#include "../loadprog.c"

static int init_stdio(struct console_struct *console)
{
  struct file_struct **file_structs = console->vcpu->file_struct;

  file_structs[2]->is_ioport = file_structs[1]->is_ioport = \
    file_structs[0]->is_ioport = 12;
  file_structs[2]->open = file_structs[1]->open = file_structs[0]->open = 1;

  return init_envs(console->vcpu);
}

static int console_init_asm(struct console_struct *console)
{
  console->symbols = calloc(2, sizeof(struct symbol)); 
  if(!console->symbols) return 1;

  console->asm_content = calloc(1, sizeof(struct text_content));
  if(!console->asm_content) return 1;

  console->asm_content->count = 1;
  console->asm_content->asm_code = calloc(console->asm_content->count+2, sizeof(char*));
  if(!console->asm_content->asm_code) return 1;
  console->asm_content->asm_code[0] = calloc(SCR_COLS+1, sizeof(char));
  if(!console->asm_content->asm_code[0]) return 1;

  return 0;
}

int new_console(struct console_struct *retval, char *filename)
{
  char vpu_welcome_msg[] = "Virtual CPU Console v. 0.9 beta";

  retval->orig_vpu = retval->vcpu = spawn_new_vpu();

  retval->con_stdin.buff = calloc(32, sizeof(char));
  retval->con_stdin.len = 32;
  retval->con_stdout.buff = calloc(32, sizeof(char));
  retval->con_stdout.len = 32;  
  retval->con_stderr.buff = calloc(32, sizeof(char));
  retval->con_stderr.len = 32;  

  if(!retval->vcpu || !retval->con_stdin.buff || !retval->con_stdout.buff || !retval->con_stderr.buff)
  {
     free(retval->con_stdin.buff);
     retval->con_stdin.buff=0;
     free(retval->con_stdout.buff);
     retval->con_stdout.buff=0;
     free(retval->con_stderr.buff);
     retval->con_stderr.buff=0;
     return 1;
  }

  retval->lines = VPUCON_LINES;

  if(!filename)
  {
    if(!init_vpu(retval->vcpu, 512, 2048, 1, 4096, 1, 256))
      return 1; 
    if(!init_data_segs(retval->vcpu))
      return 1;
    show_seg_info(retval->vcpu);
    if(init_stdio(retval)) return 1;
    putstr(vpu_welcome_msg);
    putstr(response_ready);
  }

  else
  {
    retval->runmode=3;    
    switch(load_program(filename, retval->vcpu))
    {
      case 2:
        return 2;
      case 1:
        return 1;
      case 3:
        putstr("Too small data segment!");
        return 2;
      case 4:
        putstr("Too small code segment!");
        return 2;
    }
    if(init_stdio(retval))
      return 1;
    return 0;
  }

  return console_init_asm(retval);
}

static int core_debug(struct console_struct *console)
{
  if(!debug)
    return 1;
  if(console_init_asm(console))
    return 1;
  console->runmode = 0;
  console->vcpu->signal = SIGSHOWCON;
  process_vpu_signal(console->vcpu);
  return 0;
}

void delete_console(struct console_struct *console)
{
  delete_vpu(console->orig_vpu);
  if(console->runmode < 3)
  {
    if(console->asm_content)
    {
      while(console->asm_content->count--)
        free(console->asm_content->asm_code[console->asm_content->count]);
      free(console->asm_content->asm_code);
      free(console->asm_content);
    }
    free(console->symbols);
  }
  free(console->con_stdin.buff);
  free(console->con_stderr.buff);
  free(console->con_stdout.buff);  
  running_console = 0;
}
