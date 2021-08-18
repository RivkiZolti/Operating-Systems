// name: rivki zolti
// id: 322667080


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#define _GNU_SOURCE
#include <errno.h>


int a;
int status;
int b;
char** arglist = NULL;
void killl();
void zomb();
int p;

void killl(){
	kill(getpid(),SIGINT);
	
}
int prepare(void){

	signal(SIGINT,SIG_IGN);
	signal(SIGCHLD,SIG_IGN);
	return 0;
}





int finalize(void){
	return 0;
}


void zomb()
{
	 while (waitpid((pid_t)(-1), 0, WNOHANG) > 0) {}
}

 

int process_arglist(int count, char** arglist){
	for(int i=0;i<count;i++){
		if (strcmp(arglist[i],"|")==0){
			a=1;
			b=i;
			break;
		}
	}
	if(a==1){
		arglist[b]=NULL;
		int pipe_fd[2]; 
		pid_t pid1,pid2;
		 if(pipe(pipe_fd)<0){
		 	fprintf(stderr,"Erorr: %s\n",strerror(errno));
 			exit(1);
		 }
       
        
		pid1=fork();
		if(pid1<0)
 		{
 			fprintf(stderr,"Erorr: %s\n",strerror(errno));
 			exit(1);
 		}
		if (pid1==0){
				signal(SIGINT,&killl);
				close(1);
				dup2(pipe_fd[1],1);
				
				if(execvp(arglist[0],arglist)<0){
					fprintf(stderr,"Erorr: %s\n",strerror(errno));
 					exit(1);
				};
			
		}else
		{		
				if(close(pipe_fd[1])<0){
					fprintf(stderr,"Erorr: %s\n",strerror(errno));
					return 0;
				}
				pid2=fork();
				if(pid1<0){
			
 				fprintf(stderr,"Erorr: %s\n",strerror(errno));
 				exit(1);
				}
				if (pid2==0){
						signal(SIGINT,&killl);
						close(0);
						
						dup2(pipe_fd[0],0);
						
						if(execvp(arglist[b+1],&arglist[b+1])<0){
							fprintf(stderr,"Erorr: %s\n",strerror(errno));
 							exit(1);
				}
		} 
		wait(&status);wait(&status);
		return 1;
}
	}
	
	if (a!=1){
	 	if (strcmp(arglist[count-1],"&")==0){
	 		
	 	p=fork();
		if(p==0){
			arglist[count-1]=NULL;
			execvp(arglist[0],arglist);	
		}
		signal(SIGCHLD,&zomb);
	 		
	 		
		}
		else{
		p=fork();
		if(p==0){
			signal(SIGINT,&killl);
			execvp(arglist[0],arglist);
		}
		 wait(&status);
		}
	}

	return 1;
}
