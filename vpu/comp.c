static void condjump_debug(unsigned char res)
{
  char condition_satisfied[] = "Condition satisfied, performing jump";
  char condition_unsatisfied[] = "Condition not satisfied";
  if(res)
    smessage = condition_satisfied;
  else
    smessage = condition_unsatisfied;

  output_debug_info(message);
}

static int vpu_instr_cmp(struct vpu *vpu, unsigned flags)
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
    vpu->compreg = vpu->regs[reg0 & 0x0F] - src;
    if(vpu->compreg > src) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }
  else
  {
    temp = vpu->byteregs[reg0 & 0x0F];
    vpu->compreg = vpu->byteregs[reg0 & 0x0F] - src;
    if(vpu->compreg > temp) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }

  set_overflow(vpu, reg0, vpu->compreg);
  vpu->flags.sign = !!(vpu->compreg & 0x8000);

  return 0;
}

static int vpu_instr_test(struct vpu *vpu, unsigned flags)
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
    vpu->compreg = vpu->regs[reg0 & 0x0F] & src;
    if(vpu->compreg > src) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }
  else
  {
    temp = vpu->byteregs[reg0 & 0x0F];
    vpu->compreg = vpu->byteregs[reg0 & 0x0F] & src;
    if(vpu->compreg > temp) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }

  set_overflow(vpu, reg0, vpu->compreg);
  vpu->flags.sign = !!(vpu->compreg & 0x8000);

  return 0;
}

static int vpu_instr_je(struct vpu *vpu, unsigned flags)
{
  if(!vpu->compreg)
  {
    vpu->ip = *(unsigned short*)&vpu->code[vpu->code_segment][vpu->ip];
    if(debug)
    {
      condjump_debug(1);
    }
  }
  else
  {
    vpu->ip+=2;
    if(debug)
    {
      condjump_debug(0);
    }
  }

  return 0;
}

static int vpu_instr_jne(struct vpu *vpu, unsigned flags)
{
  if(vpu->compreg)
  {
    vpu->ip = *(unsigned short*)&vpu->code[vpu->code_segment][vpu->ip];
    if(debug)
    {
      condjump_debug(1);
    }
  }
  else
  {
    vpu->ip+=2;
    if(debug)
    {
      condjump_debug(0);
    }
  }

  return 0;
}

static int vpu_instr_jg(struct vpu *vpu, unsigned flags)
{
  if(vpu->compreg > 0)
  {
    vpu->ip = *(unsigned short*)&vpu->code[vpu->code_segment][vpu->ip];
    if(debug)
    {
      condjump_debug(1);
    }
  }
  else
  {
    vpu->ip+=2;
    if(debug)
    {
      condjump_debug(0);
    }
  }

  return 0;
}

static int vpu_instr_jl(struct vpu *vpu, unsigned flags)
{
  if(vpu->compreg < 0)
  {
    vpu->ip = *(unsigned short*)&vpu->code[vpu->code_segment][vpu->ip];
    if(debug)
    {
      condjump_debug(1);
    }
  }
  else
  {
    vpu->ip+=2;
    if(debug)
      condjump_debug(0);
  }

  return 0;
}


static int vpu_instr_jge(struct vpu *vpu, unsigned flags)
{
  if(vpu->compreg >= 0)
  {
    vpu->ip = *(unsigned short*)&vpu->code[vpu->code_segment][vpu->ip];
    if(debug)
    {
      condjump_debug(1);
    }
  }
  else
  {
    vpu->ip+=2;
    if(debug)
      condjump_debug(0);
  }

  return 0;
}

static int vpu_instr_jle(struct vpu *vpu, unsigned flags)
{
  if(vpu->compreg <= 0)
  {
    vpu->ip = *(unsigned short*)&vpu->code[vpu->code_segment][vpu->ip];
    if(debug)
    {
      condjump_debug(1);
    }
  }
  else
  {
    vpu->ip+=2;
    if(debug)
      condjump_debug(0);
  }

  return 0;
}

static int vpu_instr_jcz(struct vpu *vpu, unsigned flags)
{
  if(!vpu->regs[2])
  {
    vpu->ip = *(unsigned short*)&vpu->code[vpu->code_segment][vpu->ip];
    if(debug)
    {
      condjump_debug(1);
    }
  }
  else
  {
    vpu->ip+=2;
    if(debug)
      condjump_debug(0);
  }

  return 0;
}

static int vpu_instr_jfs(struct vpu *vpu, unsigned flags)
{
  unsigned short operand0 = *(unsigned short*)&vpu->code[vpu->code_segment][vpu->ip];
  vpu->ip+=2;


  if(*(unsigned short*)&vpu->flags & operand0)
  {
    vpu->ip = *(unsigned short*)&vpu->code[vpu->code_segment][vpu->ip];
    if(debug)
    {
      condjump_debug(1);
    }
  }
  else
  {
    vpu->ip+=2;
    if(debug)
      condjump_debug(0);
  }

  return 0;
}

static int vpu_instr_jfns(struct vpu *vpu, unsigned flags)
{
  unsigned short operand0 = *(unsigned short*)&vpu->code[vpu->code_segment][vpu->ip];
  vpu->ip+=2;

  if(!(*(unsigned short*)&vpu->flags & operand0))
  {
    vpu->ip = *(unsigned short*)&vpu->code[vpu->code_segment][vpu->ip];
    if(debug)
    {
      condjump_debug(1);
    }
  }
  else
  {
    vpu->ip+=2;
    if(debug)
      condjump_debug(0);
  }

  return 0;
}
