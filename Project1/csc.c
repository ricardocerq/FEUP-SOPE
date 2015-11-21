#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h> 
#include <sys/stat.h>
#include <strings.h>

#include "str_concat.h"

#define MAX_LINE 1024

// Pipe indexes
#define READ 0
#define WRITE 1

void proccess_line(char* line, int file, char* words) {
	static char* current_word = NULL; // Keeps the current word being processed across function calls
	static int length = 0;
	char* colon_start = strchr(line, ':');
	int newlength = colon_start - line;

	if(current_word == NULL || (length != newlength || strncasecmp(current_word, line, length) != 0)) {
		if(current_word != NULL) {
			write(file, "\n\n", 2);
			free(current_word);
		}
		current_word = (char*) calloc(newlength+1, sizeof(char));
		strncpy(current_word, line, newlength);

		int fd[2];
		if(pipe(fd)<0){
			perror("pipe");
			exit(1);
		}
		pid_t pid = 0;
		if((pid = fork()) < 0){
			perror("fork failed");
			exit(1);
		}
		if(pid == 0) {
			// Child
			close(fd[READ]);
			dup2(fd[WRITE], STDOUT_FILENO);
			execlp("grep", "grep", "-iwo", current_word, words, NULL);
			perror("exec grep failed");
			exit(1);
		}
		close(fd[WRITE]);
		read(fd[READ], current_word, newlength);
		close(fd[READ]);
		//current_word = (char*) calloc(colon_start - line + 1, sizeof(char));
		//strncpy(current_word, line, colon_start - line);
		//strcpy(current_word, entry);

		length = newlength;
		//write(file, line, strlen(line));
		write(file, current_word, newlength);
		write(file, colon_start, strlen(colon_start));
	} else {
		write(file, ",", 1);
		write(file, colon_start + 1, strlen(colon_start + 1));
	}
}

// Merges the multiple occurences of the same word into one line and writes it to the index.txt file
// Uses the auxiliary function above
void print_index(char* input, char* leftover, int file,char* words) {
	// Find the start of the last line
	char* lastline = input + strlen(input);
	for(; *lastline != '\n'; lastline--)
		;
	lastline++;

	char* current_temp = strtok(input, "\n");
	char* current = (char*) calloc(strlen(current_temp) + strlen(leftover) + 1, sizeof(char));
	strcpy(current, leftover);
	strcat(current, current_temp);
	char* to_free = current;

	while(current != NULL) {
		if(lastline != current)
			proccess_line(current, file, words);
		current = strtok(NULL, "\n");
	}

	strcpy(leftover, lastline);
	free(to_free);
}

int main(int argc, char *argv[]) {
	// Check valid argument usage
	if(argc != 2){
		// Invalid usage
		printf("Usage: %s <folder path>\n", argv[0]);
		return 1;
	}

	// Open directory specified in argv[1]
	DIR *dirp;
	struct dirent *direntp;
	struct stat stat_entry;

	if ((dirp = opendir(argv[1])) == NULL) {
		perror(argv[1]);
		return 1;
	}

	// Add / to end of folder path if it hasn't
	char* fldr;

	if(argv[1][strlen(argv[1])-1] != '/') {
		fldr = str_concat(argv[1], "/");
	} else {
		fldr = (char*) malloc(strlen(argv[1]) * sizeof(char));
		strcpy(fldr, argv[1]);
	}

	// Open index.txt, create it if doesn't exist
	char* index_path = str_concat(fldr, "index.txt");
	int index_file = open(index_path, O_CREAT | O_WRONLY | O_TRUNC , 0777);
	if(index_file < 0){
		perror("index.txt");
		return 1;
	}
	free(index_path);

	// Open 2 pipes, one from cat to sort, other from sort to merge
	int cat_to_sort[2];
	int sort_to_merge[2];
	if(pipe(cat_to_sort) < 0){
		perror("pipe failed");
		exit(1);
	}
	if(pipe(sort_to_merge) < 0){
		perror("pipe failed");
		exit(1);
	}

	pid_t pid = 0;

	// Create child to sort
	if((pid = fork()) < 0){
		perror("fork failed");
		exit(1);
	}
	
	if(pid == 0) {
		// Child
		close(cat_to_sort[WRITE]);
		close(sort_to_merge[READ]);
		// Direct cat output to sort input
		dup2(cat_to_sort[READ], STDIN_FILENO);
		// Direct sort output to pipe
		dup2(sort_to_merge[WRITE], STDOUT_FILENO);
		execlp("sort", "sort", "-fV", NULL);
		perror("exec sort failed");
		return 1;
	}

	// Process files with "_output.txt" in the end
	while ((direntp = readdir( dirp)) != NULL) {
		// Read directory item info
		char* file_path = str_concat(fldr, direntp->d_name);
		if (stat(file_path, &stat_entry) == -1) {
			perror("stat failed");
			exit(1);
		}
		free(file_path);

		// Check if it's a regular file
		if (S_ISREG(stat_entry.st_mode)) {
			// Make sure the file has "_output.txt" in the name
			if(strstr(direntp->d_name, "_output.txt") != NULL && direntp->d_name[strlen(direntp->d_name)-1] != '~'){
				// Spawn child process that runs cat
				if((pid = fork()) < 0) {
					perror("fork failed");
					exit(1);
				}
				if(pid == 0){
					// Child
					close(cat_to_sort[READ]);
					close(sort_to_merge[WRITE]);
					close(sort_to_merge[READ]);

					// Direct cat output to pipe
					dup2(cat_to_sort[WRITE], STDOUT_FILENO);
					char* file_path = str_concat(fldr, direntp->d_name);
					execlp("cat", "cat" , file_path, NULL);
					perror("exec cat failed");
					return 1;
				}
 			}
 		}
	}
	closedir(dirp);

	close(cat_to_sort[READ]);
	close(cat_to_sort[WRITE]);
	close(sort_to_merge[WRITE]);

	// Read sort output and print to index file
	char leftover[256];
	leftover[0] = '\0';
	write(index_file, "INDEX\n\n", 7);
	int n;
	char line[MAX_LINE];
	char* words = str_concat(argv[1], "/words.txt");
	while((n = read(sort_to_merge[READ], line, MAX_LINE - 1)) != 0) {
		line[n] = '\0';
		print_index(line, leftover, index_file, words);
	}
	write(index_file, "\n", 1);
	close(index_file);
	free (words);
	return 0;
}