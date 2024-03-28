/*char *types[] =
{
  "int",
  "ptr"
}*/

static int init_data_segs(struct vpu *vpu)
{
  if(!vpu)
  {
    data = calloc(1, sizeof(char*));
    if(!data) return 0;
    data_allocated = 1024;
    data[0] = calloc(data_allocated, sizeof(char));
    if(!data[0]) return 0;
    return 1;
  }
  else
  {
    data = vpu->data;
    data_allocated = vpu->data_size;
  }
  return 1;
}

static void *allocate_more_data(void)
{
  if(console)
  {
    datap_offset = 0;
    putstr("Data segment full!");
    return (void*)1;
  }
  data_allocated += 1024;
  data[datap_segment] = realloc(data[datap_segment], data_allocated  * sizeof(char));
  return data[datap_segment];
//  if(!data[datap_segment]) return 0;
//  return 1;
}

static void check_data_size()
{
  if(datap_offset+1 >= data_allocated)
    if(!allocate_more_data())
    {
      putstr(outofmemory);
      abort;
    }
}

static int new_data_segment(void)
{
  data_count++;
  data = realloc(data, data_count * sizeof(char*));
  if(!data) return 0;
  data[data_count-1] = calloc(data_allocated, sizeof(char));
  if(!data[data_count-1]) return 0;
  return 1;
}

static short getwordvalue(char **instr)
{
  short retval = 0;
  if(detect_number(*instr))
  {
    retval = parse_number(*instr);
  }
  else if(**instr == '\'')
  {
    ++*instr;
    retval = parse_char(instr);
  }
  *instr = skip_word(*instr);
  return retval;
}

static void add_byte(short value, unsigned char type)
{
  char *sbytep = (char*)&data[datap_segment][datap_offset];
  check_data_size();
  
  if(type & 0x06 == 0x06)
    *sbytep = value;
  else
    data[datap_segment][datap_offset] = value;
  
  datap_offset++;
}

static unsigned char define_byte(char *instr, unsigned char data_type)
{
  char byte_name[SYMBOL_NAME_LENGTH];
  int value;
  

  copy_str(byte_name, instr);

  if(!define_symbol(byte_name, data_type))
    return 0;  
  
  instr = skip_word(instr);
  instr = skip_whitespaces(instr);
  
  if(*instr == '{')
  {
    instr = skip_word(instr);
    instr = skip_whitespaces(instr);    
    while(*instr != '}')
    {
      value = getwordvalue(&instr);
      add_byte(value, data_type);
      instr = skip_whitespaces(instr);              
    }
    instr = skip_word(instr);
    instr = skip_whitespaces(instr);        
  }
  else
  {
    value = getwordvalue(&instr);
    add_byte(value, data_type);
  }

  return 1;
}

static void add_word(unsigned short value, unsigned char type)
{
  unsigned short *wordp = (unsigned short*)&data[datap_segment][datap_offset];
  signed short *swordp = (signed short*)&data[datap_segment][datap_offset];

  check_data_size();
  
   if(type & 0x06 == 0x06)
    *swordp = value;
  else
    *wordp = value;
  datap_offset += 2;
}

static unsigned char define_word(char *instr, unsigned char data_type)
{
  char word_name[SYMBOL_NAME_LENGTH];
  unsigned short value;

  copy_str(word_name, instr);

  if(!define_symbol(word_name, data_type))
    return 0;  
  
  instr = skip_word(instr);
  instr = skip_whitespaces(instr);

  
  if(*instr == '{')
  {
    instr = skip_word(instr);
    instr = skip_whitespaces(instr);    
    while(*instr != '}')
    {
      value = getwordvalue(&instr);
      add_word(value, data_type);
      instr = skip_whitespaces(instr);              
    }
  }
  else
  {
    value = getwordvalue(&instr);
    add_word(value, data_type);
  }

  return 1;
}

static char *parse_string(char *instr, unsigned *stringlen)
{
  unsigned allocated = 1024;
  char *retval = calloc(allocated, sizeof(char));
  unsigned n = 0;
  void *new_pointer;
  
  if(*instr++ != '"')
  {
    sprintf(message, "%s%u", syntaxerror, line_num);
    putstr(message);
    free(retval);
    return 0;    
  }
  if(*instr == '"')
    goto emptystr;
  do
  {
    retval[n++] = parse_char(&instr);
    if(n == allocated)
    {
      allocated += 1024;
      new_pointer = realloc(retval, allocated * sizeof(char));
      if(!new_pointer)
      {
        free(retval);
        return 0;
      }
      retval = new_pointer;
    }
  }
  while(*instr != '"');
  emptystr:retval[n++] = 0;

  *stringlen = n;
  
  return retval;
}



static void add_string_to_data(char *str, unsigned stringlen)
{
  if(debug)
  {
    sprintf(message, "Defined string contents: %s", str);
    putstr(message);
  }

  while(datap_offset+stringlen >= data_allocated)
    if(!allocate_more_data())
    {
      putstr(outofmemory);
      abort();
    }

  memcpy(&data[datap_segment][datap_offset], str, stringlen);
  datap_offset += stringlen;
}

static void add_pointer(char *symbol_name, unsigned short offset)
{
  char define_ptr[SYMBOL_NAME_LENGTH+13];  
  sprintf(define_ptr, "%s 0x%.4X", symbol_name, offset);
  if(!define_word(define_ptr, 0x05))
    putstr(outofmemory);       
}

static char *add_str(char *instr)
{
  char *string_contents;
  unsigned n;
  if(detect_number(instr))
  {
    n = parse_number(instr);
    while(n--)
    {
      data[datap_segment][datap_offset++] = 0;
      check_data_size();
    }
    instr = skip_word(instr);   
  }
  else
  {
    string_contents = parse_string(instr, &n);
    if(string_contents)
    {
      add_string_to_data(string_contents, n); 
//      instr += strlen(string_contents)+2;
      instr += n+2;
    }
    free(string_contents);    
  }
  return instr;
}

static int define_str(char *instr, unsigned char make_pointer)
{
  char str_name[SYMBOL_NAME_LENGTH];
  unsigned n, p;
  unsigned pointer_to_str = datap_offset;
  void *new_pointer;
  unsigned short *pointer_array;

  if(make_pointer)
  {
    copy_str(str_name, instr);

    instr = skip_word(instr);
    instr = skip_whitespaces(instr);
  }


  if(*instr == '{')
  {
    p = 0;
    pointer_array = calloc(2, sizeof(unsigned short));
    instr++;
    instr = skip_whitespaces(instr);    
    while(*instr != '}')
    {
      new_pointer = realloc(pointer_array, ++p * sizeof(unsigned short));
      if(!new_pointer)
      {
        free(pointer_array);
        return 0;
      }
      pointer_array = new_pointer;
      pointer_array[p-1] = datap_offset;

      instr = add_str(instr);
      instr = skip_whitespaces(instr);          
    }
    if(make_pointer)    
      for(n=0;n<p;n++)
      {
        if(!n)
        {
          add_pointer(str_name, pointer_array[0]);
        }
        else
        {
          *(unsigned short*)&data[datap_segment][datap_offset] = pointer_array[n];
          datap_offset+=2;
        }
      }
    free(pointer_array);
    return 1;
  }
  else
  {
    if(!define_symbol(str_name, 0))
      return 0;        
    add_str(instr);
  }

  if(debug)
  {      
    sprintf(message, "Array begins in offset %u", pointer_to_str);
    putstr(message);
  }

  return 1;
}
