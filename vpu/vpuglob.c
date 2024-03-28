#define MAX_PATH 80
static unsigned char vpu_segment_in_use(struct vpu *vpu, unsigned short segment);
int getchr(void);
struct int21time *dostime(void);
struct int21date *dosdate(void);
inline void vpu_block(struct vpu *vpu);
void draw_xpm_bitmap(struct xpm_bitmap *bitmap, unsigned x0, unsigned y0, unsigned x1, unsigned y1);
int load_xpm_bitmap(char *filename, struct xpm_bitmap *retval);
void vpu_cwd(struct vpu *vpu);
void vpuconsole_printtxt(struct prog_window *window, unsigned char *string);
//void vpuconsole_printchar(struct prog_window *window, unsigned char chr);
//void vpuconsole_printtxt_l(struct prog_window *window, unsigned char *string, unsigned length);
void flush_stdout(void);
//unsigned disable_vpu_interrupts = 0;
int send_vpu_signal(struct vpu *, unsigned short);
static struct vpu *pid_to_vpu(unsigned);

static void vpu_kill_clones(struct vpu *);

void log_error(char *);

void append_path(struct vpu *, char *, char *);

extern int videodriver;
extern char outofmemory[];
unsigned DEFAULT_PRIVS = 0xFFFF;

extern volatile signed next_window;

unsigned starttime_high, starttime_low;

static unsigned char memory_allowed(struct vpu *vpu, unsigned long offset);
static unsigned segment_exists(struct vpu *vpu, unsigned);
static void segfault(struct vpu *vpu);

char stackoverflowerror[] = "Stack overflow!";
char stackunderflowerror[] = "Stack underflow!";
char segfaulterror[] = "Segmentation violation!";

void vpu_sched_yield(struct vpu *vpu);

extern struct prog_window *draw_window_box;

volatile extern unsigned __far clock_ticks;

extern unsigned char __based(__segname("_TEXT")) diskio_in_progress;
