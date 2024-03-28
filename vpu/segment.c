static int vpu_instr_dseg(struct vpu *vpu, unsigned flags)
{
  unsigned short new_segment = vpu->regs[vpu->code[vpu->code_segment][vpu->ip++] & 0x0F];
  if(!vpu_segment_in_use(vpu, new_segment))
  {
//    sprintf(message, "Data segment %u is unallocated!\n", new_segment);
//    putstr(message);
    vpu->flags.dsegv = 1;
    putstr(segfaulterror);    
    send_vpu_signal(vpu, SIGSEGV);    
    return 0;
  }
  vpu->data_segment = new_segment;
  if(debug)
  {
    sprintf(message, "Data segment is now %u\n", vpu->data_segment);
    output_debug_info(message);
  }
  return 0;
}

