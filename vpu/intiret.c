static int vpu_instr_hardint(struct vpu *vpu, unsigned short vpu_interrupt)
{
  vpu->interrupt_wait = 0;
  
  if(!vpu->interrupts[vpu_interrupt].segment && !vpu->interrupts[vpu_interrupt].address)
    return 0;

  vpu->interrupt_run = 1;

  if(vpu->sp < 3)
  {
    putstr(stackoverflowerror);
    vpu->flags.stackv = 1;
    return 3;
  }

  vpu->stack[vpu->sp--] = *(unsigned short*)&vpu->flags;
  vpu->stack[vpu->sp--] = vpu->ip;
  vpu->stack[vpu->sp--] = vpu->code_segment;

  if(debug)
  {
    sprintf(message, "  Hardware triggering interrupt %u, ", vpu_interrupt);
    output_debug_info(message);
  }

  if(vpu->interrupts[vpu_interrupt].segment >= vpu->code_count)
  {
    putstr("Invalid interrupt vector!");    
    return 0;
  }
  vpu->code_segment = vpu->interrupts[vpu_interrupt].segment;
  vpu->ip = vpu->interrupts[vpu_interrupt].address;

  if(debug)
  {
    sprintf(message, "Jumping to segment 0x%.4X offset 0x%.4X\n", vpu->code_segment, vpu->ip);
    output_debug_info(message);
  }

  return 0;
}

static int vpu_instr_int(struct vpu *vpu, unsigned flags)
{
  unsigned short operand = *(unsigned short*)&vpu->code[vpu->code_segment][vpu->ip];

  if(operand >= vpu->interrupt_count)
  {
    sprintf(message, "Program tried to trigger interrupt %u but interrupt table is only %u long!\n", operand, vpu->interrupt_count);
    putstr(message);
    return 0;
  }

  if(vpu->sp < 3)
  {
    putstr(stackoverflowerror);
    vpu->flags.stackv = 1;        
    return 3;
  }

  vpu->stack[vpu->sp--] = *(unsigned short*)&vpu->flags;
  vpu->stack[vpu->sp--] = vpu->ip+2;
  vpu->stack[vpu->sp--] = vpu->code_segment;

  if(vpu->interrupts[operand].segment >= vpu->code_count)
  {
    putstr("Invalid interrupt vector!");
    return 0;
  }
  vpu->code_segment = vpu->interrupts[operand].segment;
  vpu->ip = vpu->interrupts[operand].address;


  if(debug)
  {
    sprintf(message, "jumping to segment 0x%.4X offset 0x%.4X\n", vpu->code_segment, vpu->ip);
    output_debug_info(message);
  }

  vpu->interrupt_run=1;

  return 0;
}

static int vpu_instr_iret(struct vpu *vpu, unsigned flags)
{
  if(vpu->stack_size - vpu->sp < 4)
  {
    putstr(stackunderflowerror);
    vpu->flags.stackv = 1;
    send_vpu_signal(vpu, SIGSEGV);        
    return 4;
  }

  vpu->code_segment = vpu->stack[++vpu->sp];
  vpu->ip = vpu->stack[++vpu->sp];
  vpu->flags = *(struct vpu_flags*)&vpu->stack[++vpu->sp];


  if(debug)
  {
    sprintf(message, "Returning to segment 0x%.4X offset 0x%.4X\n", vpu->code_segment, vpu->ip);
    output_debug_info(message);
  }
  vpu->interrupt_run = 0;
  vpu->wait = vpu->retwait;

  return 0;
}

static int vpu_instr_setint(struct vpu *vpu, unsigned flags)
{
/*  unsigned short operand0;// = *(unsigned short*)&vpu->code[vpu->code_segment][vpu->ip];
  unsigned short operand1, operand2;*/
/*  vpu->ip+=2;
  operand1 = *(unsigned short*)&vpu->code[vpu->code_segment][vpu->ip];
  vpu->ip+=2;
  operand2 = *(unsigned short*)&vpu->code[vpu->code_segment][vpu->ip];
  vpu->ip+=2;*/
  unsigned operand0 = vpu_next_code_word(vpu);
  unsigned operand1 = vpu_next_code_word(vpu);
  unsigned operand2 = vpu_next_code_word(vpu);

  if(operand0 >= vpu->interrupt_count)
  {
    sprintf(message, "Program tried to set interrupt vector %u but interrupt table is only %u long!\n", operand0, vpu->interrupt_count);
    putstr(message);
    send_vpu_signal(vpu, SIGSEGV);        
    return 4;
  }

  if(debug)
  {
    sprintf(message, "Setting interrupt %u to segment 0x%.4X offset 0x%.4X\n", operand0, operand1, operand2);
    output_debug_info(message);
  }

  vpu->interrupts[operand0].segment = operand1;
  vpu->interrupts[operand0].address = operand2;

  return 0;
}
