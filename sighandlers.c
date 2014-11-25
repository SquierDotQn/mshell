/* ------------------------------
   $Id: sighandlers.c,v 1.1 2005/03/17 13:00:46 marquet Exp $
   ------------------------------------------------------------

   mshell - a job manager
   
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>

#include "jobs.h"
#include "common.h"
#include "sighandlers.h"


/*
 * Signal - wrapper for the sigaction function
 */
int
signal_wrapper(int signum, handler_t *handler){
    int sa_result;
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    assert((sa_result = sigaction(signum, &sa, NULL))!=-1);
    return sa_result;
}


/* 
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children
 */
void
sigchld_handler(int sig){
    int status;
    pid_t pid;

    if (verbose)
	printf("sigchld_handler: entering, recieved signal %d \n", sig);
    
    while((pid=waitpid(-1, &status, WNOHANG|WUNTRACED)) >0){
    	if(jobs_getjobpid(pid)!=NULL){
    		if(WIFEXITED(status)){
	    	  printf("[%d]+ : Exited %d\n", jobs_pid2jid(pid), jobs_getjobpid(pid)->jb_state);
	    	  jobs_deletejob(pid);
	    	}else if(WIFSIGNALED(status)){
	    	  printf("\n[%d]+ : Signaled %d\n", jobs_pid2jid(pid), jobs_getjobpid(pid)->jb_state);
	    	  jobs_deletejob(pid);
	    	}else if(WIFSTOPPED(status)){
		  printf("\n[%d]+ : Stopped %d\n", jobs_pid2jid(pid), jobs_getjobpid(pid)->jb_state);
		  jobs_getjobpid(pid)->jb_state = ST;
		}
    	}
    }
    
    if (verbose)
	printf("sigchld_handler: exiting\n");
    
    return;
}

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.  
 */
void
sigint_handler(int sig){
    pid_t pid;
    if (verbose)
	printf("sigint_handler: entering\n");
    
    
    pid = jobs_fgpid();
    if(pid!=0){
    	kill(pid, sig);
    	if(verbose) printf("\nkilled %d with SIGINT : %d \n", pid, sig);
    }
    
    if (verbose)
	printf("sigint_handler: exiting\n");
    
    return;
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.  
 */
void
sigtstp_handler(int sig){
    pid_t pid;
    if (verbose)
	printf("sigtstp_handler: entering\n");
    
    
    pid = jobs_fgpid();
    if(pid!=0){
    	kill(pid, sig);
    	if(verbose) printf("\nstopped %d with SIGTSTP : %d \n", pid, sig);
    }
    if (verbose)
	printf("sigtstp_handler: exiting\n");
    
    return;
}
