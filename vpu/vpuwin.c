//#include "../globals.c"

unsigned vpuconsole_program(struct prog_window *window, unsigned args, int *keypress);
struct prog_window *init_vpuprog_window(unsigned x, unsigned y, unsigned char *title, unsigned mouse_int, unsigned mouse_port, void *vpu);
struct window_element *window_text(struct prog_window *window, unsigned int x, unsigned int y, unsigned char *text, unsigned char color);
struct window_element *window_button(struct prog_window *window, unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char *text, unsigned char color);
struct window_element *window_textbox(struct prog_window *window, unsigned int x, unsigned int y, unsigned int width, unsigned char *text, unsigned int len);
struct window_element *window_list(struct prog_window *window, unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char **items, unsigned int len);
struct window_element *window_image(struct prog_window *window, unsigned int x, unsigned int y, unsigned int width, unsigned int height, char *filename);
struct window_element *window_scrollbar(struct prog_window *window, unsigned int x, unsigned int y, unsigned type, unsigned int length, unsigned int count, int element, int *value);
struct window_element *window_icon(struct prog_window *window, unsigned int x, unsigned int y, unsigned char *text, char *icon_file);
struct window_element *window_checkbox(struct prog_window *window, unsigned x, unsigned y, char *text, unsigned color);
struct window_element *window_radiobutton(struct prog_window *window, unsigned x, unsigned y, char *text, unsigned color, struct window_element *prev_element);
struct window_element *window_ml_textbox(struct prog_window*, unsigned, unsigned, unsigned, unsigned, char*, unsigned);
unsigned vpuwindow_program(struct prog_window *window, unsigned args, int *keypress);
void put_pixel(unsigned int x, unsigned int y, unsigned char color);
extern struct prog_window *running_window;
void draw_line(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, unsigned char color);
void draw_rectangle(int x0, int y0, unsigned int width, unsigned int height, unsigned char color);
void draw_circle(unsigned x, unsigned y, unsigned r, unsigned char color);
extern unsigned char _FONT_HEIGHT;
extern unsigned char _WINDOW_BGCOLOR;
void gprint_text(unsigned int x, unsigned int y, unsigned char __far *text, unsigned char color, unsigned char font_height, unsigned int x2, unsigned monospace);
void draw_window(struct prog_window *window, unsigned active, unsigned update);
//void draw_mouse_cursor(unsigned updatebg);
int *get_mouse_movement(void);
unsigned char windowpos_visible(struct prog_window *window, int x, int y);
void draw_screen(void);
void video_mode(unsigned char mode);
int dialog(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned type, unsigned help_button, unsigned sound, unsigned char *message, unsigned char *title, unsigned char *helptext);
void window_radiobutton_select(struct prog_window *window, struct window_element *radiobutton);
extern struct prog_window **windows;
extern int active_window;
extern unsigned int _RES_Y;
extern unsigned int _RES_X;
extern volatile unsigned char video;
extern unsigned char __far *VGA;
extern unsigned window_count;
void set_active_window(int window, unsigned draw);
extern int *window_visibility_order;
void window_drawelement(struct prog_window *window, unsigned int element);
void window_recalc_hitboxes(struct prog_window *);

char nowindowerror[] = "No such window!";

static int vpu_scrcall_createwindow(struct vpu *vpu)
{
  char *title = &vpu->data[vpu->data_segment][vpu->regs[3]];
  unsigned short window_descriptor;


  if(!memory_allowed(vpu, (long)vpu->regs[3]+strlen(title)))
    return 4;

  for(window_descriptor=0;vpu->windows[window_descriptor].window && window_descriptor < MAX_WINDOWS;window_descriptor++);

  if(window_descriptor < MAX_WINDOWS-1)
  {
//    draw_mouse_cursor(2);    
    vpu->windows[window_descriptor].window = init_vpuprog_window(vpu->regs[1], vpu->regs[2], title, vpu->regs[6], vpu->regs[7], vpu);
    if(!vpu->windows[window_descriptor].window)
      goto error;
    draw_window(vpu->windows[window_descriptor].window, 1, 0);
//    draw_mouse_cursor(1);
    vpu->regs[0] = window_descriptor;
    vpu->windows[window_descriptor].keypress_int = vpu->regs[4];
    vpu->windows[window_descriptor].keypress_port = vpu->regs[5];
  }
  else
    error:vpu->regs[0] = 0xFFFF;

  return 0;
}

