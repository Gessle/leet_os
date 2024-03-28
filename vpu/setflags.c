static int vpu_instr_zeroflags(struct vpu *vpu, unsigned flags)
{
  memset(&vpu->flags, 0, sizeof(struct vpu_flags));

  return 0;
}
