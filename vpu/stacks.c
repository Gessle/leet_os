static int vpu_select_primary_stack(struct vpu *vpu, unsigned flags)
{
  if(vpu->stack == vpu->primary_stack)
    return 0;
  vpu->stack = vpu->primary_stack;
  vpu->ssp = vpu->sp;
  vpu->sp = vpu->psp;

  return 0;
}

static int vpu_select_secondary_stack(struct vpu *vpu, unsigned flags)
{
  if(vpu->stack == vpu->secondary_stack)
    return 0;
  vpu->stack = vpu->secondary_stack; 
  vpu->psp = vpu->sp;
  vpu->sp = vpu->ssp;

  return 0;
}


