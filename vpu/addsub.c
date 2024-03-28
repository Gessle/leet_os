static void vpu_set_sign(struct vpu *vpu, unsigned char reg0)
{
  if(reg0 & 0x80)
    vpu->flags.sign = !!((vpu->compreg = vpu->regs[reg0 & 0x0F]) & 0x8000);
  else
    vpu->flags.sign = !!((vpu->compreg = (signed char)vpu->byteregs[reg0 & 0x0F]) & 0x80);
}

static void set_overflow(struct vpu *vpu, unsigned char reg0, unsigned temp)
{
  if(reg0 & 0x80)
    vpu->flags.overflow = !!((vpu->regs[reg0 & 0x0F] & 0x8000) ^ (temp & 0x8000));
  else
    vpu->flags.overflow = !!((vpu->byteregs[reg0 & 0x0F] & 0x80) ^ (temp & 0x80));
}

static int vpu_instr_add(struct vpu *vpu, unsigned flags)
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
    vpu->regs[reg0 & 0x0F] += src;
    if(vpu->regs[reg0 & 0x0F] < temp) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }
  else
  {
    temp = vpu->byteregs[reg0 & 0x0F];
    vpu->byteregs[reg0 & 0x0F] += src;
    if(vpu->byteregs[reg0 & 0x0F] < temp) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }

  set_overflow(vpu, reg0, temp);
  vpu_set_sign(vpu, reg0);

  return 0;
}

static int vpu_instr_sub(struct vpu *vpu, unsigned flags)
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
    vpu->regs[reg0 & 0x0F] -= src;
    if(vpu->regs[reg0 & 0x0F] > temp) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }
  else
  {
    temp = vpu->byteregs[reg0 & 0x0F];
    vpu->byteregs[reg0 & 0x0F] -= src;
    if(vpu->byteregs[reg0 & 0x0F] > temp) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }

  set_overflow(vpu, reg0, temp);
  vpu_set_sign(vpu, reg0);

  return 0;
}
