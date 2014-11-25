/* ------------------------------
   $Id: pipe.c,v 1.2 2005/03/29 09:46:52 marquet Exp $
   ------------------------------------------------------------

   mshell - a job manager
   
*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "pipe.h"
#include "jobs.h"
#include "cmd.h"
#include "common.h"


void do_pipe(char *cmds[MAXCMDS][MAXARGS], int nbcmd, int bg) {
	/*
	while(i<MAXCMDS && cmds[i][0]!=NULL ){
		j=0;
		while(j<MAXARGS && cmds[i][j]!=NULL){
			printf("%s cmd : %d arg : %d\n", cmds[i][j], i, j);
			 j++;
		}
		i++;
	}*/
	int fd[2];
	int pid;
	char cmdline[80];
	int size_cat;
	int i, j;
	i=0;
	size_cat = 0;
	while(i<MAXCMDS && cmds[i][0]!=NULL ){
		j=0;
		if(i!=0){
			size_cat += snprintf(cmdline+size_cat, 80-size_cat, "| ");
		}
		while(j<MAXARGS && cmds[i][j]!=NULL){
			size_cat += snprintf(cmdline+size_cat, 80-size_cat, "%s ", cmds[i][j]);
			j++;
		}
		i++;
	}
	if(verbose){
		printf("%s \n",cmdline);
	}
	assert(pipe(fd)!=-1);
	/*printf("Avant premier fork\n");*/
	switch (pid = fork()) {
		case -1: assert(0);
		case 0: setpgid(0, 0);
			dup2(fd[0], STDIN_FILENO);
			close(fd[0]); close(fd[1]);
			execvp(cmds[1][0], cmds[1]);
			assert(0);
	}
	close(fd[0]);
	/*printf("Avant deuxième fork\n");*/
	switch (fork()) {
		case -1: assert(0);
		case 0: setpgid(0, pid);
			dup2(fd[1], STDOUT_FILENO);
			close(fd[1]);		
			execvp(cmds[0][0], cmds[0]);
			assert(0);	
	}
	close(fd[1]);
	if(bg){
		jobs_addjob(pid, BG, cmdline);
  	}else{
		jobs_addjob(pid, FG, cmdline);
		waitfg(pid);
  	}
	return;
}