static int vpu_scrcall_fullscreen(struct vpu *vpu)
{
  const unsigned char allow_modes[] = { 0x06, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x00 };
//  unsigned long buffer_size = 0;
  vpu_scrcall_createwindow(vpu);
  vpu->windows[vpu->regs[0]].window->fullscreen=1;
  if(vpu->regs[1] && strchr(allow_modes, vpu->regs[1]))  
    vpu->windows[vpu->regs[0]].window->scrmode = vpu->regs[1];
  else
    vpu->windows[vpu->regs[0]].window->scrmode = video;
  video_mode(vpu->windows[vpu->regs[0]].window->scrmode);
  vpu->windows[vpu->regs[0]].window->width = _RES_X;
  vpu->windows[vpu->regs[0]].window->height = _RES_Y;
  vpu->windows[vpu->regs[0]].window->x = 0;
  vpu->windows[vpu->regs[0]].window->y = 0;
  switch(vpu->regs[1])
  {
    case 0x13:
      vpu->windows[vpu->regs[0]].buffer_size = 64000;
      break;
    case 0x06:
      vpu->windows[vpu->regs[0]].buffer_size = 16384;
      break;
    case 0x0F:
      vpu->windows[vpu->regs[0]].buffer_size = 28000;
      break;
    case 0x11:
      vpu->windows[vpu->regs[0]].buffer_size = 38400;
  }
  if(vpu->windows[vpu->regs[0]].buffer_size)
    vpu->windows[vpu->regs[0]].renderer = _fcalloc(vpu->windows[vpu->regs[0]].buffer_size, sizeof(unsigned char));  
  draw_screen();

  return 0;
}

static int vpu_scrcall_putpixel(struct vpu *vpu)
{
  unsigned x, y; 

  x = vpu->windows[vpu->regs[4]].window->x + vpu->regs[1];
  y = vpu->windows[vpu->regs[4]].window->y + vpu->regs[2] + _FONT_HEIGHT+2;

//  if(windowpos_visible(vpu->windows[vpu->regs[4]].window, x, y))
  put_pixel(x, y, vpu->regs[3]);

  return 0;
}

static int vpu_scrcall_memset(struct vpu *vpu)
{
  unsigned color = vpu->regs[3];
  unsigned count = vpu->regs[5];
  unsigned maxx = vpu->windows[vpu->regs[4]].window->x + vpu->windows[vpu->regs[4]].window->width;
  unsigned maxy = vpu->windows[vpu->regs[4]].window->y + vpu->windows[vpu->regs[4]].window->height+_FONT_HEIGHT+2;
  unsigned x = vpu->windows[vpu->regs[4]].window->x+vpu->regs[1];
  unsigned y = vpu->windows[vpu->regs[4]].window->y+vpu->regs[2]+_FONT_HEIGHT+2;

  while(count--)
  {
    if(x >= maxx)
    {
      x = vpu->windows[vpu->regs[4]].window->x;
      y++;
    }
    if(y >= maxy)
      return 0;
    put_pixel(x++, y, color);
  }
  return 0;
}

static int vpu_scrcall_memcpy(struct vpu *vpu)
{
  unsigned count = vpu->regs[5];
  unsigned dataoffset = vpu->regs[3];
  unsigned char *dataptr = &vpu->data[vpu->data_segment][dataoffset];
  unsigned maxx = vpu->windows[vpu->regs[4]].window->x + vpu->windows[vpu->regs[4]].window->width;
  unsigned maxy = vpu->windows[vpu->regs[4]].window->y + vpu->windows[vpu->regs[4]].window->height+_FONT_HEIGHT+2;
  unsigned x = vpu->windows[vpu->regs[4]].window->x+vpu->regs[1];
  unsigned y = vpu->windows[vpu->regs[4]].window->y+vpu->regs[2]+_FONT_HEIGHT+2;

  if(!memory_allowed(vpu, (long)dataoffset+count))
    return 4;

  while(count--)
  {
    if(x >= maxx)
    {
      x = vpu->windows[vpu->regs[4]].window->x;
      y++;
    }
    if(y >= maxy)
      return 0;
    put_pixel(x++, y, *dataptr++);
  }
  return 0;
}

