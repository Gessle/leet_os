static int vpu_instr_and(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu->code[vpu->code_segment][vpu->ip++];
  unsigned char reg1;
  unsigned src;

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
    vpu->regs[reg0 & 0x0F] &= src;
  else
    vpu->byteregs[reg0 & 0x0F] &= src;

  vpu_set_sign(vpu, reg0);  

  return 0;
}

static int vpu_instr_or(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu->code[vpu->code_segment][vpu->ip++];
  unsigned char reg1;
  unsigned src;

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
    vpu->regs[reg0 & 0x0F] |= src;
  else
    vpu->byteregs[reg0 & 0x0F] |= src;
  
  vpu_set_sign(vpu, reg0);    

  return 0;
}

static int vpu_instr_xor(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu->code[vpu->code_segment][vpu->ip++];
  unsigned char reg1;
  unsigned src;

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
    vpu->regs[reg0 & 0x0F] ^= src;
  else
    vpu->byteregs[reg0 & 0x0F] ^= src;

  vpu_set_sign(vpu, reg0);    

  return 0;
}


static int vpu_instr_inv(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu->code[vpu->code_segment][vpu->ip++];

  if(reg0 & 0x80)
    vpu->regs[reg0 & 0x0F] = ~vpu->regs[reg0 & 0x0F];

  else
    vpu->byteregs[reg0 & 0x0F] = ~vpu->byteregs[reg0 & 0x0F];     

  return 0;
}

static int vpu_instr_shl(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu->code[vpu->code_segment][vpu->ip++];
  unsigned char reg1;
  unsigned src;

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
    vpu->regs[reg0 & 0x0F] <<= src;
  else
    vpu->byteregs[reg0 & 0x0F] <<= src;

  return 0;
}

static int vpu_instr_shr(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu->code[vpu->code_segment][vpu->ip++];
  unsigned char reg1;
  unsigned src;

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
    vpu->regs[reg0 & 0x0F] >>= src;
  else
    vpu->byteregs[reg0 & 0x0F] >>= src;

  return 0;
}

