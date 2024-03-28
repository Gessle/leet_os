static int vpu_instr_wait(struct vpu *vpu, unsigned flags)
{
  vpu->wait = 1;

  return 3;
}