static int vpu_scrcall_update(struct vpu *vpu)
{
  if(!vpu->windows[vpu->regs[4]].window)
    putstr(nowindowerror);
  else if(!vpu->windows[vpu->regs[4]].window->minimized && vpu->windows[vpu->regs[4]].renderer)
  {
    memcpy(VGA, vpu->windows[vpu->regs[4]].renderer, vpu->windows[vpu->regs[4]].buffer_size);
  }
  return 0;
}

static int vpu_scrcall_clear(struct vpu *vpu)
{
  if(!vpu->windows[vpu->regs[4]].window->minimized
    && windows[active_window] == vpu->windows[vpu->regs[4]].window)
  {
    draw_window(vpu->windows[vpu->regs[4]].window, 0, 1);
//    return 1;
  }
  return 0;
}

static int vpu_scrcall_text(struct vpu *vpu)
{
  char *text = &vpu->data[vpu->data_segment][vpu->regs[3]];
  struct window_element *element;

  if(!memory_allowed(vpu, (long)vpu->regs[3]+strlen(text)))
    return 4;

  element = window_text(vpu->windows[vpu->regs[4]].window, vpu->regs[1], vpu->regs[2], text, vpu->regs[5]);
  if(!element) vpu->regs[0] = 0xFFFF;
//  draw_window(vpu->windows[vpu->regs[4]].window, 0, 1);
  return 0;
}

static int vpu_scrcall_button(struct vpu *vpu)
{
  char *text = &vpu->data[vpu->data_segment][vpu->regs[3]];
  struct window_element *element;

  if(!memory_allowed(vpu, (long)vpu->regs[3]+strlen(text)))
    return 4;

  element = window_button(vpu->windows[vpu->regs[4]].window, vpu->regs[1], vpu->regs[2], vpu->regs[6], vpu->regs[7],
    text, vpu->regs[5]);
  
  if(!element) vpu->regs[0] = 0xFFFF;
  else vpu->regs[0] = 0x00FF + vpu->windows[vpu->regs[4]].window->hitbox_count;
  
//  draw_window(vpu->windows[vpu->regs[4]].window, 0, 1);
  return 0;
}

static int vpu_scrcall_checkbox(struct vpu *vpu)
{
  char *text = &vpu->data[vpu->data_segment][vpu->regs[3]];
  struct window_element *element;

  if(!memory_allowed(vpu, (long)vpu->regs[3]+strlen(text)))
    return 4;

  element = window_checkbox(vpu->windows[vpu->regs[4]].window, vpu->regs[1], vpu->regs[2], text, vpu->regs[5]);

  if(!element) vpu->regs[0] = 0xFFFF;
  else
  {
    vpu->regs[0] = 0x00FF + vpu->windows[vpu->regs[4]].window->hitbox_count;
    vpu->regs[1] = vpu->windows[vpu->regs[4]].window->element_count-1;    
  }

  return 0;  
}

static int vpu_scrcall_textbox_update(struct vpu *vpu)
{
  char *text = &vpu->data[vpu->data_segment][vpu->regs[3]];
  struct window_element *element;  
  if(!memory_allowed(vpu, (long)vpu->regs[3]+strlen(text)) || vpu->regs[1] >= vpu->windows[vpu->regs[4]].window->element_count)
    return 4;

  element = vpu->windows[vpu->regs[4]].window->window_grid[vpu->regs[1]];
  free(element->text);
  element->text = calloc(element->c+1, sizeof(char)); // this calloc should not be able to fail...
  strncpy(element->text, text, element->c);

  window_drawelement(vpu->windows[vpu->regs[4]].window, vpu->regs[1]);

  return 0;
}

