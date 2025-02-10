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
    vpu->compreg = vpu->regs[regid] - src;
    if(vpu->compreg > src) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }
  else
  {
    temp = vpu->byteregs[regid];
    vpu->compreg = vpu->byteregs[regid] - src;
    if(vpu->compreg > temp) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }

  set_overflow(vpu, reg0, vpu->compreg);
  vpu->flags.sign = !!(vpu->compreg & 0x8000);

  return 0;
}

static int vpu_instr_xchg(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu->code[vpu->code_segment][vpu->ip++];
  unsigned char reg1 = vpu->code[vpu->code_segment][vpu->ip++];
  unsigned reg0data, reg1data;

  if(reg0 & 0x80)
    reg0data = vpu->regs[reg0 & 0xF];
  else
    reg0data = vpu->byteregs[reg0 & 0xF];

  if(reg1 & 0x80)
    reg1data = vpu->regs[reg1 & 0xF];
  else
    reg1data = vpu->byteregs[reg1 & 0xF];

  if(reg0 & 0x80)
    vpu->regs[reg0 & 0xF] = reg1data;
  else
    vpu->byteregs[reg0 & 0xF] = reg1data;

  if(reg1 & 0x80)
    vpu->regs[reg1 & 0xF] = reg0data;
  else
    vpu->byteregs[reg1 & 0xF] = reg0data;

  return 0;
}

static int vpu_instr_test(struct vpu *vpu, unsigned flags)
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
    vpu->compreg = vpu->regs[regid] & src;
    if(vpu->compreg > src) vpu->flags.carry = 1;
    else vpu->flags.carry = 0;
  }
  else
  {
    temp = vpu->byteregs[regid];
    vpu->compreg = vpu->byteregs[regid] & src;
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

