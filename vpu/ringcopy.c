// dst = pointer to a ring buffer
static void copy2ring(unsigned char __far *dst, unsigned char __far *src, unsigned dst_offset,
               unsigned dst_buff_size, unsigned count)
{
  unsigned copy_count;
  unsigned dst_and = dst_buff_size - 1;
  unsigned dst_buff_offset;
  unsigned src_offset = 0;

  while(count)
  {
    dst_buff_offset = dst_offset & dst_and;
    if(count > dst_buff_size - dst_buff_offset)
      copy_count = dst_buff_size - dst_buff_offset;
    else copy_count = count;

    _fmemcpy(&dst[dst_buff_offset], &src[src_offset], copy_count);

    src_offset += copy_count;
    dst_offset += copy_count;

    if(count -= copy_count)
      dst_offset = 0;
  }
}

// src = pointer to a ring buffer
// src_offset = offset to start copying from
static void ringcopy(unsigned char __far *dst, unsigned char __far *src, unsigned src_offset, 
              unsigned src_buff_size, unsigned count)
{
  unsigned copy_count;
  unsigned src_and = src_buff_size - 1;
  unsigned src_buff_offset;
  unsigned dst_offset = 0;

  while(count)
  {
    src_buff_offset = src_offset & src_and;
    if(count > src_buff_size - src_buff_offset)
      copy_count = src_buff_size - src_buff_offset;
    else copy_count = count;

    _fmemcpy(&dst[dst_offset], &src[src_buff_offset], copy_count);

    src_offset += copy_count;
    dst_offset += copy_count;

    if(count -= copy_count)
      src_offset = 0;
  }
}
