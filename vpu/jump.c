static int vpu_instr_jump(struct vpu *vpu, unsigned flags)
{
  unsigned short new_segment = vpu->code_segment;

  if(vpu->code[vpu->code_segment][vpu->ip] & 0x80)
  {
    new_segment = *(unsigned short*)&vpu->code[vpu->code_segment][vpu->ip] & 0x7FFF;
    if(new_segment >= vpu->code_count)
    {
      vpu->flags.csegv = 1;
      putstr(segfaulterror);      
      send_vpu_signal(vpu, SIGSEGV);      
      return 0;
    }
    vpu->ip += 2;
  }
  else
    vpu->ip++;

  vpu->ip = *(unsigned short*)&vpu->code[vpu->code_segment][vpu->ip];
  vpu->code_segment = new_segment;

  if(debug)
  {
    sprintf(message, "  JUMP to segment 0x%.4X address 0x%.4X\n", new_segment, vpu->ip);
    output_debug_info(message);
  }

  return 0;
}

