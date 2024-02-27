
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAXLINE    1024   
#define MAXARGS     128   
#define MAXJOBS      16   
#define MAXJID    1<<16   


#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

/* 
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

struct job_t {              /* The job struct */
    pid_t pid;              /* job PID */
    int jid;                /* job ID [1, 2, ...] */
    int state;              /* UNDEF, BG, FG, or ST */
    char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);

void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);

/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv); 
void sigquit_handler(int sig);

void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs); 
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid); 
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid); 
int pid2jid(pid_t pid); 
void listjobs(struct job_t *jobs);

void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);


int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; 

    dup2(1, 2);

    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute read/eval loop */
    while (1) {

	/* Read command line */
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}
	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) */
	    fflush(stdout);
	    exit(0);
	}

	/* Evaluate the command line */
	eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    } 

    exit(0); 
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a (quit, jobs, bg or fg)
 * then execute it immediately. 
   Otherwise, fork a child process and
   run the job in the context of the child. 
   If the job is running in
   the foreground, wait for it to terminate and then return.  
   
   Note:
 * each child process must have a unique process group ID so that our
   background children don't receive SIGINT (SIGTSTP) from the kernel
   when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline) 
{
     int state = UNDEF;
     char* argv[MAXARGS];
     sigset_t set;
     pid_t pid;
     if(parseline(cmdline,argv) == 1) {//将cmdline中字符串赋给argv
        state == BG; //状态置为后台运行
     }
     else state = FG;
     if(argv[0] == 0)
        return;
     else {
        if(builtin_cmd(argv) != 0)//如果命令已经内置
           return;
        else {
           if(sigemptyset(&set) < 0) { //往set中清空信号
              unix_error("sigemptyset error");
           } 
           if(sigaddset(&set,SIGINT) < 0) {//添加信号
              unix_error("sigaddset error");
           }
           if(sigaddset(&set,SIGTSTP) < 0) {
              unix_error("sigaddset error");
           }
           if(sigaddset(&set,SIGCHLD) < 0) {
              unix_error("sigaddset error");
           }
           if(sigprocmask(SIG_BLOCK,&set,NULL) < 0) {//设置为屏蔽信号
              unix_error("sigprocmask error");
           }
           if((pid = fork()) < 0) {//创建失败
              unix_error("fork error");
           } 
           else if(pid ==0) {
                   if(sigprocmask(SIG_UNBLOCK, &set, NULL) < 0)  //解除阻塞
                      unix_error("sigprocmask error");
                   if(setpgid(0,0) < 0) {
                      unix_error("setpgid error");
                   }
                   if(execve(argv[0],argv,environ) < 0) {//execve通过寻址创建一个新进程
                      printf("%s: Command not found\n",argv[0]);
                      return;
                   }
           }
           addjob(jobs,pid,state,cmdline);
           if(sigprocmask(SIG_UNBLOCK,&set,NULL) < 0) {
              unix_error("sigprocmask error");
           }
           if(state == FG)
            waitfg(pid);  //前台作业等待
           else
            printf("[%d] (%d) %s", pid2jid(pid), pid, cmdline);   
        }
     }
    

    return;
}
/* 
 * parseline - Parse and build the argv array.
 * 
 * Return true if the user has requested a BG job, false if
   the user has requested a FG job.  
 */
int parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* local copy of command line */
    char *buf = array;          
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* if background job */

    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
	buf++;
	delim = strchr(buf, '\'');
    }
    else {
	delim = strchr(buf, ' ');
    }

    while (delim) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* ignore spaces */
	       buf++;

	if (*buf == '\'') {
	    buf++;
	    delim = strchr(buf, '\'');
	}
	else {
	    delim = strchr(buf, ' ');
	}
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* ignore blank line */
	return 1;

    /* should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0) {
	argv[--argc] = NULL;
    }
    return bg;
}

/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 */
int builtin_cmd(char **argv) 
{
    if(!strcmp(argv[0],"quit")) {
       exit(0);
       return 1;
    }
    else if(!strcmp(argv[0],"jobs")) {
            listjobs(jobs);
            return 1;
    }
    else if(!strcmp(argv[0],"bg") || !strcmp(argv[0],"fg")) {
            do_bgfg(argv);
            return 1;
    }
    /*else if(argv[0][0] != '&')
            return 0;
    else if(argv[0][1] == NULL)
            return 1;*/ //not a built-in command
    else    return 0;
}

