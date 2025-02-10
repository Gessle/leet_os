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

static void vpu_decode_source(unsigned *src, struct vpu *vpu, unsigned flags)
{
  unsigned offset;
  unsigned char reg;
//  unsigned char target_reg = vpu->code[vpu->code_segment][vpu->ip-1];

  if(!flags)
  {
    reg = vpu_next_code_byte(vpu);
    if(reg & 0x80)
      *src = vpu->regs[reg & 0x0F];
    else
      *src = vpu->byteregs[reg & 0x0F];
  }
  else
  {
    if(flags & DATA_FLAG)
    {
      offset = vpu_next_code_word(vpu);
      if(!(flags & DATA_WORD_FLAG))
      {
        if(memory_allowed(vpu, offset))
          *src = vpu->data[vpu->data_segment][offset];
      }
      else
      {
        if(memory_allowed(vpu, offset+1))
          *src = *(unsigned*)&vpu->data[vpu->data_segment][offset];
      }
    }
    else if(flags == STK_FLAG)
    {
      reg = vpu_next_code_byte(vpu);
      offset = vpu->regs[reg & 0xF];
      if(offset < vpu->stack_size)
        *src = vpu->stack[offset];
      else
      {
        vpu->flags.stackv = 1;
        send_vpu_signal(vpu, SIGSEGV);
      }
    }
  }
}

/*static void vpu_decode_target(unsigned char reg, unsigned **target, struct vpu *vpu)
{
  if(reg & 0x80)
    *target = &vpu->regs[reg & 0x0F];
  else *target = &vpu->byteregs[reg & 0x0F];
}*/

static int vpu_instr_add(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char regid = reg0 & 0x0F;
  unsigned src;
  unsigned temp;

  if(flags & IMM_FLAG)
  {
    src = imm_src(vpu, flags);
  }
  else
  {
    vpu_decode_source(&src, vpu, flags);
  }

  if(reg0 & 0x80)
  {
    temp = vpu->regs[regid];
    vpu->regs[regid] += src;
    if(vpu->regs[regid] < temp) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }
  else
  {
    temp = vpu->byteregs[regid];
    vpu->byteregs[regid] += src;
    if(vpu->byteregs[regid] < temp) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }

  set_overflow(vpu, reg0, temp);
  vpu_set_sign(vpu, reg0);

  return 0;
}

static int vpu_instr_adc(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char regid = reg0 & 0x0F;
  unsigned src;
  unsigned temp;

  if(flags & IMM_FLAG)
  {
    src = imm_src(vpu, flags);
  }
  else
  {
    vpu_decode_source(&src, vpu, flags);
  }

  src += vpu->flags.carry;

  if(reg0 & 0x80)
  {
    temp = vpu->regs[regid];
    vpu->regs[regid] += src;
    if(vpu->regs[regid] < temp || 
       (vpu->flags.carry && !src)) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }
  else
  {
    temp = vpu->byteregs[regid];
    vpu->byteregs[regid] += src;
    if(vpu->byteregs[regid] < temp ||
       (vpu->flags.carry && !src)) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }

  set_overflow(vpu, reg0, temp);
  vpu_set_sign(vpu, reg0);

  return 0;
}

static int vpu_instr_sub(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char regid = reg0 & 0x0F;
  unsigned src;
  unsigned temp;

  if(flags & IMM_FLAG)
  {
    src = imm_src(vpu, flags);
  }
  else
  {
    vpu_decode_source(&src, vpu, flags);
  }

  if(reg0 & 0x80)
  {
    temp = vpu->regs[regid];
    vpu->regs[regid] -= src;
    if(vpu->regs[regid] > temp) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }
  else
  {
    temp = vpu->byteregs[regid];
    vpu->byteregs[regid] -= src;
    if(vpu->byteregs[regid] > temp) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }

  set_overflow(vpu, reg0, temp);
  vpu_set_sign(vpu, reg0);

  return 0;
}

static int vpu_instr_sbb(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char regid = reg0 & 0x0F;
  unsigned src;
  unsigned temp;

  if(flags & IMM_FLAG)
  {
    src = imm_src(vpu, flags);
  }
  else
  {
    vpu_decode_source(&src, vpu, flags);
  }

  src += vpu->flags.carry;

  if(reg0 & 0x80)
  {
    temp = vpu->regs[regid];
    vpu->regs[regid] -= src;
    if(vpu->regs[regid] > temp ||
       (vpu->flags.carry && !src)) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }
  else
  {
    temp = vpu->byteregs[regid];
    vpu->byteregs[regid] -= src;
    if(vpu->byteregs[regid] > temp ||
       (vpu->flags.carry && !src)) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }

  set_overflow(vpu, reg0, temp);
  vpu_set_sign(vpu, reg0);

  return 0;
}
