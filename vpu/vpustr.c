static int vpu_instr_memcpy(struct vpu *vpu, unsigned flags)
{
  unsigned slen0, slen1;
  if(debug)
  {
    sprintf(message, "Instruction: MEMCPY %u bytes, %.4X:%.4X => %.4X:%.4X", vpu->regs[2], vpu->regs[0], vpu->regs[1], vpu->regs[3], vpu->regs[4]);
    output_debug_info(message);
  }

  if(!(slen0 = segment_exists(vpu, vpu->regs[0])) || !(slen1 = segment_exists(vpu, vpu->regs[3])))
  {
    vpu->flags.dsegv = 1;
    return 0;
  }
  if(slen0 < vpu->regs[1] + vpu->regs[2] - 1 || slen1 < vpu->regs[4] + vpu->regs[2] - 1)
  {
    segfault(vpu);
    return 0;
  }

  memmove(&vpu->data[vpu->regs[3]][vpu->regs[4]], &vpu->data[vpu->regs[0]][vpu->regs[1]], vpu->regs[2]);

  vpu->regs[4] += vpu->regs[2];
  vpu->regs[1] += vpu->regs[2];
  vpu->regs[2] = 0;

  return 0;
}

static int vpu_instr_memset(struct vpu *vpu, unsigned flags)
{
  unsigned char copy_byte;
  unsigned slen;
  if(debug)
  {
    sprintf(message, "  Instruction: MEMSET %u bytes, dl => %.4X:%.4X", vpu->regs[2], vpu->regs[0], vpu->regs[1]);
    output_debug_info(message);
  }

  if(!(slen = segment_exists(vpu, vpu->regs[0])))
  {
    vpu->flags.dsegv = 1;
    return 0;
  }

  if(slen < vpu->regs[1] + vpu->regs[2] - 1)
  {
    return 0;
  }

  copy_byte = vpu->byteregs[6];

  memset(&vpu->data[vpu->regs[0]][vpu->regs[1]], copy_byte, vpu->regs[2]);

  vpu->regs[1] += vpu->regs[2];
  vpu->regs[2] = 0;

  return 0;
}

static int vpu_instr_memchr(struct vpu *vpu, unsigned flags)
{
  unsigned char needle;
  unsigned char *ptr;
  unsigned char *buff;
  unsigned slen;

  if(debug)
  {
    sprintf(message, "  Instruction: MEMCHR %u bytes, %.4X:%.4X", vpu->regs[2], vpu->regs[0], vpu->regs[1]);
    output_debug_info(message);
  }

  if(!(slen = segment_exists(vpu, vpu->regs[0])))
  {
    vpu->flags.dsegv = 1;
    return 0;
  }

  if(slen < vpu->regs[1] + vpu->regs[2] - 1)
  {
    return 0;
  }

  needle = vpu->byteregs[6];

  buff = &vpu->data[vpu->regs[0]][vpu->regs[1]];

  ptr = memchr(buff, needle, vpu->regs[2]);
  if(!ptr)
  {
    vpu->regs[1] += vpu->regs[2];
    vpu->regs[2] = 0;
  }
  else
  {
    vpu->regs[1] += ptr - buff;
    vpu->regs[2] -= ptr - buff;
  }

  return 0;
}

static int vpu_instr_memcmp(struct vpu *vpu, unsigned flags)
{
  unsigned slen0, slen1;

  if(debug)
  {
    sprintf(message, "  Instruction: MEMCMP %u bytes, %.4X:%.4X <> %.4X:%.4X", vpu->regs[2], vpu->regs[0], vpu->regs[1], vpu->regs[3], vpu->regs[4]);
    output_debug_info(message);
  }

  if(!(slen0 = segment_exists(vpu, vpu->regs[0])) || !(slen1 = segment_exists(vpu, vpu->regs[3])))
  {
    vpu->flags.dsegv = 1;
    return 0;
  }
  if(slen0 < vpu->regs[1] + vpu->regs[2] - 1 || slen1 < vpu->regs[4] + vpu->regs[2] - 1)
  {
    segfault(vpu);
    return 0;
  }

  do
  {
    if(vpu->data[vpu->regs[0]][vpu->regs[1]] != vpu->data[vpu->regs[3]][vpu->regs[4]])
      break;
    vpu->regs[1]++;
    vpu->regs[4]++;
  }
  while(--vpu->regs[2]);

  return 0;
}