/* 
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv)
{
    struct job_t *job;
    int val;
    if(argv[1] == NULL) {
       printf("%s command requires PID or %%jobid argument\n",argv[0]);
       return;
    }

    if(argv[1][0] == '%') { 
       val = strtol(argv[1][1],NULL,10);
       job = getjobjid(jobs,val);
       if(val <= 0) {
          printf("%s: argument must be a PID or %%jobid\n",argv[0]);
          return;
       }
       if(job == NULL) {  
          printf("%s: No such job\n",val);
          return;
       }
    }
    else { 
       val = strtol(argv[1],NULL,10);
       job = getjobpid(jobs,val);
       if(val <= 0) {
          printf("%s: argument must be a PID or %%jobid\n",argv[0]);
          return;
       }
       if(job == NULL) {
          printf("(%d): No such process\n",val);
          return;
       }
    }
    
    if(!strcmp(argv[0],"bg")) { 
          job->state = BG;
          if(kill(-job->pid,SIGCONT) < 0) { 
             unix_error("kill (bg) error");
          }
          printf("[%d] (%d) %s",job->jid,job->pid,job->cmdline);
          return;
       }
       else if(!strcmp(argv[0],"fg")) {
          job->state = FG; 
          if(kill(-job->pid,SIGCONT) < 0) { 
             unix_error("kill (fg) error");
          }
          waitfg(job->pid);
          return;
       }
       else {
          puts("do_bgfg: Internal error");
          exit(0);
       }
    return;
}

/* 
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid)
{
    struct job_t *j = getjobpid(jobs,pid);
    if(j == 0) { 
       return;
    }
    /*else if(j.pid == pid) {
            if(j.state != 1) {
               if(verbose == 0)
                  return;
               else {
                   printf("waitfg: Process (%d) no longer the fg process\n",pid);
                   return;
               }
             }
             do {
               sleep(1);
             } while(j.state == 1);

             if(verbose == 0)
                  return;
               else {
                   printf("waitfg: Process (%d) no longer the fg process\n",pid);
                   return;
             }
    }
    else {
               if(verbose == 0)
                  return;
               else {
                   printf("waitfg: Process (%d) no longer the fg process\n",pid);
                   return;
               }
    }*/

    //由于verbose始终为0，j.pid也始终等于pid，上述代码可简化为
    do {
      sleep(1);
    } while(j->state == 1);
    return;
}

/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
       a child job terminates (becomes a zombie), or stops because it
       received a SIGSTOP or SIGTSTP signal. 
       The handler reaps all available zombie children, 
       but doesn't wait for any other currently running children to terminate.  
 */
