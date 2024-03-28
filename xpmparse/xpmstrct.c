struct xpm_color
{
  char chars[6];
  unsigned color;
};

struct xpm_data
{
  unsigned width;
  unsigned height;
  unsigned colors;
  unsigned chars_per_pixel;
  struct xpm_color *color_table;
};

struct xpm_bitmap
{
  unsigned width;
  unsigned height;
  unsigned char __far *bitmap;
};
