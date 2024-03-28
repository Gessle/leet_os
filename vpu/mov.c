inline void *vpu_instr_mov_po(struct vpu *vpu, unsigned char operand_type)
{
  unsigned short operand;
  
  operand = vpu->code[vpu->code_segment][vpu->ip++];

  switch(operand_type & 0x0C)
  {
    // if operand is not an immediate number
    case 0: case 4:
      if(operand_type & 0x03)
        operand |= vpu->code[vpu->code_segment][vpu->ip++] << 8;
      switch(operand_type & 0x03)
      {
        // register
        case 0:
          operand &= 0x0F;
          if(operand_type & 0x04)
          {
            return &vpu->regs[operand];
          }
          return &vpu->byteregs[operand];
  
        // data
        case 1:
          if(!memory_allowed(vpu, operand + !!(operand_type & 0x04)))
            return 0;
          return &vpu->data[vpu->data_segment][operand];        

        // code
        case 2:
          if(operand >= vpu->code_size)
          {
            vpu->flags.codev = 1;
            putstr(segfaulterror);            
            send_vpu_signal(vpu, SIGSEGV);                
            return 0;
          }
          return &vpu->realcode[vpu->code_segment][operand];        
  
        // stack
        case 3:
          if(operand >= vpu->stack_size)
          {
            vpu->flags.stackv = 1;
            putstr(segfaulterror);            
            send_vpu_signal(vpu, SIGSEGV);                
            return 0;
          }
          return &vpu->stack[operand];        
      }

    // operand is a pointer in register
    case 0x08:
      operand &= 0x0F;
      switch(operand_type & 0x03)
      {
        // data
        case 1:    
          if(!memory_allowed(vpu, vpu->regs[operand] + !!(operand & 0x04)))
            return 0;
          return &vpu->data[vpu->data_segment][vpu->regs[operand]];
        // code
        case 2:
          if(vpu->regs[operand&0x07] >= vpu->code_size)
          {
            vpu->flags.codev = 1;
            putstr(segfaulterror);            
            send_vpu_signal(vpu, SIGSEGV);                
            return 0;
          }
          return &vpu->realcode[vpu->code_segment][vpu->regs[operand]];
        // stack
        case 3:
          if(vpu->regs[operand] >= vpu->stack_size)
          {
            vpu->flags.stackv = 1;
            putstr(segfaulterror);            
            send_vpu_signal(vpu, SIGSEGV);                
            return 0;
          }
          return &vpu->stack[vpu->regs[operand]];
      }
    // if operand is an immediate number
    case 0x0C:
      if(operand_type & 0x01)
      {
        vpu->ip++;
        return &vpu->code[vpu->code_segment][vpu->ip -2];      
      }
      return &vpu->code[vpu->code_segment][vpu->ip -1];
  }
  return 0;
}

static int vpu_instr_mov(struct vpu *vpu, unsigned flags)
{
  // operand types: 0x00 = register, 0x01 = data address, 0x02 = code address, 0x03 = stack address
  // Bit 0x04 = wide bit
  // 0x08 = operand is a register that contains a pointer to dereference
  // If bits 0x08 and 0x04 are both set, the operand is immediate number. In that case 0x01 is the wide bit
  unsigned char operand_types = vpu->code[vpu->code_segment][vpu->ip++];
  unsigned char n=0;
  void *operand_as_pointer[2];

  do
  {
    operand_as_pointer[n] = vpu_instr_mov_po(vpu, (operand_types >> (n<<2)) & 0x0F);
    if(!operand_as_pointer[n])
    {
//  putstr("    MOV operand is too big!\n");
      return 0;
    }
  }
  while(++n < 2);

  if(operand_types & 0x04 && !(operand_types & 0x40))
  {
    leftword:        
    if((operand_types & 0xC0) == 0x80) goto bothword;
    *(unsigned short*)operand_as_pointer[0] = *(unsigned char*)operand_as_pointer[1];
  }
  else if(!(operand_types & 0x04) && operand_types & 0x40)
  {
    if((operand_types & 0x0C) == 0x08)
    {
      if((operand_types & 0xC0) == 0xC0 && !(operand_types & 0x10))
        goto leftword;
      else
        goto bothword;
    }
    if(!((operand_types & 0xC0) == 0xC0) || !(operand_types & 0x10))
      *(unsigned char*)operand_as_pointer[0] = *(unsigned short*)operand_as_pointer[1];
    else
      goto bothbyte;
  }
  else if(operand_types & 0x40 && operand_types & 0x04)
  {
    if(!((operand_types & 0xC0) == 0xC0) || operand_types & 0x10)
    {
      bothword:
      *(unsigned short*)operand_as_pointer[0] = *(unsigned short*)operand_as_pointer[1];
    }
    else
      goto leftword;
  }
  else
  {
    bothbyte:
    *(unsigned char*)operand_as_pointer[0] = *(unsigned char*)operand_as_pointer[1];
  }

  if(vpu->code_segment >= vpu->code_count)
    send_vpu_signal(vpu, SIGSEGV);

  return 0;
}
