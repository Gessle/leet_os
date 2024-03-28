static char *skip_whitespaces(char *str)
{
  while(*str == ' ')
    str++;
  return str;
}

static char *skip_word(char *str)
{
  while(*str && *str != ' ' && *str != '\n')
    str++;
  return str;
}

static unsigned comp_str(char *null_term, char *space_nl_term)
{
  unsigned n = 0;
  while(null_term[n] && space_nl_term[n] && null_term[n] == space_nl_term[n])
    n++;
  if(!null_term[n] && (space_nl_term[n] == ' ' || space_nl_term[n] == '\n' || !space_nl_term[n]))
    return 0;
  return 1;
}

static void copy_str(char *null_term, char *space_nl_term)
{
  unsigned n = 0;
  while(space_nl_term[n] && space_nl_term[n] != ' ' && space_nl_term[n] != '\n')
  {
    null_term[n] = space_nl_term[n];
    n++;
  }
  null_term[n] = 0;
}

static long parse_number(char *str)
{
  if(*str == '0')
  {
    switch(*(str+1))
    {
      case 'x':
        return strtol(str+2, 0, 16);
      case 'b':
        return strtol(str+2, 0, 2);
      case ' ':
        return 0;
    }
    return strtol(str+1, 0, 8);
  }
  return strtol(str, 0, 10);
}

static unsigned detect_number(char *str)
{
  if(*str >= 48 && *str <=57 || *str == '-')
    return 1;
  return 0;
}

static unsigned short parse_char(char **chr)
{
  unsigned short retval;
  
  if(**chr == '\\')
  {
    ++*chr;
    if(detect_number(*chr))
      retval = strtoul(*chr, chr, 010);
    else if(**chr == 'x')
      retval = strtoul((*chr)+1, chr, 0x10);
    else
    {
      switch(**chr)
      {
        case 'a': retval = 0x07; break;
        case 'b': retval = 0x08; break;
        case 'f': retval = 0x0C; break;
        case 'n': retval = 0x0A; break;
        case 'r': retval = 0x0D; break;
        case 't': retval = 0x09; break;
        case 'v': retval = 0x0B; break;
        case '\\': case '\'': case '"':
          retval = **chr;
          break;
        case '?': retval = 0x3F; break;
      }
      ++*chr;         
    }
  }
  else
    retval = *(*chr)++;

  return retval;
}

static void parsejump(char *instr, struct return_string *retval, unsigned make_call)
{
  unsigned char *retvalp = retval->str+1;

  if(make_call || (get_symbol_struct(instr, 1) && get_symbol_seg(instr) != codep_segment))
  {
    *(unsigned short*)retvalp |= 0x80;
    *(unsigned short*)retvalp |= get_symbol_seg(instr);
    retvalp+=2;
  }
  else
    *retvalp++ = 0x00;

  *(unsigned short*)retvalp = get_symbol_addr(instr);
  retvalp+=2;

  *retvalp = 0;
  retval->length = retvalp - retval->str;      
}

static struct return_string *build_instr(char *str)
{
  struct return_string *retval;
  unsigned char n = 0;
  unsigned char b = 0;
  unsigned char w = 0;
  unsigned short *shortp;
  unsigned char wreg = 0;
  unsigned char check_imm = 0;
  unsigned temp;

  do
  {
    if(!comp_str(INSTR[n], str))
      break;
  }
  while(INSTR[++n]);

  if(!INSTR[n])
  {
    sprintf(message, "Unknown instruction in line %u: %s\n", line_num, str);
    putstr(message);
    return 0;
  }

  else
  {
    if(debug)
    {
      sprintf(message, "Instruction %s on line %u\n", INSTR[n], line_num);
      putstr(message);
    }
  }

  retval = calloc(1, sizeof(struct return_string));
  if(!retval) return 0;

  retval->str[0] = n;
  retval->length = 1;

  str = skip_word(str);
  str = skip_whitespaces(str);

  if(instr_in_array(n, VINSTR));

