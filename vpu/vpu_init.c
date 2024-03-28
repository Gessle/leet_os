#define FIRST_FD 3
#include "vpustrct.c"

struct vpu **vpus;
unsigned vpu_count = 0;
unsigned pid_counter = 0;

static struct vpu *vpu_shares_envs(struct vpu *);

static int init_vpu(struct vpu *vpu, unsigned short stack_size, unsigned short code_size, unsigned short code_count, unsigned short 
data_size, unsigned short data_count, unsigned short interrupt_count)
{
  unsigned n;
  
  vpu->priority = DEFAULT_PRIORITY;
  vpu->sp = vpu->psp = vpu->ssp = stack_size-1;

  vpu->byteregs = (unsigned char*)vpu->regs;

  vpu->primary_stack = calloc(stack_size, sizeof(unsigned short));
  vpu->secondary_stack = calloc(stack_size, sizeof(unsigned short));
  if(!vpu->primary_stack || !vpu->secondary_stack)
    return 0;
  vpu->stack = vpu->primary_stack;

  vpu->stack_size = stack_size;    

  vpu->code = malloc(code_count * sizeof(char*));
  if(!vpu->code)
    return 0;

  vpu->realcode = vpu->code;

  while(vpu->code_count < code_count)
  {    
    vpu->code[vpu->code_count] = calloc(code_size+10, sizeof(char));
    if(!vpu->code[vpu->code_count])
      return 0;
    vpu->code_count++;
  }
  vpu->code_size = code_size;  

  vpu->data = malloc(data_count * sizeof(char*));
  if(!vpu->data)
    return 0;

  vpu->dataseg_table_len = data_count;  
  
  while(vpu->data_count < data_count)
  {
    vpu->data[vpu->data_count] = calloc(data_size, sizeof(char));
    if(!vpu->data[vpu->data_count])    
      return 0;
    vpu->data_count++;
  }

  vpu->data_size = data_size;  

  vpu->interrupts = calloc(interrupt_count, sizeof(struct vpu_interrupt));
  if(!vpu->interrupts)
    return 0;

  vpu->interrupt_count = interrupt_count;

  vpu->ports = calloc(VPU_PORT_COUNT, sizeof(unsigned short));
  if(!vpu->ports)
    return 0;
  vpu->port_count = VPU_PORT_COUNT;

  if(!vpu->file_struct)
  {

    vpu->file_struct = calloc(MAX_FILES, sizeof(struct file_struct*));
    if(!vpu->file_struct) return 0;
    for(n=MAX_FILES;n--;)
    {
      vpu->file_struct[n] = calloc(1,sizeof(struct file_struct));
      if(!vpu->file_struct[n]) return 0;
    }

    vpu->dir_handles = calloc(MAX_DIRS, sizeof(struct dir_struct*));
    if(!vpu->dir_handles) return 0;
  }

  if(!*vpu->iwd)
    getcwd(&vpu->iwd, MAX_PATH);

  vpu->privileges = DEFAULT_PRIVS;

  return 1;
}

static struct vpu *spawn_new_vpu(void)
{
  void *new_pointer;
  if(!vpu_count++)
  {
    vpus = malloc(vpu_count * sizeof(struct vpu*));
    if(!vpus)
    {
      outofmemory0:
      vpu_count--;
      return 0;
    }
  }
  else
  {
    new_pointer = realloc(vpus, vpu_count*sizeof(struct vpu*));
    if(!new_pointer)
    {
      goto outofmemory0;
    }
    vpus = new_pointer;    
  }
  if(!(vpus[vpu_count-1] = calloc(1, sizeof(struct vpu))))
  {
    if(--vpu_count)
      vpus = realloc(vpus, vpu_count*sizeof(struct vpu*));
    else
      free(vpus);      
    return 0;
  }

  while(pid_to_vpu(++pid_counter));
  vpus[vpu_count-1]->pid = pid_counter;
  return vpus[vpu_count-1];  
}