static int vpu_scrcall_textbox(struct vpu *vpu)
{
  char *text = &vpu->data[vpu->data_segment][vpu->regs[3]];
  struct window_element *element;

  if(!memory_allowed(vpu, (long)vpu->regs[3]+vpu->regs[5]))
    return 4;

  element = window_textbox(vpu->windows[vpu->regs[4]].window, vpu->regs[1], vpu->regs[2], vpu->regs[6], text,
    vpu->regs[5]);
  
  if(!element) vpu->regs[0] = 0xFFFF;
  else
  {
    vpu->regs[0] = 0x00FF + vpu->windows[vpu->regs[4]].window->hitbox_count;
    vpu->regs[1] = vpu->windows[vpu->regs[4]].window->element_count-1;
  }
  

  element->connect_value = text;
  
//  draw_window(vpu->windows[vpu->regs[4]].window, 0, 1);
  return 0;
}

static unsigned char **make_list(struct vpu *vpu, unsigned list_len, unsigned short *items)
{
  unsigned char **retval = calloc(list_len, sizeof(char *));
  if(!retval)
  {
    nomemory:
    vpu->regs[0] = 0xFFFF;
    return 0;
  }

  while(list_len--)
    if(memory_allowed(vpu, items[list_len]))
    {
      if(!(retval[list_len] = calloc(1+strlen(&vpu->data[vpu->data_segment][items[list_len]]), sizeof(unsigned char))))
      {
        free_list:
        while(++list_len < vpu->regs[7])
          free(retval[list_len]);
        free(retval);
        goto nomemory;
      }
      strcpy(retval[list_len], &vpu->data[vpu->data_segment][items[list_len]]);
    }
    else
      goto free_list;
  return retval;
}

static int vpu_scrcall_textbuff(struct vpu *vpu)
{
  struct window_element *element;
  unsigned char *text_buff = &vpu->data[vpu->data_segment][vpu->regs[3]];
  unsigned count = vpu->regs[5];

  if(!memory_allowed(vpu, (long)vpu->regs[3] + count))
    return 4;
  element = window_ml_textbox(vpu->windows[vpu->regs[4]].window, vpu->regs[1],
              vpu->regs[2], vpu->regs[6], vpu->regs[7], text_buff, count);
  if(!element)
  {
    vpu->regs[0] = 0xFFFF;
    return 1;
  }
  vpu->regs[0] = 0x00FF + vpu->windows[vpu->regs[4]].window->hitbox_count;
  vpu->regs[1] = vpu->windows[vpu->regs[4]].window->element_count -1;
  return 0;
}

static int vpu_scrcall_list_textbox(struct vpu *vpu, unsigned char type)
{
  struct window_element *element;
  unsigned short *items = (unsigned short*)&vpu->data[vpu->data_segment][vpu->regs[3]];
  unsigned short list_len = vpu->regs[7];
  unsigned char **list_rows;
  

  if(!memory_allowed(vpu, (long)vpu->regs[3]+(list_len<<1)))
    return 4;

  element = window_list(vpu->windows[vpu->regs[4]].window, vpu->regs[1], vpu->regs[2], vpu->regs[5], vpu->regs[6], 0, vpu->regs[7]);
  if(!element)
  {
    nomemory:
    vpu->regs[0] = 0xFFFF;
    return 4;
  }
  
  list_rows = make_list(vpu, list_len, items);
  if(list_rows)
    element->items = list_rows;
  else
    goto nomemory;

//  free(list_rows);

  if(type)
    element->active = 0;

  vpu->regs[0] = 0x00FF + vpu->windows[vpu->regs[4]].window->hitbox_count;
  vpu->regs[1] = vpu->windows[vpu->regs[4]].window->element_count-1;

  return 0;
}

static int vpu_scrcall_list(struct vpu *vpu)
{
  return vpu_scrcall_list_textbox(vpu, 0);
}

static int vpu_scrcall_mltext(struct vpu *vpu)
{
  return vpu_scrcall_list_textbox(vpu, 1);
}

static int vpu_scrcall_updatemltext(struct vpu *vpu)
{
  struct window_element *element;
  unsigned short *items = (unsigned short*)&vpu->data[vpu->data_segment][vpu->regs[3]];
  unsigned short list_len = vpu->regs[7];
  unsigned char **list_rows;

  if(!memory_allowed(vpu, (long)vpu->regs[3]+(list_len<<1)) || vpu->regs[1] >= vpu->windows[vpu->regs[4]].window->element_count)
    return 4;  
    
  element = vpu->windows[vpu->regs[4]].window->window_grid[vpu->regs[1]];

  while(element->c--)
    free(element->items[element->c]);
  free(element->items);

  list_rows = make_list(vpu, list_len, items);
  if(list_rows)
  {
    element->items = list_rows;
    element->c = vpu->regs[7];
  }
  else
  {
    vpu->regs[0] = 0xFFFF;
    return 4;
  }

  window_drawelement(vpu->windows[vpu->regs[4]].window, vpu->regs[1]);
  return 0;
}

