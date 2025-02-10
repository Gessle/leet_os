static int vpu_instr_wait(struct vpu *vpu, unsigned flags)
{
  unsigned char instruction;
  if(flags)
  {
    instruction = vpu_next_code_byte(vpu) & 0x3F;
    flags >>= 2;
    return (*instruction_func_ptr[instruction])(vpu, flags);
  }

  vpu->wait = 1;

  return 3;
}
