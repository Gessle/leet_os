static int vpu_instr_and(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char regid = reg0 & 0x0F;
//  unsigned char reg1;
  unsigned src;

  if(flags & IMM_FLAG)
  {
    src = imm_src(vpu, flags);
  }
  else
  {
    vpu_decode_source(&src, vpu, flags);
  }

  if(reg0 & 0x80)
    vpu->regs[regid] &= src;
  else
    vpu->byteregs[regid] &= src;

  vpu_set_sign(vpu, reg0);  

  return 0;
}

static int vpu_instr_or(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char regid = reg0 & 0x0F;
//  unsigned char reg1;
  unsigned src;

  if(flags & IMM_FLAG)
  {
    src = imm_src(vpu, flags);
  }
  else
  {
    vpu_decode_source(&src, vpu, flags);
  }

  if(reg0 & 0x80)
    vpu->regs[regid] |= src;
  else
    vpu->byteregs[regid] |= src;
  
  vpu_set_sign(vpu, reg0);    

  return 0;
}

static int vpu_instr_xor(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char regid = reg0 & 0x0F;
//  unsigned char reg1;
  unsigned src;

  if(flags & IMM_FLAG)
  {
    src = imm_src(vpu, flags);
  }
  else
  {
    vpu_decode_source(&src, vpu, flags);
  }

  if(reg0 & 0x80)
    vpu->regs[regid] ^= src;
  else
    vpu->byteregs[regid] ^= src;

  vpu_set_sign(vpu, reg0);    

  return 0;
}


static int vpu_instr_inv(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char regid = reg0 & 0x0F;

  if(reg0 & 0x80)
    vpu->regs[regid] = ~vpu->regs[regid];

  else
    vpu->byteregs[regid] = ~vpu->byteregs[regid];     

  return 0;
}

static int vpu_instr_shl(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char regid = reg0 & 0x0F;
//  unsigned char reg1;
  unsigned src;

  if(flags & IMM_FLAG)
  {
    src = imm_src(vpu, flags);
  }
  else
  {
    vpu_decode_source(&src, vpu, flags);
  }

  if(reg0 & 0x80)
    vpu->regs[regid] <<= src;
  else
    vpu->byteregs[regid] <<= src;

  return 0;
}

static int vpu_instr_shcl(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char regid = reg0 & 0x0F;
//  unsigned char reg1;
  unsigned src;

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
    vpu->regs[regid] <<= src - 1;
    vpu->flags.carry = !!(vpu->regs[regid] & 0x8000);
    vpu->regs[regid] <<= 1;
  }
  else
  {
    vpu->byteregs[regid] <<= src - 1;
    vpu->flags.carry = !!(vpu->byteregs[regid] & 0x80);
    vpu->byteregs[regid] <<= 1;
  }

  return 0;
}

static int vpu_instr_rol(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char regid = reg0 & 0x0F;
  unsigned char temp;
  unsigned src;

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
    while(src--)
    {
      temp = !!(vpu->regs[regid] & 0x8000);
      vpu->regs[regid] <<= 1;
      vpu->regs[regid] |= temp;
    }
  }
  else
  {
    while(src--)
    {
      temp = !!(vpu->byteregs[regid] & 0x80);
      vpu->byteregs[regid] <<= 1;
      vpu->byteregs[regid] |= temp;
    }
  }

  return 0;
}

static int vpu_instr_rcl(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char regid = reg0 & 0x0F;
  unsigned char temp;
  unsigned src;

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
    while(src--)
    {
      temp = vpu->flags.carry;
      vpu->flags.carry = !!(vpu->regs[regid] & 0x8000);
      vpu->regs[regid] <<= 1;
      vpu->regs[regid] |= temp;
    }
  }
  else
  {
    while(src--)
    {
      temp = vpu->flags.carry;
      vpu->flags.carry = !!(vpu->byteregs[regid] & 0x80);
      vpu->byteregs[regid] <<= 1;
      vpu->byteregs[regid] |= temp;
    }
  }

  return 0;
}


static int vpu_instr_shr(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char regid = reg0 & 0x0F;
  unsigned src;

  if(flags & IMM_FLAG)
  {
    src = imm_src(vpu, flags);
  }
  else
  {
    vpu_decode_source(&src, vpu, flags);
  }

  if(reg0 & 0x80)
    vpu->regs[regid] >>= src;
  else
    vpu->byteregs[regid] >>= src;

  return 0;
}

static int vpu_instr_shcr(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char regid = reg0 & 0x0F;
  unsigned src;

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
    vpu->regs[regid] >>= src - 1;
    vpu->flags.carry = (vpu->regs[regid] & 0x0001);
    vpu->regs[regid] >>= 1;
  }
  else
  {
    vpu->byteregs[regid] >>= src - 1;
    vpu->flags.carry = (vpu->byteregs[regid] & 0x01);
    vpu->byteregs[regid] >>= 1;
  }

  return 0;
}

static int vpu_instr_ror(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char regid = reg0 & 0x0F;
  unsigned temp;
  unsigned src;

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
    while(src--)
    {
      temp = (vpu->regs[regid])<<15;
      vpu->regs[regid] >>= 1;
      vpu->regs[regid] |= temp;
    }
  }
  else
  {
    while(src--)
    {
      temp = (vpu->byteregs[regid])<<7;
      vpu->byteregs[regid] >>= 1;
      vpu->byteregs[regid] |= temp;
    }
  }

  return 0;
}

static int vpu_instr_rcr(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu);
  unsigned char regid = reg0 & 0x0F;
  unsigned temp;
  unsigned src;

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
    while(src--)
    {
      temp = vpu->flags.carry;
      vpu->flags.carry = (vpu->regs[regid]);
      vpu->regs[regid] >>= 1;
      vpu->regs[regid] |= temp << 15;
    }
  }
  else
  {
    while(src--)
    {
      temp = vpu->flags.carry;
      vpu->flags.carry = (vpu->byteregs[regid]);
      vpu->byteregs[regid] >>= 1;
      vpu->byteregs[regid] |= temp << 7;
    }
  }

  return 0;
}
