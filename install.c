#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h> 
#include <direct.h>
#include <conio.h>
#include <errno.h>


char *base_system_files[] =
{
  "MAIN.EXE",
  "ASCSFX.COM",
  "SCEDITOR.COM",
  "CONFIG.COM",
  "SETUP.INI",
  "FILEASSC.DAT",
  "SCRNSVRS\\MATRIX.COM",
  "SCRNSVRS\\VGASTARS.COM",
  "SCRNSVRS\\CGASTARS.COM",
  "NORMAL.CUR",
  "HOURGLAS.CUR",
  "FONTS\\CGA.FON",
  "FONTS\\CGAMON.FON",
  "FONTS\\VGA.FON",
  "FONTS\\VGAMON.FON",
  "ICONS\\CLOCK.ICO",
  "ICONS\\FILEMAN.ICO",
  "ICONS\\ASCSFX.ICO",
  "ICONS\\COMMAND.ICO",
  "ICONS\\CPANEL.ICO",
  "ICONS\\SCEDITOR.ICO",
  "ICONS\\WNDWDFLT.ICO",
  "ICONS\\VPUCON.ICO",
  "ICONS\\RUNPRMPT.ICO",
  "ICONS\\CONFIG.ICO",
  "DESKTOP\\COMMAND.LNK"            
};

char *doc_files[] =
{
  "DOCS\\ASCSFX.TXT",
  "DOCS\\CONFIG.TXT",
  "DOCS\\CUREDIT.TXT",
  "DOCS\\FONTEDIT.TXT",
  "DOCS\\ICONEDIT.TXT",
  "DOCS\\INSTRLST.TXT",
  "DOCS\\KEYBOARD.TXT",
  "DOCS\\MANUAL.TXT",
  "DOCS\\README.TXT",
  "DOCS\\SCEDITOR.TXT",
  "DOCS\\SCRCALLS.TXT",
  "DOCS\\SHORTCUT.TXT",
  "DOCS\\SIGNALS.TXT",
  "DOCS\\STSHELL.TXT",
  "DOCS\\SYSCALLS.TXT"
};

char *utilities[] =
{
  "ICONEDIT.COM",
  "FONTEDIT.COM",
  "CUREDIT.COM",
  "ASSEMBLE.EXE",  
  "2XPM2.EXE",
  "TEXTEDIT.COM",
  "DESKTOP\\TEXTEDIT.LNK",
  "STSHELL.COM",
  "XPMVIEW.APP",
  "XPMVIEW.ASM",
  "DOSH.APP",
  "DOSH.ASM",
  "DESKTOP\\CONSOLE.LNK",
  "DOSH\\LS",
  "LS.ASM",
  "DOSH\\CAT",
  "CAT.ASM",
  "DOSH\\TOLOWER",
  "TOLOWER.ASM",
  "ALARM.APP",
  "ALARM.ASM",
  "DOSH\\BENCH",
  "BENCH.ASM",
  "DOSH\\ECHO",
  "ECHO.ASM",
  "DOSH\\PROCINFO",
  "PROCINFO.ASM",
  "DOSH\\MORE",
  "MORE.ASM",
  "DOSH\\GREP",
  "GREP.ASM",
  "NOTEPAD.APP",
  "DOSH\\UPTIME",
  "DOSH\\KILL",
  "DOSH\\UNLOAD"
};

char *dir_list[] =
{
  "SCRNSVRS",
  "FONTS",
  "DESKTOP",
  "ICONS",
  "DRIVERS",
  "STARTUP",
  "DOCS",
  "DOSH"
};

int copy_file(char *src, char *dst)
{
  FILE *srcfp = fopen(src, "rb");
  FILE *dstfp = fopen(dst, "wb");
  int c;
  if(!srcfp || !dstfp)
    return 1;
  while((c = fgetc(srcfp)) != EOF)
    fputc(c, dstfp);
  fclose(srcfp);
  fclose(dstfp);
  return 0;
}

