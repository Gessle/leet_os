static void message_a_reg(unsigned char reg)
{
  char regstr[3];
  if(reg & 0x80)
    sprintf(regstr, "%cx", 97 + (reg & 0x7F));
  else
  {
    if(reg & 0x01)
      sprintf(regstr, "%ch", 97 + (reg >> 1));
    else
      sprintf(regstr, "%cl", 97 + (reg >> 1));
  }
  sprintf(message, "%s %s", message, regstr);
}

static void message_a_oreg(unsigned operand, unsigned char reg)
{
  sprintf(message, "%s, operand %u register", message, operand);
  message_a_reg(reg);
}

static unsigned a_movdebuginfo_otype(unsigned char type)
{
  char wordstr[] = "16-bit ";
  char bytestr[] = "8-bit ";
  char pointer[] = "pointer";
  if(!(type & 0x8))
  {
    if(type & 0x4)
      sprintf(message, "%s%s", message, wordstr);
    else
      sprintf(message, "%s%s", message, bytestr);
    switch(type & 0x3)
    {
      case 0x00:
        sprintf(message, "%s%s", message, "register");
        return 0x11;
      case 0x1:
        sprintf(message, "%s%s%s", message, "data", pointer);
        return 0x02;
      case 0x2:
        sprintf(message, "%s%s%s", message, "code", pointer);
        return 0x02;
      case 0x3:
        sprintf(message, "%s%s%s", message, "stack", pointer);
        return 0x02;
    }
  }
  else
  {
    if(!(type & 0x4))
    {
      sprintf(message, "%s%s%s", message, pointer, " to dereference");
      return 0x02;
    }
    else
    {
      if(type & 0x1)
        sprintf(message, "%s%s", message, wordstr);
      else
        sprintf(message, "%s%s", message, bytestr);
      sprintf(message, "%s%s", message, "immediate number");
      if(type & 0x1)
        return 0x02;
      return 0x01;
    }
  }
  return 0;
}

static void a_movdebuginfo(unsigned char *i)
{
  unsigned char o = 0;
  unsigned char infobyte = *i++;
  unsigned char infohbyte;
  unsigned char ow;

  do
  {
    sprintf(message, "%s, operand %u type: ", message, o+1);
    infohbyte = infobyte >> (o<<2);
    ow = a_movdebuginfo_otype(infohbyte);
    if((ow & 0x0F) == 1)
    {
      if(!(ow & 0x10))
        sprintf(message, "%s 0x%.2X", message, *i);
      else
        if(infohbyte & 0x4)
          message_a_reg(*i | 0x80);
        else
          message_a_reg(*i);
    }
    else
      sprintf(message, "%s 0x%.4X", message, *(unsigned short*)i);      
    i += ow & 0xF;
  }
  while(++o < 2);
}

static void vpu_step_debuginfo(struct vpu *vpu, unsigned char instruction)
{

  unsigned b = 0;
  unsigned w = 0;
  unsigned o = 0;
  unsigned char *i = &vpu->code[vpu->code_segment][vpu->ip];
  sprintf(message, "Instruction: %s", INSTR[instruction]);

  if(instr_in_array(instruction, VINSTR));

  else if(instr_in_array(instruction, SBYTE_INSTR))
    b=1;
  else if(instr_in_array(instruction, DBYTE_INSTR))
    b=2;
  else if(instr_in_array(instruction, SWORD_INSTR))
    w=1;
  else if(instr_in_array(instruction, DWORD_INSTR))
    w=2;
  else if(instr_in_array(instruction, TWORD_INSTR))
    w=3;

  else if(instruction == OPCODE_MOV)
    a_movdebuginfo(i);

  if(instr_in_array(instruction, BYTEINT_INSTR))
    while(b--)
    sprintf(message, "%s, operand %u: 0x%.2X", message, ++o, *i++);

  else if(instr_in_array(instruction, WREG_INSTR))
    while(b--)
      message_a_oreg(++o, *i++ | 0x80);
  else
    while(b--)
      message_a_oreg(++o, *i++);

  while(w--)
  {
    sprintf(message, "%s, operand %u: 0x%.4X", message, ++o, *(unsigned short*)i);
    i += 2;
  }
  output_debug_info(message);
}
