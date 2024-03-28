static void update_envvars(void)
{
  path = getenv("PATH");
  comspec = getenv("COMSPEC");
  if(!comspec) comspec = "COMMAND.COM";
}
