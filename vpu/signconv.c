static int vpu_instr_sconvw(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu) & 0x07;
  unsigned char reg1 = vpu_next_code_byte(vpu) & 0x0F;
  signed short *destwp = (signed short*)&vpu->regs[reg0];

  *destwp = *(signed char*)&vpu->byteregs[reg1];

  return 0;
}

static int vpu_instr_sconvb(struct vpu *vpu, unsigned flags)
{
  unsigned char reg0 = vpu_next_code_byte(vpu) & 0x0F;
  unsigned char reg1 = vpu_next_code_byte(vpu) & 0x07;

  vpu->byteregs[reg0] = *(signed char*)&vpu->regs[reg1];

  return 0;
}
