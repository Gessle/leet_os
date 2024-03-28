#define VPU_REGISTER_COUNT 8
#define LINE_LEN 128

#define SYMBOL_NAME_LENGTH 32

#include "../../structs.c"

struct symbol
{
  char name[SYMBOL_NAME_LENGTH];
  unsigned short address;
  unsigned short segment;
  unsigned char type;
};
/*struct label
{
  char name[32];
  unsigned short address;
  unsigned short segment;
};*/
unsigned symbol_count = 0;
//unsigned label_count = 0;

struct symbol *symbols;
//struct label *labels;

unsigned char data_auto_len = 0;
unsigned char code_auto_len = 0;

unsigned short datap_segment=0;
unsigned short datap_offset=0;

unsigned short codep_offset=0;
unsigned short codep_segment=0;

unsigned line_num = 0;
unsigned char pass = 0;

static char *skip_word(char *str);
static char *skip_whitespaces(char *str);
static void copy_str(char *null_term, char *space_nl_term);
static unsigned detect_number(char *str);
static long parse_number(char *str);
static int define_symbol(char *name, unsigned char type);
static unsigned short get_symbol_addr(char *name);
static unsigned short get_symbol_seg(char *name);
static unsigned char define_symbol_type(unsigned char operand, char *symstr);
static unsigned char define_register_type(unsigned char operand, char *regstr);
static int define_label(char *name);
static void define_data(char *instr, unsigned char data_type);
static unsigned char parse_wide_register(char *str);
static struct symbol *get_symbol_struct(char *name, unsigned char verbose);
static unsigned short parse_char(char **chr);

unsigned char **data;
unsigned short data_allocated;
unsigned short data_count = 1;

char *data_types[] =
{
  "byte",
  "word",
  "str"
};

char syntaxerror[] = "Syntax error in line ";