static void delete_child(struct vpu *child)
{
  struct vpu *parent = child->parent;
  unsigned n = parent->child_count--;

  if(!parent->child_count)
  {
    free(parent->child);
    parent->child=0;
    return;
  }
  while(n--)
    if(parent->child[n] == child)
      break;

  memmovel(&parent->child[n], &parent->child[n+1], (parent->child_count-n)*sizeof(struct vpu*));

  parent->child = realloc(parent->child, parent->child_count* sizeof(struct vpu*));
}

inline int vpu_shares_args(struct vpu *vpu)
{
  unsigned n = vpu_count;
  while(n--)
    if(vpus[n] != vpu && vpu->argv == vpus[n]->argv)
      return 1;
  return 0;
}

static void free_vpu_args(struct vpu *vpu)
{  
  if(vpu->argv)
  {
    if(vpu_shares_args(vpu))
      vpu->argc = 0;
    else
    {
      while(vpu->argc)
        free(vpu->argv[--vpu->argc]);
      free(vpu->argv);
    }
    vpu->argv=0;
  }
}

static void free_vpu_envs(struct vpu *vpu)
{
  if(vpu->envs)
  {
    if(vpu_shares_envs(vpu))
      vpu->env_count = 0;
    else
    {
      while(vpu->env_count)
        free(vpu->envs[--vpu->env_count]);
      free(vpu->envs);
    }
    vpu->envs = 0;
  }
}

static void vpu_free_dynamic_segment(struct vpu *vpu, unsigned short segment);

static int send_termination_signal(struct vpu *vpu)
{
  if(vpu->termination_signal && vpu->parent)
  {
    if(!vpu->parent->zombie)
      if(!send_vpu_signal(vpu->parent, vpu->termination_signal))
      {
        vpu->termination_signal = 0;
        return 0;
      }
      else return 1;
  }
  return 0;
}

static void free_vpu_resources(struct vpu *vpu)
{
  unsigned n;
  struct vpu *parent;

  for(n=vpu_hwint_count;n--;)
    if(vpu_hardware_int_table[n].vpu == vpu)
      remove_vpu_hw_int(vpu, vpu_hardware_int_table[n].vpu_int);  

  vpu_kill_clones(vpu);

  free_vpu_args(vpu);
  free_vpu_envs(vpu);

  if(vpu->is_clone) goto clonekill;
 
  if(vpu->dir_handles)        
    for(n = MAX_DIRS;n--;)
      if(vpu->dir_handles[n])
        vpu_dd_close(vpu, n);
  free(vpu->dir_handles);

  if(vpu->file_struct)  
    for(n=MAX_FILES;n--;)
    {
      if(vpu->file_struct[n]->open)
        vpu_fd_close(vpu, n);
      free(vpu->file_struct[n]);
    }
  free(vpu->file_struct);

  while(vpu->dynsegment_count)
    vpu_free_dynamic_segment(vpu, 0);
 
  while(vpu->data_count--)
    free(vpu->data[vpu->data_count]);
  free(vpu->data);

  while(vpu->code_count--)
    free(vpu->code[vpu->code_count]);
  free(vpu->code);

  free(vpu->interrupts);
  free(vpu->ports);

  for(n=MAX_WINDOWS;n--;)
    if(vpu->windows[n].window)
    {
      del_window(vpu->windows[n].window);
      free(vpu->windows[n].renderer);
    }

  clonekill:
  free(vpu->primary_stack);  
  free(vpu->secondary_stack);

  vpu->zombie = 1;
}

static void delete_vpu(struct vpu *vpu)
{
  unsigned n;

  if(!vpu) return;

  if(!vpu->zombie)
    free_vpu_resources(vpu);

  while(vpu->child_count)
    delete_vpu(vpu->child[0]);
 
  if(vpu->parent)
    delete_child(vpu);

//  free_vpu_args(vpu);

  for(n=0;vpus[n]!=vpu;n++);
  vpu_count--;
  memmovel(&vpus[n], &vpus[n+1], (vpu_count-n)*sizeof(struct vpu *));

  free(vpu);
   
  if(vpu_count)
    vpus = realloc(vpus, vpu_count*sizeof(struct vpu*));
  else
  {
    free(vpus);
    vpus = 0;
  }
}

