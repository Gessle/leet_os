static void inc_debug_info(unsigned newvalue)
{
  sprintf(message, "New unsigned value is %u", newvalue);
  output_debug_info(message);
}

static int vpu_instr_inc(struct vpu *vpu, unsigned flags)
{
  unsigned char reg = vpu->code[vpu->code_segment][vpu->ip++];

  if((reg &= 0x8F) & 0x80)
  {
    reg &= 0x0F;
    vpu->regs[reg]++;
    if(!vpu->regs[reg])
      vpu->flags.overflow=1;
    if(debug)
    {
      inc_debug_info(vpu->regs[reg]);
    }
  }
  else
  {
    vpu->byteregs[reg]++;
    if(!vpu->byteregs[reg])
      vpu->flags.overflow=1;
    if(debug)
    {
      inc_debug_info(vpu->byteregs[reg]);
    }
  }
  return 0;
}

static int vpu_instr_dec(struct vpu *vpu, unsigned flags)
{
  unsigned char reg = vpu->code[vpu->code_segment][vpu->ip++];
  if((reg &= 0x8F) & 0x80)
  {
    reg &= 0x0F;
    vpu->regs[reg]--;
    if(vpu->regs[reg] == 0xFFFF)
      vpu->flags.overflow=1;
    if(debug)
    {
      inc_debug_info(vpu->regs[reg]);
    }
  }
  else
  {
    vpu->byteregs[reg]--;
    if(vpu->byteregs[reg] == 0xFF)
      vpu->flags.overflow=1;
    if(debug)
    {
      inc_debug_info(vpu->byteregs[reg]);
    }
  }
  return 0;
}
