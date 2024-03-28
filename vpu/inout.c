static int vpu_instr_in(struct vpu *vpu, unsigned flags)
{
  unsigned char port = vpu->code[vpu->code_segment][vpu->ip++];

  if(port >= vpu->port_count)
  {
    sprintf(message, "Program tried to read unallocated port %u!\n", port);
    putstr(message);
    return 0;
  }

  vpu->regs[0] = vpu->ports[port];
  vpu->ports[port] = -1;

  return 0;
}

static int vpu_instr_out(struct vpu *vpu, unsigned flags)
{
  unsigned char port = vpu->code[vpu->code_segment][vpu->ip++];

  if(port >= vpu->port_count)
  {
    sprintf(message, "Program tried to write to unallocated port %u!\n", port);
    putstr(message);
    return 0;
  }

  vpu->ports[port] = vpu->regs[0];

  return 0;
}