  else if(instr_in_array(n, SBYTE_INSTR))
    b=1;
  else if(instr_in_array(n, DBYTE_INSTR))
  {
    b=2;
    if(instr_in_array(n, OPERIMM_INSTR))
      check_imm = 1;
  }
  else if(instr_in_array(n, SWORD_INSTR))
    w=1;
  else if(instr_in_array(n, DWORD_INSTR))
    w=2;
  else if(instr_in_array(n, TWORD_INSTR))
    w=3;
  else if(n == OPCODE_MOV)
  {
    parsemov(str, retval);
    goto end;
  }
  else if(n == OPCODE_JUMP)
  {
    parsejump(str, retval, 0);
    goto end;
  }
  else if(n == OPCODE_CALL)
  {
    parsejump(str, retval, 1);
    goto end;
  }

  if(b)
    if(instr_in_array(n, WREG_INSTR))
      wreg = 1;

  n = 1;

  while(b--)
  {
    if(detect_number(str) || *str == '\'')
    {
//      temp = parse_number(str);
      temp = getwordvalue(&str);
      if(check_imm)
      {
        retval->str[0] |= 0x80;
        if(temp & 0xFF00)
        {
          retval->str[0] |= 0x40;
          *(unsigned short*)&retval->str[n] = temp;
          n += 2;
          goto imm_ready;
        }
      }
      retval->str[n++] = temp;
      imm_ready:
      str = skip_word(str);
      str = skip_whitespaces(str);
    }
    else
    {
      // muuta rekisteri tavukoodiksi
      retval->str[n] = parse_wide_register(str);
      if(wreg) retval->str[n] &= 0x7F;
      n++;
      str = skip_word(str);
      str = skip_whitespaces(str);
    }
  }

  while(w--)
  {
    shortp = (unsigned short*)&retval->str[n];
    n += 2;
    if(detect_number(str) || *str == '\'')
    {
//      *shortp = parse_number(str);
      *shortp = getwordvalue(&str);
      str = skip_word(str);
      str = skip_whitespaces(str);
    }
    else
    {
      // muuta symboli tavukoodiksi
        if(*str == '.')
        {
          str++;
          *shortp = get_symbol_seg(str);
        }
        else
        {
          if(*str == '$') str++;
          *shortp = get_symbol_addr(str);
        }
        str = skip_word(str);
        str = skip_whitespaces(str);
    }
  }

  retval->str[n] = 0;
  retval->length = n;

  end:
  return retval;

}

static unsigned char detect_data_type(char **instr)
{
  unsigned n = (sizeof(data_types) / sizeof(char *));
  char *instrp = *instr;
  unsigned char type = 0x01;

  if(!comp_str("signed", instrp))
  {
    instrp = skip_word(instrp);
    instrp = skip_whitespaces(instrp);
    type |= 0x08;
  }

  while(n--)
  {
    if(!comp_str(data_types[n], instrp))
    {
      instrp = skip_word(instrp);
      instrp = skip_whitespaces(instrp);
      *instr = instrp;
      break;
    }
    else if(!n)
      return 0xFF;
  }
  if(n == 2)
    return 0;
  if(n == 1)
    return type | 0x04;
  if(!n)
    return type;

  return 0;
}

static int start_instr(char *instr, struct return_string **instruction)
{
  unsigned char data_type;

  instr = skip_whitespaces(instr);

  switch((data_type = detect_data_type(&instr)))
  {
    case 0xFF:
      break;
    default:
      if(!pass)
        define_data(instr, data_type);
      return 1;
  }

  switch(*instr)
  {
    case ':':
      if(pass) return 1;
      if(define_label(&instr[1]))
        return 1;
      break;
    case ';': case '\n':
      return 1;
    default:
      *instruction = build_instr(instr);      
      if(!*instruction) 
        return -1;
  }
  return 0;
}

static void define_data(char *instr, unsigned char data_type)
{
  if(!data_type)
  {
    if(!detect_number(instr))
    {
      if(!define_str(instr, 1))
        putstr(outofmemory);
    }
    else if(*instr == '-')
    {
      instr = skip_word(instr);
      instr = skip_whitespaces(instr);
      if(!define_str(instr, 0))
        putstr(outofmemory);
    }
  }

  else
  {
    if(data_type & 0x04)
    {
      if(!define_word(instr, data_type))
        putstr(outofmemory);
    }

    else
      if(!define_byte(instr, data_type))
        putstr(outofmemory);
  }
}
