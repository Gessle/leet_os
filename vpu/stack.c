static int vpu_instr_push(struct vpu *vpu, unsigned flags)
{
  unsigned char reg = vpu_next_code_byte(vpu);
  unsigned short *nongpregs[4];
  nongpregs[0] = &vpu->code_segment;
  nongpregs[1] = &vpu->ip;
  nongpregs[2] = &vpu->data_segment;
  nongpregs[3] = &vpu->sp;

  if(!vpu->sp || vpu->sp >= vpu->stack_size)
  {
    putstr(stackoverflowerror);
    send_vpu_signal(vpu, SIGSEGV);    
    return 4;
  }
  else
  {
    if(reg & 0x08)
      vpu->stack[vpu->sp--] = *nongpregs[reg & 0x03];
    else
      vpu->stack[vpu->sp--] = vpu->regs[reg & 0x07];
  }
  return 0;
}

static int vpu_instr_pop(struct vpu *vpu, unsigned flags)
{
  unsigned char reg = vpu_next_code_byte(vpu);
  unsigned short *nongpregs[4];
  nongpregs[0] = &vpu->code_segment;
  nongpregs[1] = &vpu->ip;
  nongpregs[2] = &vpu->data_segment;
  nongpregs[3] = &vpu->sp;

  if(vpu->sp >= vpu->stack_size - 1)
  {
    putstr(stackunderflowerror);
    send_vpu_signal(vpu, SIGSEGV);        
    return 4;
  }
  else
  {
    if(reg & 0x08)
      *nongpregs[reg & 0x03] = vpu->stack[++vpu->sp];
    else
      vpu->regs[reg & 0x07] = vpu->stack[++vpu->sp];
  }
  return 0;
}