static int vpu_scrcall_scrollbar(struct vpu *vpu)
{
  struct prog_window *window = vpu->windows[vpu->regs[4]].window;
  unsigned x = vpu->regs[1];
  unsigned y = vpu->regs[2];
  // types: 0 = vertical scrollbar, bind value to a list's vertical scroll
  // 1: horizontal scrollbar, bind value to a list's horizontal scroll
  // 2: vertical scrollbar, read or write value from a port in element reg
  // 3: horizontal scrollbar, read or write the value from a port
  unsigned type = vpu->regs[3];
  unsigned length = vpu->regs[5];
  unsigned count = vpu->regs[6];
  unsigned read_value = vpu->regs[7];
  struct window_element *element;
  int *connect_value;
  struct window_element *return_element;
  

  if(type < 2)
  {
    if(read_value >= window->hitbox_count)
    {
      error:vpu->regs[0] = -1;
      return 0;
    }
    element = window->window_grid[read_value];
    if(!type)
      connect_value = (int*)&window->window_grid[read_value]->scroll_y;
    else
      connect_value = (int*)&window->window_grid[read_value]->scroll_x;
  }
  else
  {
    element = 0;
    if(read_value >= VPU_PORT_COUNT) goto error;
    connect_value = (int*)&vpu->ports[read_value];
  }

  return_element = window_scrollbar(window, x, y, type, length, count, read_value, connect_value);
  if(!return_element) goto error;
  vpu->regs[0] = 0x00FF + window->hitbox_count;
  draw_window(window, 1, 3);

  return 0;
}

static int vpu_scrcall_icon(struct vpu *vpu)
{
  char *text = &vpu->data[vpu->data_segment][vpu->regs[3]];
  char *icon = &vpu->data[vpu->data_segment][vpu->regs[5]];
  struct window_element *element;

  if(!memory_allowed(vpu, (long)vpu->regs[3]+strlen(text)) || !memory_allowed(vpu, (long)vpu->regs[5]+strlen(icon)))
    return 4;

  element = window_icon(vpu->windows[vpu->regs[4]].window, vpu->regs[1], vpu->regs[2], text, icon);
  
  if(!element) vpu->regs[0] = 0xFFFF;
  else vpu->regs[0] = 0x00FF + vpu->windows[vpu->regs[4]].window->hitbox_count;
  
//  draw_window(vpu->windows[vpu->regs[4]].window, 0, 1);
  return 0;
}

static int vpu_scrcall_circle(struct vpu *vpu)
{
  unsigned x, y, r; 

  x = vpu->windows[vpu->regs[4]].window->x + vpu->regs[1];
  y = vpu->windows[vpu->regs[4]].window->y + vpu->regs[2] + _FONT_HEIGHT+2;
  r = vpu->regs[5];

  draw_circle(x, y, r, vpu->regs[3]);

  return 0;
}

static int vpu_scrcall_line(struct vpu *vpu)
{
  unsigned x0, y0, x1, y1;
  x0 = vpu->windows[vpu->regs[4]].window->x + vpu->regs[1];
  y0 = vpu->windows[vpu->regs[4]].window->y + vpu->regs[2] + _FONT_HEIGHT+2;
  x1 = vpu->windows[vpu->regs[4]].window->x + vpu->regs[5];
  y1 = vpu->windows[vpu->regs[4]].window->y + vpu->regs[6] + _FONT_HEIGHT+2;

  draw_line(x0, y0, x1, y1, vpu->regs[3]);

  return 0;
}

