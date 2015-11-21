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
#include <signal.h>
#include <bits/sigaction.h>

#include "logging.h"
#include "sh_mem.h"

#define MAX_LINE 256

int sigaction(int signum, const struct sigaction *act,
                     struct sigaction *oldact);

int total_clientes = 0;

// SIGCHLD handler
void sigchld_handler(int sig){
	int ret;
	int wpid = wait(&ret);

	// wait() returned an error
	if(wpid == -1)
		perror("wait");
	else if(ret != 0) // Child return code is an error
		printf("Child (PID=%d) terminated with an error.\n", wpid);
	
	total_clientes--;
}

void logStart(char * fifoName, int numBalcao){// TODO change

}
void logFinish(char * fifoName, int numBalcao){// TODO change

}

int getServerInfo(char* nome_mempartilhada, int* numBalcao, int* fifofd){// TODO change
	*fifofd = open("/tmp/testeBalcao", O_WRONLY);
	*numBalcao = 1;
	return *fifofd; 
}


int create_client(char* nome_mempartilhada) {
	//Create the clients FIFO
	pid_t pid = getpid();
	char fifoName[MAX_FIFO_NAME_SIZE];
	
	sprintf(fifoName, "/tmp/fc_%u", pid);

	int clientFifo = open(fifoName, O_RDWR | O_CREAT | O_EXCL);
	if(clientFifo <= 0){
		perror(fifoName);
		return 1;
	}

	//get number and FIFO name of server
	int serverFifo, numBalcao;

	getServerInfo(nome_mempartilhada, &numBalcao, &serverFifo);

	logStart(fifoName, numBalcao);

	write(serverFifo, fifoName, strlen(fifoName));//give the name of the fifo to server
	char line[MAX_LINE];
	while(1){
		int numRead = 0;
		numRead = read(clientFifo, line, MAX_LINE);
		if(numRead > 0){
			if(strcmp(line, "fim_atendimento") == 0)
				break;
		}
	}
	logFinish(fifoName, numBalcao);

	unlink(fifoName);
	return 0;
}

int verifySharedMemory(char* nome_mempartilhada){//TODO change
	return 0;
}


int main(int argc, char** argv) {
	// Check valid argument usage
	if(argc != 3){
		// Invalid usage
		printf("Usage: %s <nome_mempartilhada> <num_clientes>\n", argv[0]);
		exit(1);
	}

	if(!verifySharedMemory(argv[1])){
		printf("Unable to access shared memory\n");
		exit(1);
	}

	// Parse number of clients
	long num_clientes = strtol(argv[2], NULL, 10);
	if(num_clientes == LONG_MIN || num_clientes == LONG_MAX) {
		perror("num_clientes");
		exit(2);
	}

	//Install SIGCHLD handler
	struct sigaction action1;
	sigaction(SIGCHLD,NULL, &action1);
	action1.sa_handler = sigchld_handler;
	if (sigaction(SIGCHLD,&action1,NULL) < 0)
	{
		fprintf(stderr,"Unable to install SIGCHLDhandler\n");
		exit(1);
	}

	// Create one child process per client
	for(long n = 0; n < num_clientes; n++) {
		pid_t pid = fork();

		if(pid < 0) {
			// Fork failed
			perror("fork");
		} else if(pid > 0) {
			// Parent
			// Increast number of clients
			total_clientes++;
		} else if(pid == 0) {
			// Child
			// Create the new client
			return create_client(argv[1]);
		}
	}

	// Wait for all clients child processes to finish
	// acho que nao e preciso
	while(total_clientes > 0) {
		int ret;
		int wpid = wait(&ret);
	
		// wait() returned an error
		if(wpid == -1)
			perror("wait");
		else if(ret != 0) // Child return code is an error
			printf("Child (PID=%d) terminated with an error.\n", wpid);
		
		total_clientes--;
	}

	return 0;
}