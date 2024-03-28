static int con_stdin_read(void);

static int vpu_instr_getstr(struct vpu *vpu, unsigned flags)
{
  unsigned short data_pointer = vpu->regs[vpu->code[vpu->code_segment][vpu->ip++] & 0x07];
  unsigned short stringlen = vpu->regs[vpu->code[vpu->code_segment][vpu->ip++] & 0x07];
  int c;
  unsigned n = 0;

  char *string = &vpu->data[vpu->data_segment][data_pointer];

  if(!memory_allowed(vpu, (long)data_pointer+stringlen))
  {
    vpu->flags.dsegv = 1;
    return 0;
  }

  if((c = con_stdin_read()) == -1)
  {
    vpu->ip-=3;
    return 0;
  }

  while(n < stringlen)
  {
    string[n++] = c;
    if((c = con_stdin_read()) == -1)
      break;
    if(c == 13) c = 0;
  }

  return 0;
}
