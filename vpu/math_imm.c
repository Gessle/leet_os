static unsigned imm_src(struct vpu *vpu, unsigned flags)
{
  register unsigned temp;
  if(flags & IMM_W_FLAG)
  {
    temp = vpu->ip;
    vpu->ip += 2;
    return *(unsigned short __far*)&vpu->code[vpu->code_segment][temp];
  }
  else
    return vpu->code[vpu->code_segment][vpu->ip++];
}
