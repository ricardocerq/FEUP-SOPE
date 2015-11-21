//#define _POSIX_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <bits/sigaction.h>
#include "str_concat.h"

#define MAX_LINE 1024
int sigaction(int signum, const struct sigaction *act,
                     struct sigaction *oldact);
// Pipe indexes
#define READ 0
#define WRITE 1

int txt_ext_size = 0; // Number of characters from end until the file extenion'

// SIGCHLD handler
void receive_chld(int sig){
	int ret;
	wait(&ret);
}

// Processes a line and writes it to the file pointed by the file descriptor
// Input format: line_number:word
// Output to file format: word: file_name-line_number
int proccess_line_aux(char* input, char* filename, int file){
	char* colon_pos = strchr(input, ':');
	if(colon_pos == NULL)
		return 1;

	char output[MAX_LINE];

	strcpy(output, colon_pos + 1);
	strcat(output, ": ");
	strncat(output, filename, strlen(filename) - txt_ext_size);
	strcat(output, "-");
	strncat(output, input, colon_pos - input);
	int lastpos = strlen(input) + 2 + strlen(filename) - txt_ext_size + 1;
	output[lastpos+1] = '\0';
	write(file, output, strlen(output));
	write(file, "\n", 1);

	return 0;
}

// Processes the string in line according to the work specifications and writes it 
// to the file pointed by the file descriptor.
// filename corresponds to the file from which the word came from
// Uses the auxiliaty function above
int proccess_line(char* line, char* filename, int file){
	char* to_proccess = strtok(line, "\n");
	while(to_proccess != NULL) {
		if(proccess_line_aux(to_proccess, filename, file) != 0)
			return 1;
		to_proccess= strtok(NULL, "\n");
	}

	return 0;
}

int main(int argc, char *argv[]) {
	// Check valid argument usage
	if(argc != 2){
		// Invalid usage
		printf("Usage: %s <folder path>\n", argv[0]);
		exit(1);
	}

	// Check if file to process exists (argv[1])
	int file = open(argv[1], O_RDONLY, 0600);
	if(file <= 0) {
		perror(argv[1]);
		exit(1);
	}
	close(file);

	// Install SIGCHLD handler
	struct sigaction action;
	if(sigaction(SIGCHLD, NULL, &action) == -1){
		perror("Could not retrieve previous SIGCHLD action");
		exit(1);
	}
	action.sa_handler = receive_chld;
	if(sigaction(SIGCHLD,  &action, NULL) == -1){
		perror("Could not set new SIGCHLD action");
		exit(1);
	}

	// Create pipe
	int fd[2];
	pid_t pid;
	if(pipe(fd) < 0) {
		perror("pipe failed");
		exit(1);
	}
	if((pid = fork()) < 0) {
		perror("fork failed");
		exit(1);
	}

	if(pid != 0) {
		// Parent
		close(fd[WRITE]);

		// Find file extension size
		size_t i = strlen(argv[1]);
		for( ; argv[1][i] != '.' && i > 0; i--)
			;
		txt_ext_size = strlen(argv[1]) - i;

		// Generate output filename
		char* output_filename = (char*) malloc(strlen(argv[1]) + strlen("_output.txt") + 1 - txt_ext_size);
		strncpy(output_filename, argv[1], strlen(argv[1]) - txt_ext_size);
		output_filename[strlen(argv[1]) - txt_ext_size] = 0;
		strcat(output_filename, "_output.txt");

		// Open output file
		int file = open(output_filename, O_CREAT | O_WRONLY | O_TRUNC, 0777);
		if(file <= 0){
			perror(output_filename);
			exit(1);
		}

		// Reat data from child process and process it
		char line[MAX_LINE];
		int nr =  0;

		while((nr = read(fd[READ], line, MAX_LINE)) != 0){
			line[nr] = '\0';
			if(proccess_line(line, argv[1], file))
				printf("Error proccessing line.\n");
		}

		close(fd[READ]);
		close(file);
		free(output_filename);
	} else {
		// Child
		close(fd[READ]);

		// Redirect grep output to pipe
		dup2(fd[WRITE], STDOUT_FILENO); 
		sleep(10);
		//grep --line-buffered -iwonf words.txt argv[1]
		execlp("grep", "grep","--line-buffered", "-iwonf", "words.txt", argv[1], NULL);
		perror("exec grep failed");

		exit(1);
	}
	
	exit(0);
}