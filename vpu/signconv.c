static int vpu_instr_sconvw(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu->code[vpu->code_segment][vpu->ip++];
  unsigned char reg1 = vpu->code[vpu->code_segment][vpu->ip++];
  signed short *destwp = (signed short*)&vpu->regs[reg0 & 0x07];

  *destwp = *(signed char*)&vpu->byteregs[reg1 & 0x0F];

  return 0;
}

static int vpu_instr_sconvb(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu->code[vpu->code_segment][vpu->ip++];
  unsigned char reg1 = vpu->code[vpu->code_segment][vpu->ip++];

  vpu->byteregs[reg0 & 0x0F] = *(signed char*)&vpu->regs[reg1 & 0x07];

  return 0;
}
