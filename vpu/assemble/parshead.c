char *header_keywords[] =
{
  "stack",
  "code",
  "data",
  "interrupts"
};

static unsigned char header_keyword(char *str)
{
  unsigned n = 4;

  while(n--)
  {
    if(!comp_str(header_keywords[n], str))
      break;
  }

  return n;
}

/*
  Program header format:

  First byte: Bits 7-4: Stack, code, data, interrupts
              Bits 3-2-1-0: word, byte, word, byte

  If (first byte & 0x08), next comes two bytes (unsigned short int) indicating segment count
  Else only one byte

  If (first byte & 0x04), next comes two bytes indicating segment length
  Else only one byte

  
*/

static struct return_string parse_header(FILE *fp)
{
  unsigned n = 4;
  unsigned m;
  unsigned long number;
  unsigned short *wp;
  struct return_string retval;
  unsigned char *bcode = retval.str;
  unsigned char *header_byte;
  char strline[LINE_LEN];
  char *str;

  while(n--)
  {
    fgets(strline, LINE_LEN, fp);
    str = strline;

    sprintf(message, "Header line %u: %s\n", 4-n, strline);
    putstr(message);
    header_byte = bcode;
    *header_byte = 0x80 >> header_keyword(str);

    str = skip_word(str);
    str = skip_whitespaces(str);

    if(!(*header_byte & 0x80) && !(*header_byte & 0x10)) m=2;
    else m=1;

    if(m==2 && !strncmp(str, "auto", 4))
    {
      if(*header_byte & 0x20)
        data_auto_len = 1;
      else
        code_auto_len = 1;
      continue;
    }

    bcode++;

    while(m--)
    {
      number = parse_number(str);
      str = skip_word(str);
      str = skip_whitespaces(str);

      if(number > 0xFF)
      {
        *header_byte |= 0x02 << (m*2);
        wp = (unsigned short*)bcode;
        *wp = number;
        bcode+=2;
      }
      else
      {
        *header_byte |= 0x01 << (m*2);
        *bcode++ = number;
      }
    }
  }
  retval.length = bcode - retval.str;

  sprintf(message, "Generated %u bytes long header: \n", retval.length);
  putstr(message);
  message[0] = 0;
  for(n=0;n<retval.length;n++)
  {
    sprintf(hexstr, "0x%.2X ", retval.str[n]);
    strcat(message, hexstr);
  }
  putstr(message);

  line_num = 4;

  return retval;

}