static int vpu_scrcall_rectangle(struct vpu *vpu)
{
  unsigned x, y, w, h;

  x = vpu->windows[vpu->regs[4]].window->x + vpu->regs[1];
  y = vpu->windows[vpu->regs[4]].window->y + vpu->regs[2] + _FONT_HEIGHT+2;
  w = vpu->regs[5];
  h = vpu->regs[6];

  draw_rectangle(x, y, w, h, vpu->regs[3]);

  return 0;
}

static int vpu_scrcall_temptext(struct vpu *vpu)
{
  char *text = &vpu->data[vpu->data_segment][vpu->regs[3]];
  unsigned x = vpu->windows[vpu->regs[4]].window->x + vpu->regs[1];
  unsigned y = vpu->windows[vpu->regs[4]].window->y + vpu->regs[2] + _FONT_HEIGHT+2;
  unsigned x2 = vpu->windows[vpu->regs[4]].window->x+vpu->windows[vpu->regs[4]].window->width;

  if(!memory_allowed(vpu, (long)vpu->regs[3]+strlen(text)))
    return 4;

  gprint_text(x, y, text, vpu->regs[5], _FONT_HEIGHT, x2, vpu->regs[6]);

  return 0;
}

/*
  BX == title
  CX == message
  DX == help
  EL == type
  EH == sound
*/
static int vpu_scrcall_dialog(struct vpu *vpu)
{
  char *title = &vpu->data[vpu->data_segment][vpu->regs[1]];
  char *message = &vpu->data[vpu->data_segment][vpu->regs[2]];
  char *helptext = &vpu->data[vpu->data_segment][vpu->regs[3]];

  if(!memory_allowed(vpu, (long)vpu->regs[2]+strlen(message)) && !memory_allowed(vpu, (long)vpu->regs[1]+strlen(title)))
    return 4;
  if(vpu->regs[3])
  {
    if(!memory_allowed(vpu, (long)vpu->regs[3]+strlen(helptext)))    
      return 4;
  }
  else helptext = 0;

  
  vpu->regs[0] = dialog(-1, -1, 170+(strlen(message)<<3), 50, vpu->regs[4]&0x00FF, vpu->regs[3], (vpu->regs[4]&0xFF00)>>8, message, title, helptext);

  return 0;
}

static int vpu_scrcall_xpm2img(struct vpu *vpu)
{
  char *filename = &vpu->data[vpu->data_segment][vpu->regs[3]];
  char file_path[MAX_PATH*2+2];
  struct window_element *element;

  if(!memory_allowed(vpu, (long)vpu->regs[3]+strlen(filename)))
    return 4;

  append_path(vpu, file_path, filename);

  element = window_image(vpu->windows[vpu->regs[4]].window, vpu->regs[1], vpu->regs[2], vpu->regs[5], vpu->regs[6], file_path);
  if(!element) vpu->regs[0] = 0xFFFF;
//  draw_window(vpu->windows[vpu->regs[4]].window, 0, 1);
  return 0;
}

void window_grid_delelement(struct prog_window *window, unsigned n);

// todo: test this
static int vpu_scrcall_delelement(struct vpu *vpu)
{
  if(vpu->regs[1] < vpu->windows[vpu->regs[4]].window->element_count)
  {
    window_grid_delelement(vpu->windows[vpu->regs[4]].window, vpu->regs[1]);
//    draw_window(vpu->windows[vpu->regs[4]].window, 0, 1);          
  }

  return 0;
}

static int vpu_scrcall_move_element(struct vpu *vpu)
{
  struct window_element *element;
  struct prog_window *window = vpu->windows[vpu->regs[4]].window;
  unsigned n;
  struct hitbox *hitbox;

  if(vpu->regs[1] < window->element_count)
    element = window->window_grid[vpu->regs[1]];
  else return 0;
  
  element->x = vpu->regs[2];
  element->y = vpu->regs[3];

  window_recalc_hitboxes(window);
//  draw_window(vpu->windows[vpu->regs[4]].window, 0, 1);
  return 0;
}

static int vpu_scrcall_resize_element(struct vpu *vpu)
{
  struct window_element *element;
  struct prog_window *window = vpu->windows[vpu->regs[4]].window;

  if(vpu->regs[1] < vpu->windows[vpu->regs[4]].window->element_count)
    element = vpu->windows[vpu->regs[4]].window->window_grid[vpu->regs[1]];
  else return 0;
  
  element->width = vpu->regs[2];
  element->height = vpu->regs[3];

  window_recalc_hitboxes(window);

  return 0;
}

