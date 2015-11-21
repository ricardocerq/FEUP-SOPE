#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <bits/sigaction.h>
#include <signal.h>
#include <pthread.h>

#include "sh_mem.h"



int servicing = 0;

//int sigaction(int signum, const struct sigaction *act,
 //                    struct sigaction *oldact);

void close_program(int return){ //TODO change
	exit(return);
}

void sigalrm_handler(int signo){
	close_program(0);
}

int accessShMem(char* nome_mempartilhada){// TODO change
	return 1;
}


int createFifo(){
	pid_t pid = getpid();
	char fifoName[MAX_FIFO_NAME_SIZE];
	sprintf(fifoName, "tmp/fb_%u", pid);
	int fifo;
	if((fifo = open(fifoName, O_RDWR | O_CREAT| O_EXCL)) <= 0){
		perror(fifoName);
		return 1;
	}
	return fifo;
}

void* serviceClient(void* clientFifo){
	return NULL;//TODO change
}


int main(int argc, char** argv) {
	// Check valid argument usage
	if(argc != 3){
		// Invalid usage
		printf("Usage: %s <nome_mempartilhada> <tempo_abertura>\n", argv[0]);
		exit(1);
	}

	// Parse opening time
	long tempo_abertura = strtol(argv[2], NULL, 10);
	if(tempo_abertura == LONG_MIN || tempo_abertura == LONG_MAX) {
		perror("tempo_abertura");
		exit(2);
	}
	struct sigaction action1;
	sigaction(SIGALRM,NULL, &action1);
	action1.sa_handler = sigalrm_handler;
	if (sigaction(SIGALRM,&action1,NULL) < 0)
	{
		fprintf(stderr,"Unable to install SIGALRM handler\n");
		exit(1);
	}

	if(accessShMem(argv[1])){
		fprintf(stderr,"Unable to access Shared Memory\n");
		exit(1);
	}
	
	if((int serverFifo = createFifo()) <= 0){
		exit(1);
	}
	alarm(tempo_abertura);
	char line[MAX_FIFO_NAME_SIZE];
	while(1){
		int num = read(serverFifo, line, MAX_FIFO_NAME_SIZE);
		pthread_t th;
		if(num != 0){
			line[num] = '\0';
			char* clientFifo = malloc((strlen(line)+1)*sizeof(char));
			strcpy(clientFifo, line);
			pthread_create(&th, NULL, serviceClient, (void *) clientFifo);
		}
	}

	return 0;
}