void sigchld_handler(int sig)
{
    pid_t pid;
    int jid;
    int status;
    struct job_t* job;
    if(verbose) {
       puts("sigchld_handler: entering");
    }
    while((pid = waitpid(-1,&status,WNOHANG | WUNTRACED)) > 0) {   
           job = getjobpid(jobs,pid);
           if(job == NULL) {
              printf("Lost track of (%d)\n",pid);
              return;
           }
           jid = job->jid;
           if(WIFSTOPPED(status)) {
              printf("Job [%d] (%d) stopped by signal %d\n",jid,pid,WSTOPSIG(status));
              job->state = ST;
           }
           else if(WIFEXITED(status)) {
                   if(deletejob(jobs,pid))
                      if(verbose) {
                         printf("sigchld_handler: Job [%d] (%d) deleted\n",jid,pid);
                         printf("sigchld_handler: Job [%d] (%d) terminates OK (status %d)\n",jid,pid,WEXITSTATUS(status));
                      }
           }
           else {
               if(deletejob(jobs,pid)) {
                  if(verbose)
                     printf("sigchld_handler: Job [%d] (%d) deleted\n",jid,pid);
               }
               printf("Job [%d] (%d) terminated by signal %d\n",jid,pid,WTERMSIG(status));
           }
    }

    if(verbose) 
       printf("sigchld_handler: exiting");
    return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void sigint_handler(int sig) 
{
    if(verbose) {
       puts("sigint_handler: entering");
    }
    pid_t pid = fgpid(jobs);//返回在前台运行进程的pid

    if(pid <= 0) {
       if(verbose) {
          puts("sigint_handler: exiting");
       }
       return;
    }
    if(kill(-pid,SIGINT) < 0) {
       unix_error("kill (sigint) error");
    }
    if(verbose) {
       printf("sigint_handler: Job (%d) killed\n",pid);
       if(verbose) {
          puts("sigint_handler: exiting");
       }
       return;
    }
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void sigtstp_handler(int sig) 
{
    if(verbose) {
       puts("sigtstp_handler: entering");
    }
    pid_t pid = fgpid(jobs);
    if(pid <= 0) {
       if(verbose) {
          puts("sigtstp_handler: exiting");
       }
       return;
    }
    if(kill(-pid,SIGTSTP) < 0) {
       unix_error("kill (tstp) error");
    }
    if(verbose) {
       printf("sigtstp_handler: Job [%d] (%d) stopped\n",pid2jid(pid),pid);
       if(verbose) {
          puts("sigtstp_handler: exiting");
       }
       return;
    }
    return;
}

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
    job->pid = 0;
    job->jid = 0;
    job->state = UNDEF;
    job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) 
{
    int i, max=0;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid > max)
	    max = jobs[i].jid;
    return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) 
{
    int i;
    
    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == 0) {
	    jobs[i].pid = pid;
	    jobs[i].state = state;
	    jobs[i].jid = nextjid++;
	    if (nextjid > MAXJOBS)
		nextjid = 1;
	    strcpy(jobs[i].cmdline, cmdline);
  	    if(verbose){
	        printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
            }
            return 1;
	}
    }
    printf("Tried to create too many jobs\n");
    return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;

    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid == pid) {
	    clearjob(&jobs[i]);
	    nextjid = maxjid(jobs)+1;
	    return 1;
	}
    }
    return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
    int i;

    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].state == FG)
	    return jobs[i].pid;
    return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
    int i;

    if (pid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid)
	    return &jobs[i];
    return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) 
{
    int i;

    if (jid < 1)
	return NULL;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].jid == jid)
	    return &jobs[i];
    return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) 
{
    int i;

    if (pid < 1)
	return 0;
    for (i = 0; i < MAXJOBS; i++)
	if (jobs[i].pid == pid) {
            return jobs[i].jid;
        }
    return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) 
{
    int i;
    
    for (i = 0; i < MAXJOBS; i++) {
	if (jobs[i].pid != 0) {
	    printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
	    switch (jobs[i].state) {
		case BG: 
		    printf("Running ");
		    break;
		case FG: 
		    printf("Foreground ");
		    break;
		case ST: 
		    printf("Stopped ");
		    break;
	    default:
		    printf("listjobs: Internal error: job[%d].state=%d ", 
			   i, jobs[i].state);
	    }
	    printf("%s", jobs[i].cmdline);
	}
    }
}
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) 
{
    printf("Usage: shell [-hvp]\n");
    printf("   -h   print this message\n");
    printf("   -v   print additional diagnostic information\n");
    printf("   -p   do not emit a command prompt\n");
    exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg)
{
    fprintf(stdout, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
	unix_error("Signal error");
    return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) 
{
    printf("Terminating after receipt of SIGQUIT signal\n");
    exit(1);
}



