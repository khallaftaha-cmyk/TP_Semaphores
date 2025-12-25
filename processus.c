#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
	pid_t pid;
	pid = fork();

	if(pid < 0){
		perror("Fork failed");
		return 1;
	}
	else if(pid == 0){
		for(int i = 0; i<5; i++){
			printf("Je suis le fils mon pid est: %d\n",getpid());
			sleep(1);
		}
		exit(0);
	}
	else{
		for(int i = 0; i<5; i++){
			printf("Je suis le père mon pid est: %d\n",getpid());
			sleep(1);
		}
	wait(NULL);
	}
	
	return 0;
}
