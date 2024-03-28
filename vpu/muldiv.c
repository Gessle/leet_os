static int vpu_instr_mul(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu->code[vpu->code_segment][vpu->ip++];
  unsigned char reg1;
  unsigned src;
  unsigned temp;

  if(flags & IMM_FLAG)
  {
    src = imm_src(vpu, flags);
  }
  else
  {
    reg1 = vpu->code[vpu->code_segment][vpu->ip++];
    if(reg1 & 0x80)
      src = vpu->regs[reg1 & 0xF];
    else
      src = vpu->byteregs[reg1 & 0xF];
  }

  if(reg0 & 0x80)
  {
    temp = vpu->regs[reg0 & 0x0F];
    vpu->regs[reg0 & 0x0F] *= src;
    if(vpu->regs[reg0 & 0x0F] < temp) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }
  else
  {
    temp = vpu->byteregs[reg0 & 0x0F];
    vpu->byteregs[reg0 & 0x0F] *= src;
    if(vpu->byteregs[reg0 & 0x0F] < temp) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }

  set_overflow(vpu, reg0, temp);
  vpu_set_sign(vpu, reg0);

  return 0;
}

static int vpu_instr_div(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu->code[vpu->code_segment][vpu->ip++];
  unsigned char reg1;
  unsigned src;
  unsigned mod;

  if(flags & IMM_FLAG)
  {
    src = imm_src(vpu, flags);
  }
  else
  {
    reg1 = vpu->code[vpu->code_segment][vpu->ip++];
    if(reg1 & 0x80)
      src = vpu->regs[reg1 & 0xF];
    else
      src = vpu->byteregs[reg1 & 0xF];
  }

  if(!src)
  {
    send_vpu_signal(vpu, SIGFPE);
    return 0;
  }

  if(reg0 & 0x80)
  {
    if(!(flags & IMM_FLAG))
    {
      mod = vpu->regs[reg0 & 0x0F] % src;
      if(reg1 & 0x80)
        vpu->regs[reg1 & 0x0F] = mod;
      else
        vpu->byteregs[reg1 & 0x0F] = mod;
    }
    vpu->regs[reg0 & 0x0F] /= src;
  }
  else
  {
    if(!(flags & IMM_FLAG))
    {
      mod = vpu->byteregs[reg0 & 0x0F] % src;
      if(reg1 & 0x80)
        vpu->regs[reg1 & 0x0F] = mod;
      else
        vpu->byteregs[reg1 & 0x0F] = mod;
    }
    vpu->byteregs[reg0 & 0x0F] /= src;
  }

  vpu_set_sign(vpu, reg0);

  return 0;
}

static int vpu_instr_idiv(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu->code[vpu->code_segment][vpu->ip++];
  unsigned char reg1;
  signed src;
  signed mod;

  if(flags & IMM_FLAG)
  {
    src = imm_src(vpu, flags);
  }
  else
  {
    reg1 = vpu->code[vpu->code_segment][vpu->ip++];
    if(reg1 & 0x80)
      src = vpu->regs[reg1 & 0xF];
    else
      src = vpu->byteregs[reg1 & 0xF];
  }

  if(!src)
  {
    send_vpu_signal(vpu, SIGFPE);
    return 0;
  }

  if(reg0 & 0x80)
  {
    if(!(flags & IMM_FLAG))
    {
      mod = (signed)vpu->regs[reg0 & 0x0F] % src;
      if(reg1 & 0x80)
        vpu->regs[reg1 & 0x0F] = mod;
      else
        vpu->byteregs[reg1 & 0x0F] = mod;
    }
    vpu->regs[reg0 & 0x0F] = (signed)vpu->regs[reg0 & 0x0F] / src;
  }
  else
  {
    if(!(flags & IMM_FLAG))
    {
      mod = (signed)vpu->byteregs[reg0 & 0x0F] % src;
      if(reg1 & 0x80)
        vpu->regs[reg1 & 0x0F] = mod;
      else
        vpu->byteregs[reg1 & 0x0F] = mod;
    }
    vpu->byteregs[reg0 & 0x0F] = (signed)vpu->byteregs[reg0 & 0x0F] / src;
  }

  vpu_set_sign(vpu, reg0);

  return 0;
}
