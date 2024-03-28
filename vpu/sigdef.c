int send_vpu_signal(struct vpu *vpu, unsigned short signal);

#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGILL 4
#define SIGTRAP 5
#define SIGABRT 6
#define SIGEMT 7
#define SIGFPE 8
#define SIGKILL 9
#define SIGBUS 10
#define SIGSEGV 11
#define SIGSYS 12
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15
#define SIGUSR1 16
#define SIGUSR2 17
#define SIGCHLD 18
#define SIGPWR 19
#define SIGWINCH 20
#define SIGURG 21
#define SIGPOLL 22
#define SIGSTOP 23
#define SIGTSTP 24
#define SIGCONT 25
#define SIGTTIN 26
#define SIGTTOU 27
#define SIGVTALRM 28
#define SIGPROF 29
#define SIGXCPU 30
#define SIGXFSZ 31
#define SIGWAITING 32
#define SIGLWP 33


#define SIGHIDECON 128
#define SIGSHOWCON 129
#define SIGMUTLOCK 130
#define SIGMUTUNLOCK 131
#define SIGFDRECVD 132
#define SIGWINRESIZE 133
