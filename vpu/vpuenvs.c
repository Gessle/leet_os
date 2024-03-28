static int vpu_addenv(struct vpu *vpu, char *value)
{
  void *new_ptr;
  if(!vpu->env_count++)
  {
    vpu->envs = malloc(sizeof(char*));
    if(!vpu->envs)
    {
      error:
      vpu->env_count--;
      return -1;
    }
  }
  else
  {
    new_ptr = realloc(vpu->envs, vpu->env_count*sizeof(char*));
    if(!new_ptr) goto error;
    vpu->envs = new_ptr;
  }

  if(!(vpu->envs[vpu->env_count-1] = malloc(strlen(value)+1)))
  {
    vpu->envs = realloc(vpu->envs, --vpu->env_count*sizeof(char*));
    return -1;
  }

  strcpy(vpu->envs[vpu->env_count-1], value);

  return 0;
}

static int init_envs(struct vpu *vpu)
{
  unsigned env_count;
  char **envptr;

  for(env_count=0,envptr=environ;*envptr;envptr++,env_count++);

  while(env_count--)
    if(vpu_addenv(vpu, environ[env_count]))
      return 1;

  return 0;
}

static int vpu_findenv(struct vpu *vpu, char *name)
{
  unsigned namelen = strlen(name);
  unsigned envnum;

  name[namelen] = '=';

  for(envnum=vpu->env_count;envnum--;)
    if(!strncmp(name, vpu->envs[envnum], namelen))
      break;
  name[namelen] = 0;
  return envnum;
}

static int vpu_unsetenv(struct vpu *vpu, char *name)
{
  unsigned envnum = vpu_findenv(vpu, name);

  if(envnum == -1) return -1;

  free(vpu->envs[envnum]);
  vpu->env_count--;
/*  while(envnum<vpu->env_count)
  {
    vpu->envs[envnum] = vpu->envs[envnum+1];
    envnum++;
  }*/
  memmovel(&vpu->envs[envnum], &vpu->envs[envnum+1], (vpu->env_count-envnum)*sizeof(char*));
  vpu->envs = realloc(vpu->envs, vpu->env_count*sizeof(char*));

  return 0;
}

static void vpu_format_env(char *target, char *name, char *value)
{
  sprintf(target, "%s=%s", name, value);
}

static int vpu_setenv(struct vpu *vpu, char *name, char *value, unsigned overwrite)
{
  unsigned envnum;
  char *new_ptr;
  int rc;

  envnum = vpu_findenv(vpu, name);
  if(envnum != -1)
  {
    if(!overwrite)
      return 0;
    new_ptr = realloc(vpu->envs[envnum], strlen(name)+1+strlen(value)+1);
    if(!new_ptr)
      return -1;

    vpu_format_env(new_ptr, name, value);
    vpu->envs[envnum] = new_ptr;
    return 0;
  }
  else
  {
    new_ptr = malloc(strlen(name)+1+strlen(value)+1);
    if(!new_ptr) return -1;
    vpu_format_env(new_ptr, name, value);
    rc = vpu_addenv(vpu, new_ptr);
    free(new_ptr);
    return rc;
  }
}

static int vpu_copy_envs(struct vpu *vpu0, struct vpu *vpu1)
{
  vpu0->env_count = 0;
  while(vpu0->env_count < vpu1->env_count)
    if(vpu_addenv(vpu0, vpu1->envs[vpu0->env_count]))
    {
      while(vpu0->env_count)
        free(vpu0->envs[--vpu0->env_count]);
      free(vpu0->envs);
      return -1;
    }

  return 0;
}

static struct vpu *vpu_shares_envs(struct vpu *vpu)
{
  unsigned n = vpu_count;
  while(n--)
    if(vpus[n] != vpu && vpus[n]->envs == vpu->envs)
      return vpus[n];
  return 0;
}
