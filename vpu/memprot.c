static unsigned segment_exists(struct vpu *vpu, unsigned seg)
{
  unsigned n;
  if(seg < vpu->data_count) return vpu->data_size;
  for(n = vpu->dynsegment_count;n--;)
    if(vpu->dynsegments[n].segment == seg)
      return vpu->dynsegments[n].length;
  return 0;
}

static void segfault(struct vpu *vpu)
{
  vpu->flags.datav = 1;
  putstr(segfaulterror);
  send_vpu_signal(vpu, SIGSEGV);
}

static unsigned char memory_allowed(struct vpu *vpu, unsigned long offset)
{
  unsigned n;

  if((offset >> 16) || (unsigned short)offset >= segment_exists(vpu, vpu->data_segment))
  {
    segfault(vpu);
    return 0;
  }

  return 1;
}

static int is_subpath(char *path, char *iwd, char *cwd)
{
  char *ptr;
  int depth = 0;
  unsigned iwdlen = strlen(iwd);
  
  if(path[1] == ':' || path[0] == '\\')
  {
    if(strncmp(path, iwd, strlen(iwd)))
      return 0;
  }

  if(strlen(cwd) > iwdlen)
    for(ptr=&cwd[iwdlen];ptr=strchr(ptr, '\\');depth++,ptr++);
      
  ptr = path;
  do
  {
    if(*ptr)
    {
      if(*(unsigned short*)ptr == ('.' | ('.' << 8)))
        depth--;
      else if(*ptr != '.') depth++;
    }
  }
  while((ptr = strchr(ptr, '\\')+1)-1);
  
  if(depth < 0)
    return 0;
    
  return 1;
}
