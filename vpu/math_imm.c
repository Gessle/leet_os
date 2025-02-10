static unsigned imm_src(struct vpu *vpu, unsigned flags)
{
  register unsigned temp;
  if(flags & IMM_W_FLAG)
  {
/*    temp = vpu->ip;
    vpu->ip += 2;
    return *(unsigned short __far*)&vpu->code[vpu->code_segment][temp];*/
    return vpu_next_code_word(vpu);
  }
  else
//    return vpu->code[vpu->code_segment][vpu->ip++];
    return vpu_next_code_byte(vpu);
}
