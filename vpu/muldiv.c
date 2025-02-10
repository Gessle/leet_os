static void mul48(unsigned long *num1, unsigned *num2)
{
  unsigned long temp1 = *num1;
  unsigned temp2 = *num2;
  unsigned of = 0;
  unsigned ofr = 0;
  unsigned long res = 0;
  

  while(1)
  {
    if(temp2 & 1)
    {
      res += temp1;
      if(res < temp1)
        ofr++;
      ofr += of;
    }

    temp2 >>= 1;
    if(!temp2) break;

    of <<= 1;
    of |= !!(temp1 & 0x80000000);
      
    temp1 <<= 1;
  }

  *num1 = res;
  *num2 = ofr;
}

static int vpu_instr_mul32(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char reg1 = vpu_next_code_byte(vpu);
  unsigned char reg2;
  unsigned src;
  unsigned long temp;

  if(flags & IMM_FLAG)
  {
    src = imm_src(vpu, flags);
  }
  else
  {
    reg2 = vpu_next_code_byte(vpu);
    src = vpu->regs[reg2 & 0xF];
  }

  temp = ((unsigned long)vpu->regs[reg0 & 0x0F] << 16) |
         vpu->regs[reg1 & 0x0F];

//  temp *= src;
  mul48(&temp, &src);

  vpu->regs[reg1 & 0x0F] = temp;
  vpu->regs[reg0 & 0x0F] = temp >> 16;
  if(!(flags & IMM_FLAG))
    vpu->regs[reg2 & 0x0F] = src;

  return 0;
}

static int vpu_instr_div32(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char reg1 = vpu_next_code_byte(vpu);
  unsigned char reg2;
  unsigned src;
  unsigned long temp;

  if(flags & IMM_FLAG)
  {
    src = imm_src(vpu, flags);
  }
  else
  {
    reg2 = vpu_next_code_byte(vpu);
    src = vpu->regs[reg2 & 0xF];
  }

  if(!src)
  {
    send_vpu_signal(vpu, SIGFPE);
    return 0;
  }


  temp = ((unsigned long)vpu->regs[reg0 & 0x0F] << 16) |
         vpu->regs[reg1 & 0x0F];

  if(!(flags & IMM_FLAG))
  {
    vpu->regs[reg2 & 0xF] = temp % src;
  }
  temp /= src;

  vpu->regs[reg1 & 0x0F] = temp;
  vpu->regs[reg0 & 0x0F] = temp >> 16;

  return 0;
}

static int vpu_instr_mul(struct vpu *vpu, unsigned flags)
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
    vpu->regs[regid] *= src;
    if(vpu->regs[regid] < temp) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }
  else
  {
    temp = vpu->byteregs[regid];
    vpu->byteregs[regid] *= src;
    if(vpu->byteregs[regid] < temp) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }

  set_overflow(vpu, reg0, temp);
  vpu_set_sign(vpu, reg0);

  return 0;
}

static int vpu_instr_div(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char regid0 = reg0 & 0x0F;
  unsigned char reg1;
  unsigned char regid1;
  unsigned src;
  unsigned mod;

  if(flags & IMM_FLAG)
  {
    src = imm_src(vpu, flags);
  }
  else
  {
    reg1 = vpu->code[vpu->code_segment][vpu->ip++];
    regid1 = reg1 & 0x0F;
    if(reg1 & 0x0F)
      src = vpu->regs[regid1];
    else
      src = vpu->byteregs[regid1];
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
      mod = vpu->regs[regid0] % src;
      if(reg1 & 0x80)
        vpu->regs[regid1] = mod;
      else
        vpu->byteregs[regid1] = mod;
    }
    vpu->regs[regid0] /= src;
  }
  else
  {
    if(!(flags & IMM_FLAG))
    {
      mod = vpu->byteregs[regid0] % src;
      if(reg1 & 0x80)
        vpu->regs[regid1] = mod;
      else
        vpu->byteregs[regid1] = mod;
    }
    vpu->byteregs[regid0] /= src;
  }

  vpu_set_sign(vpu, reg0);

  return 0;
}

static int vpu_instr_idiv(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char regid0 = reg0 & 0x0F;
  unsigned char reg1;
  unsigned char regid1;
  signed src;
  signed mod;

  if(flags & IMM_FLAG)
  {
    src = imm_src(vpu, flags);
  }
  else
  {
    reg1 = vpu->code[vpu->code_segment][vpu->ip++];
    regid1 = reg1 & 0x0F;
    if(reg1 & 0x80)
      src = vpu->regs[regid1];
    else
      src = vpu->byteregs[regid1];
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
      mod = (signed)vpu->regs[regid0] % src;
      if(reg1 & 0x80)
        vpu->regs[regid1] = mod;
      else
        vpu->byteregs[regid1] = mod;
    }
    vpu->regs[regid0] = (signed)vpu->regs[regid0] / src;
  }
  else
  {
    if(!(flags & IMM_FLAG))
    {
      mod = (signed)vpu->byteregs[regid0] % src;
      if(reg1 & 0x80)
        vpu->regs[regid1] = mod;
      else
        vpu->byteregs[regid1] = mod;
    }
    vpu->byteregs[regid0] = (signed)vpu->byteregs[regid0] / src;
  }

  vpu_set_sign(vpu, reg0);

  return 0;
}
