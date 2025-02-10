/*
  This function runs executable *path with arguments *arg
  
  Returns: 0 if success, DOS error code if error
*/

unsigned dos_run_program(char __far *path, void __far *parm_block);
#pragma aux dos_run_program = \
  "push bp" \
  "push si" \
  "push di" \
  "push ds" \
  "mov ds, cx" \
  "mov ax, 0x4B00" \
  "int 0x21" \
  "jnc no_error" \
  "jmp end" \
  "no_error:" \
  "xor ax, ax" \
  "end:" \
  "pop ds" \
  "pop di" \
  "pop si" \
  "pop bp" \
  value [ax] parm [cx dx] [es bx];

unsigned dosexec(char __far *path, char __far *arg)
{
  unsigned short parameter_block[7];
  char dos_args[128] = { 0x00, 0x0D };

  if(arg)
  {
    dos_args[0] = _fstrlen(arg);
    _fmemcpy(&dos_args[1], arg, dos_args[0]);
    dos_args[dos_args[0]+1] = 0x0D;
  }

  _fmemset(parameter_block, 0, 7*sizeof(unsigned short));
  
  parameter_block[0] = 0; // parent environment
  *(char __far**)&parameter_block[1] = dos_args;

  _bheapshrink(_NULLSEG);

  return dos_run_program(path, parameter_block);
}