static int vpu_scrcall_setelementmaxcount(struct vpu *vpu)
{
  struct window_element *element;

  if(vpu->regs[1] < vpu->windows[vpu->regs[4]].window->element_count)
    element = vpu->windows[vpu->regs[4]].window->window_grid[vpu->regs[1]];
  else return 0;

  element->c = vpu->regs[2];

  return 0;
}

static int vpu_scrcall_drawelement(struct vpu *vpu)
{
  struct window_element *element;
  if(vpu->regs[1] < vpu->windows[vpu->regs[4]].window->element_count)
    element = vpu->windows[vpu->regs[4]].window->window_grid[vpu->regs[1]];
  else return 0;

  window_drawelement(vpu->windows[vpu->regs[4]].window, vpu->regs[1]);

  return 0;
}

static int vpu_scrcall_setfont(struct vpu *vpu)
{
  struct window_element *element;

  if(vpu->regs[1] < vpu->windows[vpu->regs[4]].window->element_count)
    element = vpu->windows[vpu->regs[4]].window->window_grid[vpu->regs[1]];
  else return 0;

  element->monospace = vpu->regs[2];

  window_drawelement(vpu->windows[vpu->regs[4]].window, vpu->regs[1]);

  return 0;
}

static int vpu_scrcall_getselectc(struct vpu *vpu)
{
  struct window_element *element;

  if(vpu->regs[1] < vpu->windows[vpu->regs[4]].window->element_count)
    element = vpu->windows[vpu->regs[4]].window->window_grid[vpu->regs[1]];
  else return 0;

  vpu->regs[0] = element->selectc;

  return 0;
}

static int vpu_scrcall_setselectc(struct vpu *vpu)
{
  struct window_element *element;

  if(vpu->regs[1] < vpu->windows[vpu->regs[4]].window->element_count)
    element = vpu->windows[vpu->regs[4]].window->window_grid[vpu->regs[1]];
  else return 0;

  if(element->type == 7) // element is a radio button
    window_radiobutton_select(vpu->windows[vpu->regs[4]].window, element);

  else
    element->selectc = vpu->regs[2];

  return 0;
}

static int vpu_scrcall_getwindowsize(struct vpu *vpu)
{
  struct prog_window *window = vpu->windows[vpu->regs[4]].window;

  vpu->regs[0] = window->width;
  vpu->regs[1] = window->height;

  return 0;
}

static int vpu_scrcall_radiobutton(struct vpu *vpu)
{
  char *text = &vpu->data[vpu->data_segment][vpu->regs[3]];
  struct window_element *element;
  struct window_element *prev_element = 0;

  if(!memory_allowed(vpu, (long)vpu->regs[3]+strlen(text)))
    return 4;

  if(vpu->regs[6] < vpu->windows[vpu->regs[4]].window->element_count)
      prev_element = vpu->windows[vpu->regs[4]].window->window_grid[vpu->regs[6]];

  element = window_radiobutton(vpu->windows[vpu->regs[4]].window, vpu->regs[1], vpu->regs[2], text, vpu->regs[5], prev_element);

  if(!element) vpu->regs[0] = 0xFFFF;
  else
  {
    vpu->regs[0] = 0x00FF + vpu->windows[vpu->regs[4]].window->hitbox_count;
    vpu->regs[1] = vpu->windows[vpu->regs[4]].window->hitbox_count-1;    
  }

  return 0;  
}

