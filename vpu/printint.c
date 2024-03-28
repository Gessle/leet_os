static int vpu_instr_printuint(struct vpu *vpu, unsigned flags)
{
  unsigned char operand = vpu->code[vpu->code_segment][vpu->ip++];

  if(operand & 0x80)
      sprintf(message, "%u", vpu->regs[operand & 0x0F]);
  else
      sprintf(message, "%u", vpu->byteregs[operand & 0x0F]);
  putstr(message);

  return 3;
}

static int vpu_instr_printsint(struct vpu *vpu, unsigned flags)
{
  unsigned char operand = vpu->code[vpu->code_segment][vpu->ip++];

  if(operand & 0x80)
      sprintf(message, "%i", *(short*)&vpu->regs[operand & 0x0F]);
  else
      sprintf(message, "%i", *(signed char*)&vpu->byteregs[operand & 0x0F]);
  putstr(message);

  return 3;
}
