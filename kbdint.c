#define ESCAPE_SCANCODE 0xe0
#define ESCAPE2_SCANCODE 0xe1 // for pause/break
#define KEY_RELEASE 0x80
#define SCANCODE_ESCAPE 0x01

#define SCANCODE_LSHIFT 0x2A
#define SCANCODE_LALT 0x38
// RALT/ALTGR: ESCAPE + LALT
#define SCANCODE_LCTRL 0x1D
// RCTRL: ESCAPE + LCTRL
#define SCANCODE_C 0x2E
#define SCANCODE_S 0x1F
#define SCANCODE_Q 0x10

#define SCANCODE_DEL 0x53
#define SCANCODE_BREAK 0x46

#define SCANCODE_F1 0x3B
#define SCANCODE_ALT1 120
#define SCANCODE_ALTF1 104
#define SCANCODE_CTRLF1 0x5E

#define SCANCODE_LEFT 75
#define SCANCODE_RIGHT 77

void bios_putchar(int chr);
#pragma aux bios_putchar = \
  "push bx" \
  "mov ah, 0x0E" \
  "mov bx, 0x0007" \
  "int 0x10" \
  "pop bx" \
  parm [ax];

inline void bios_putstr(char *str)
{
  while(*str)
    bios_putchar(*str++);
  bios_putchar(0x0D);
  bios_putchar(0x0A);
}

extern volatile char new_tty;
extern volatile unsigned char tty;

struct keyboard_status keyboard_status;

static void end_kbd_int(void)
{
  unsigned char ctrl_byte = inbyte(0x61);
  outbyte(0x61, ctrl_byte | 0x80);
  outbyte(0x61, ctrl_byte);
  outbyte(0x20, 0x20);
}

inline void ctrlaltdel_handler(void)
{
  end_kbd_int();
}

void (__interrupt __far *old_kbd_handler)(void);

static void __interrupt __far keyboard_handler(void)
{
  unsigned char keycode = inbyte(0x60);

  if(!keyboard_status.prev_escape)
  {
    if(keycode & KEY_RELEASE)
      if(keycode == ESCAPE_SCANCODE)    
        keyboard_status.prev_escape = 1;
      else
        switch(keycode & 0x7F)
        {
          case SCANCODE_LALT:
            keyboard_status.alt_pressed = 0;
            break;
          case SCANCODE_LCTRL:
            keyboard_status.lctrl_pressed = 0;
            break;
          case SCANCODE_LSHIFT:
            keyboard_status.shift_pressed = 0;
            break;
        }
    else
      if(keycode == ESCAPE2_SCANCODE)
      {
        return;
      }
      if(keyboard_status.lctrl_pressed && 
         keycode >= SCANCODE_F1 && keycode < SCANCODE_F1 + 10)
      {
        new_tty = keycode - SCANCODE_F1;
        end_kbd_int();
        return;
      }
      switch(keycode)
      {
        case SCANCODE_LALT:
          keyboard_status.alt_pressed = 1;
          break;
        case SCANCODE_LCTRL:
          keyboard_status.lctrl_pressed = 1;
          break;
        case SCANCODE_LSHIFT:
          keyboard_status.shift_pressed = 1;
          break;
        case SCANCODE_C:
          if(keyboard_status.lctrl_pressed || keyboard_status.rctrl_pressed)
          {
            int23h:
//            _chain_intr(int23handler);
//            int23handler();
//            return;
            ungetch(3);
            end_kbd_int();
            return;
          }
          break;
        case SCANCODE_S:
          if(keyboard_status.lctrl_pressed || keyboard_status.rctrl_pressed)
          {        
            ungetch(19);
            end_kbd_int();
            return;
          }
        case SCANCODE_Q:
          if(keyboard_status.lctrl_pressed || keyboard_status.rctrl_pressed)
          {        
            ungetch(17);
            end_kbd_int();
            return;
          }
      }
  }
  else
  {
    keyboard_status.prev_escape = 0;
    if(keycode & KEY_RELEASE)
      switch(keycode & 0x7F)
      {
        case SCANCODE_LCTRL:
          keyboard_status.rctrl_pressed = 0;
          break;          
      }
    else
    {
      switch(keycode)
      {
        case SCANCODE_LCTRL:
          keyboard_status.rctrl_pressed = 1;
          break;                  
        case SCANCODE_DEL:
          if(keyboard_status.lctrl_pressed && keyboard_status.alt_pressed)
          {
            ctrlaltdel_handler();               
            return;
          }
          break;
        case SCANCODE_BREAK:
          goto int23h;
      }
    }
  }
  
  _chain_intr(old_kbd_handler);
}

void set_keyboard_handler(void)
{
  old_kbd_handler = _dos_getvect(0x9);
  _dos_setvect(0x9, keyboard_handler);
  memset(&keyboard_status, 0, sizeof(struct keyboard_status));
}

void reset_keyboard_handler(void)
{
  _dos_setvect(0x9, old_kbd_handler);
}