int (*vpu_instr_scrcall[])(struct vpu *) =
{                            // AX ==
  vpu_scrcall_createwindow, // 0
  vpu_scrcall_fullscreen, // 1
  vpu_scrcall_update, // 2
  vpu_scrcall_clear, // 3
  vpu_scrcall_text, // 4
  vpu_scrcall_putpixel, // 5
  vpu_scrcall_memset, // 6
  vpu_scrcall_memcpy, // 7
  vpu_scrcall_button, // 8
  vpu_scrcall_textbox, // 9
  vpu_scrcall_list, // 10
  vpu_scrcall_scrollbar, // 11
  vpu_scrcall_icon, // 12
  vpu_scrcall_circle, // 13
  vpu_scrcall_line, // 14
  vpu_scrcall_rectangle, // 15
  vpu_scrcall_mltext, // 16
  vpu_scrcall_updatemltext, // 17
  vpu_scrcall_temptext, // 18
  vpu_scrcall_textbox_update, // 19
  vpu_scrcall_dialog, // 20
  vpu_scrcall_xpm2img, // 21
  vpu_scrcall_delelement, // 22
  vpu_scrcall_resize_element, // 23
  vpu_scrcall_move_element, // 24
  vpu_scrcall_checkbox, // 25
  vpu_scrcall_getselectc, // 26
  vpu_scrcall_setselectc, // 27
  vpu_scrcall_radiobutton, // 28
  vpu_scrcall_textbuff, // 29
  vpu_scrcall_setelementmaxcount, // 30
  vpu_scrcall_drawelement, // 31
  vpu_scrcall_setfont, // 32
  vpu_scrcall_getwindowsize // 33
};

static int vpuwin_disk_busy(struct vpu *vpu)
{
  unsigned function = vpu->regs[0];
  if(function <= 1
     || function == 11
     || function == 12
     || function == 21
     || function == 31)
    if(diskio_in_progress)
    {
      vpu_block(vpu);
      return 1;
    }
  return 0;

}

static int vpu_instr_scr(struct vpu *vpu, unsigned flags)
{
  struct prog_window *oldrw = running_window;
  unsigned char __far *vgap;
  unsigned draw;
  unsigned retval;

  if(vpuwin_disk_busy(vpu))
  {
    return 3;
  }
  
  vpu->sys_wait = 1;

  if(vpu->regs[0] < sizeof(vpu_instr_scrcall) / sizeof(void*))
  {
    if(vpu->regs[0] > 2 && vpu_instr_scrcall[vpu->regs[0]] != vpu_scrcall_dialog)
    {
      if(vpu->windows[vpu->regs[4]].window)
      {
        vgap = VGA;
        running_window = vpu->windows[vpu->regs[4]].window;      
        if(vpu->windows[vpu->regs[4]].renderer && !vpu->windows[vpu->regs[4]].window->minimized)
          VGA = vpu->windows[vpu->regs[4]].renderer;
        draw = (*vpu_instr_scrcall[vpu->regs[0]])(vpu);

        if(draw)
        {
//          draw_window(vpu->windows[vpu->regs[4]].window, 0, 1);
          vpu->windows[vpu->regs[4]].window->redraw = 1;
        }
        running_window = oldrw;
        VGA = vgap;
      }
      else
      {
        putstr(nowindowerror);      
        vpu->regs[0] = 0xFFFF;
      }
    }
    else
      retval = (*vpu_instr_scrcall[vpu->regs[0]])(vpu);
  }
  else
    retval = 4;

  vpu->sys_wait = 0;

  return retval;
}

static void vpu_getwindowkeys(struct vpu *vpu)
{
  unsigned n = MAX_WINDOWS;
  static unsigned char specialkey = 0;
  {
    while(n--)
      if(vpu->windows[n].window && vpu->windows[n].keypress_int && !vpu->windows[n].window->minimized)
      {
        if(specialkey && vpu->ports[vpu->windows[n].keypress_port] == -1)
        {
          vpu->ports[vpu->windows[n].keypress_port] = ((struct vpu_progwindow *)vpu->windows[n].window->function_status)->keybuff[1];
          specialkey = 0;
        }
        else if(( (struct vpu_progwindow *)vpu->windows[n].window->function_status )->keybuff[0] != -1)
        {
//          vpu->interrupt_wait = vpu->windows[n].keypress_int;
          if(send_vpu_signal(vpu, vpu->windows[n].keypress_int))
            return;
          vpu->ports[vpu->windows[n].keypress_port] = ((struct vpu_progwindow*)vpu->windows[n].window->function_status)->keybuff[0];
          if(!((struct vpu_progwindow *)vpu->windows[n].window->function_status)->keybuff[0])
            specialkey = 1;             
          ((struct vpu_progwindow *)vpu->windows[n].window->function_status)->keybuff[0] = -1;
        }
        break;
      }
  }
}
