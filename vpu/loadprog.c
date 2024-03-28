static unsigned short fgetc_wnum(FILE *fp)
{
  unsigned short retval = fgetc(fp);
  retval |= (unsigned short)fgetc(fp) << 8;
  return retval;
}

static void show_seg_info(struct vpu *vpu)
{
  sprintf(message, "Stack size: %u, Code segments: %u, Code size: %u, Data segments: %u, Data size: %u, Interrupts: %u", vpu->stack_size, 
    vpu->code_count, vpu->code_size, vpu->data_count, vpu->data_size, vpu->interrupt_count);
  putstr(message);
}

static int load_program(char *file, struct vpu *vpu)
{
  FILE *fp;
  unsigned n;
  unsigned char infobyte;
  unsigned char *segp;
  unsigned data_count, code_count; 
  char error1[] = "VPU init error.";
  char error2[] = "Invalid header.";

  fp = fopen(file, "rb");

  if(!fp)
  {
    error:
    sprintf(message, "Cannot load %s", file);
    putstr(message);
    return 2;
  }

  // read header
  n = fgetc(fp);

  if((signed short)n == -1)
  {
    fclose(fp);
    putstr(error2);
    goto error;
  }
    

//  sprintf(message, "Program header is %u bytes long", n);
//  putstr(message);      

  while(ftell(fp) <= n)
  {
    infobyte = fgetc(fp);
    if(infobyte & 0x80)
    {
      if(infobyte & 0x02)
        vpu->stack_size = fgetc_wnum(fp);
      else
        vpu->stack_size = fgetc(fp);
    }

    else if(infobyte & 0x40)
    {
      if(infobyte & 0x08)
        code_count = fgetc_wnum(fp);
      else
        code_count = fgetc(fp);

      if(infobyte & 0x02)
        vpu->code_size = fgetc_wnum(fp);
      else
        vpu->code_size = fgetc(fp);
    }

    else if(infobyte & 0x20)
    {
      if(infobyte & 0x08)
        data_count = fgetc_wnum(fp);
      else
        data_count = fgetc(fp);

      if(infobyte & 0x02)
        vpu->data_size = fgetc_wnum(fp);
      else
        vpu->data_size = fgetc(fp);
    }

    else if(infobyte & 0x10)
    {
      if(infobyte & 0x02)
        vpu->interrupt_count = fgetc_wnum(fp);
      else
        vpu->interrupt_count = fgetc(fp);
    }
  }
  
  if(!init_vpu(vpu, vpu->stack_size, vpu->code_size, code_count, vpu->data_size, data_count, vpu->interrupt_count))
  {
    fclose(fp);
    putstr(error1);
    return 1;
  }
  if(debug)
    show_seg_info(vpu);  
  // read data
  n = fgetc_wnum(fp);
  if(n>vpu->data_size)
  {
    fclose(fp);
    putstr(error2);     
    return 3;
  }
  segp = vpu->data[0];
  while(n--)
  {
    *segp++ = fgetc(fp);
  }
  // read code
  n = fgetc_wnum(fp);
  if(n>vpu->code_size)
  {
    fclose(fp);
    putstr(error2);    
    return 4;
  }
  segp = vpu->code[0];
  while(n--)
  {
    *segp++ = fgetc(fp);
  }

  fclose(fp);
  return 0;
}