int copy_files(char *install_dir, char **file_list, unsigned file_count)
{
  unsigned n;
  char dst_path[100];

  while(file_count--)
  {
    sprintf(dst_path, "%s\\%s", install_dir, file_list[file_count]);
    printf("Copying file %s\n", dst_path);
    if(copy_file(file_list[file_count], dst_path))
      return 1;
  }
  return 0;
}

int create_dirs(char *install_dir)
{
  char dir[100];
  unsigned n;
  if(mkdir(install_dir))
  {
    if(errno != EACCES)
      return 1;
  }
  for(n=0;n<sizeof(dir_list)/sizeof(char*);n++)
  {
    sprintf(dir, "%s\\%s", install_dir, dir_list[n]);
    printf("Creating directory %s\n", dir);
    if(mkdir(dir))
    {
      if(errno != EACCES)
        return 1;
    }
  }
  return 0;
}

unsigned utilities_installed = 0;

int autoexec_append(char *install_dir)
{
  char install_drive = *install_dir;
  char autoexec_bat[100];
  char message[100];
  FILE *fp;
  sprintf(autoexec_bat, "%c:\\AUTOEXEC.BAT", install_drive);
  if(!(fp = fopen(autoexec_bat, "r")))
  {
    sprintf(message, "Failed opening %s, trying C:\\AUTOEXEC.BAT\n", autoexec_bat);
    puts(message);
    sprintf(autoexec_bat, "C:\\AUTOEXEC.BAT");
    if(!(fp = fopen(autoexec_bat, "r")))
      return 1;
  }
  fclose(fp);
  fp = fopen(autoexec_bat, "a");
  if(utilities_installed)
    fprintf(fp, "SET PATH=%s\\DOSH;%%PATH%%", install_dir);
  fprintf(fp, "\nCD %s\n", install_dir);
  fputs("MAIN.EXE\n", fp);
  fclose(fp);
  return 0;
}

int main()
{
  char install_dir[100] = "C:\\LEETOS";
  char input[100];
  int c;
  puts("Welcome to lEEt/OS installer!");
  puts("Press CTRL+C to exit the installer without installing lEEt/OS.");
  puts("Enter full path to install directory (default: C:\\leetos)");
  fgets(input, 100, stdin);
  if(input[0] != '\n')
  {
    strcpy(install_dir, input);
    install_dir[strlen(install_dir)-1] = 0;
  }
  printf("Installing to %s\n", install_dir);
  puts("Creating directories");
  if(create_dirs(install_dir))
  {
    printf("Error while creating directories: %s", strerror(errno));
    return 1;
  }
  puts("Copying base system files");
  if(copy_files(install_dir, base_system_files, sizeof(base_system_files)/sizeof(char*)))
  {
    puts("Error while copying files.");
    return 1;
  }
  puts("Ready!");
  puts("Do you want to install documentation? Y/n");
  c = fgetc(stdin);
  if(c != '\n')
    fgetc(stdin);
  if((c & 0x1F) != ('n' & 0x1F))
    if(copy_files(install_dir, doc_files, sizeof(doc_files)/sizeof(char*)))
      puts("Error while copying documentation.");
    else puts("Ready!");
  puts("Do you want to install utilities? Y/n");
  c = fgetc(stdin);
  if(c != '\n')
    fgetc(stdin);
  if((c & 0x1F) != ('n' & 0x1F))
  {
    utilities_installed = ~utilities_installed;
    if(copy_files(install_dir, utilities, sizeof(utilities)/sizeof(char*)))
      puts("Error while copying utilities.");
    else puts("Ready!");
  }
  puts("Make changes to AUTOEXEC.BAT so that lEEt/OS starts up automatically when the computer boots? Y/n");
  c = fgetc(stdin);
  if(c != '\n')
    fgetc(stdin);
  if((c & 0x1F) != ('n' & 0x1F))
    if(autoexec_append(install_dir))
      puts("Error");
    else puts("Done");
  sprintf(input, "Run %s\\CONFIG.COM if you experience problems with video settings.", install_dir);
  puts(input);
  return 0;
}
