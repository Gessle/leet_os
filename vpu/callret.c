static int vpu_instr_call(struct vpu *vpu, unsigned flags)
{
  unsigned short new_segment;

  if(debug)
  {
    sprintf(message, "  CALL FROM segment 0x%.4X address 0x%.4X -> ", vpu->code_segment, vpu->ip);
    output_debug_info(message);
  }

  if(vpu->code[vpu->code_segment][vpu->ip] & 0x80)
  {
    new_segment = *(unsigned short*)&vpu->code[vpu->code_segment][vpu->ip] & 0x7F;
    if(new_segment > vpu->code_count)
    {
      putstr(segfaulterror);
      send_vpu_signal(vpu, SIGSEGV);    
      return 0;
    }
    vpu->ip += 2;
    if(debug)
    {
      sprintf(message, "segment 0x%.4X ", new_segment);
      output_debug_info(message);
    }
  }
  else
    vpu->ip++;

  if(vpu->sp < 2)
  {
    putstr(stackoverflowerror);
    vpu->flags.stackv = 1;        
    return 3;
  }
  vpu->stack[vpu->sp--] = vpu->ip+2;
  vpu->stack[vpu->sp--] = vpu->code_segment;

  vpu->ip = *(unsigned short*)&vpu->code[vpu->code_segment][vpu->ip];
  vpu->code_segment = new_segment;
  if(debug)
  {
    sprintf(message, "offset 0x%.4X", vpu->ip);
    output_debug_info(message);
  }

  return 0;
}

static int vpu_instr_ret(struct vpu *vpu, unsigned flags)
{
  if(vpu->stack_size - vpu->sp < 3)
  {
    putstr(stackunderflowerror);
    vpu->flags.stackv = 1;
    send_vpu_signal(vpu, SIGSEGV);        
    return 0;
  }
  vpu->code_segment = vpu->stack[++vpu->sp];
  vpu->ip = vpu->stack[++vpu->sp];

  if(debug)
  {
    sprintf(message, "  RET to segment 0x%.4X offset 0x%.4X", vpu->code_segment, vpu->ip);
    output_debug_info(message);
  }

  vpu->wait = vpu->retwait;

  return 0;
}
