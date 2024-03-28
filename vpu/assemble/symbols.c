static struct symbol *get_symbol_struct(char *name, unsigned char verbose)
{
  unsigned c = symbol_count;
  while (c--)
  {
    if(verbose && debug)
    {
      sprintf(message, "Found symbol: %s address 0x%.2X", symbols[c].name, symbols[c].address);
      putstr(message);
    }
    if(!comp_str(symbols[c].name, name))
      return &symbols[c];
  }
  if(pass)
  {
    sprintf(message, "Undefined symbol %s in line %u", name, line_num);
    putstr(message);
  }
  return 0;
}

static unsigned short get_symbol_addr(char *name)
{
  struct symbol *retval = get_symbol_struct(name, 0);
  if(retval) return retval->address;
  return 0;
}

static char *get_addr_symbol(unsigned short seg, unsigned short addr, unsigned char type)
{
  unsigned c = symbol_count;
  unsigned n = 0;
  static char retval[SYMBOL_NAME_LENGTH];
  while(c--)
    if(symbols[c].segment == seg && symbols[c].address == addr && (symbols[c].type & type || type == 0x01 && !symbols[c].type))
    {
      do
      {
        retval[n] = symbols[c].name[n];
      }
      while(symbols[c].name[n++]);
      return retval;
    }
  return 0;
}

static unsigned short get_symbol_seg(char *name)
{
  struct symbol *retval = get_symbol_struct(name, 0);
  if(retval) return retval->segment;
  return 0;
}

static unsigned char define_symbol_type(unsigned char operand, char *symstr)
{
  struct symbol *retval = get_symbol_struct(symstr, 0);
  if(retval)
    return retval->type << (operand*4);

  return 0;
}

char *regs[] =
{
  "a", "b", "c", "d", "e", "f", "g", "h"
};

/*char *siregs[] =
{
  "cs", "ip", "ds", "sp"
};*/

static unsigned char define_register_type(unsigned char operand, char *regstr)
{
  unsigned n = sizeof(siregs)/sizeof(const char*);

  while(n--)
    if(!strncmp(regstr, siregs[n], 2))
      goto return_full_reg;

  if(regstr[1] == 'x')
  {
    return_full_reg:
    if(!operand)
      return 0x04;
    return 0x40;
  }
  return 0;
}

static unsigned char parse_wide_register(char *str)
{
  unsigned n = sizeof(siregs)/sizeof(const char*);
  while(n--)
    if(!strncmp(str, siregs[n], 2))
//      return 0x8|n;
      return 0x88|n;
  
  
  if(str[1] == 'x')
    return (*str - 'a') | 0x80;

  if(str[1] == 'l')
   return (*str - 'a') * 2;

  return (*str - 'a') * 2 + 1;
}

/*unsigned char parse_byte_register(char *str)
{
  unsigned retval = (*str - 'a') * 2;
  if(str[1] = 'h') retval++;
  return retval;
}

unsigned char parse_register(char *str)
{
  if(define_register_type(0, str))
    return parse_wide_register(str) & 0x08;
  else
    return parse_byte_register(str);
}*/

static int define_symbol(char *name, unsigned char type)
{
  void *new_pointer;
  new_pointer = realloc(symbols, (symbol_count + 1) * sizeof(struct symbol));
  if(!new_pointer) return 0;
  symbols = new_pointer;

  symbols[symbol_count].segment = datap_segment;
  symbols[symbol_count].address = datap_offset;

  symbols[symbol_count].type = type;
  strcpy(symbols[symbol_count].name, name);

  if(debug)
  {
    sprintf(message, "Defined symbol %s, type 0x%.2X, on line %u", name, type, line_num);
    putstr(message);
  }

  symbol_count++;

  return 1;
}

static int define_label(char *name)
{
  void *new_pointer;
  if(get_symbol_struct(name, 0))
  {
    if(debug)
    {
      sprintf(message, "Label %s on line %u already defined, skipping...", name, line_num);
      putstr(message);
    }
    return 1;
  }

  new_pointer = realloc(symbols, (symbol_count + 1) * sizeof(struct symbol));
  if(!new_pointer) return 0;
  symbols = new_pointer;

  symbols[symbol_count].address = codep_offset;
  symbols[symbol_count].segment = codep_segment;
  symbols[symbol_count].type = 0x02;

//  strcpy(symbols[symbol_count].name, name);
  copy_str(symbols[symbol_count].name, name);

//  while(name[n] != '\n')
//    symbols[symbol_count].name[n] = name[n++];
//  symbols[symbol_count].name[n] = 0;

  if(debug)
  {
    sprintf(message, "Defined label %s - line %u", name, line_num);
    putstr(message);
  }

  symbol_count++;

  return 1;
}
