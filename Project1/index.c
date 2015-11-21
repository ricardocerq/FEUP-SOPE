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

#include "str_concat.h"

#define MAX_LINE 1024

// realpath prototype
char* realpath(const char *path, char *resolved_path);

int main(int argc, char* argv[]) {
	// Compute full real program path from argv[0]
	char programpath[MAX_LINE];
	realpath(argv[0], programpath);
	size_t pathend;
	for(pathend = strlen(programpath); programpath[pathend]!= '/'; pathend--)
		;
	programpath[++pathend] = '\0';

	// Check valid argument usage
	if(argc != 2) {
		// Invalid usage
		printf("Usage: %s <folder path>\n", argv[0]);
		exit(1);
	}

	// Open directory specified in argv[1]
	DIR *dirp;
	struct dirent *direntp;
	struct stat stat_entry;

	if ((dirp = opendir(argv[1])) == NULL) {
		// Failed to open directory
		perror(argv[1]);
		exit(1);
	}

	// Add / to end of folder path if it hasn't
	char* fldr;

	if(argv[1][strlen(argv[1])-1] != '/') {
		fldr = str_concat(argv[1], "/");
	} else {
		fldr = (char*) malloc(strlen(argv[1]) * sizeof(char));
		strcpy(fldr, argv[1]);
	}

	// Check if words.txt exists
	char* words_path = str_concat(fldr, "words.txt");
	int words = open(words_path, O_RDONLY, 0600);
	if(words < 0){
		// words.txt file does not exist
		perror("words.txt");
		exit(1);
	}
	close(words);
	free(words_path);

	// Check if index.txt exists
	char* index_path = str_concat(fldr, "index.txt");
	int index_tmp = open(index_path, O_RDONLY, 0600);
	if(index_tmp >= 0) {
		// index.txt exists, delete it
		close(index_tmp);
		unlink(index_path);
	}
	free(index_path);

	// Spawn child processes for each file to process
	pid_t pid = 0;

	int children = 0;
	while ((direntp = readdir(dirp)) != NULL) {
		// Read directory item info
		char* file_path = str_concat(fldr, direntp->d_name);
		if (stat(file_path, &stat_entry) == -1) {
			perror("stat error");
			closedir(dirp);
			exit(1);
		}
		free(file_path);

		// Check if it's a regular file
		if (S_ISREG(stat_entry.st_mode)) {
			// Check if it's not words.txt or some other file not to process
			if(strcmp(direntp->d_name, "words.txt") != 0 && direntp->d_name[strlen(direntp->d_name) - 1] != '~'){
				if((pid = fork()) < 0){
					// Fork error
					perror("fork failed");
					exit(1);
				} else if(pid == 0){
					// Child process, use sw program on the file
					strcat(programpath, "sw");
					if(chdir(argv[1]) < 0) {
						perror(argv[1]);
						exit(1);
					}
					execlp(programpath, "sw" , direntp->d_name, NULL);
					perror("exec sw failed");
					exit(1);
				} else {
					// Parent, increase number of childs
					children++;
				}
			}
		}
	}

	// Wait for all childs to finish running sw
	while(children > 0) {
		int ret;
		int wpid = wait(&ret);

		// wait() returned an error
		if(wpid == -1)
			perror("wait");
		else if(ret != 0) // Child return code is an error
			printf("sw child (PID=%d) terminated with error code %d.\n", wpid, ret);
		children--;
	}

	// Create new child for csc
	if((pid = fork()) < 0) {
		perror("fork failed");
		exit(1);
	}

	if(pid == 0) {
		// Child process, use csc program to process the folder
		strcat(programpath, "csc");
		execlp(programpath, "csc" , argv[1], NULL);
		perror("exec csc failed");
		exit(1);
	} else {
		// Parent, wait for child process to finish
		int ret;
		int wpid = wait(&ret);

		// wait() returned an error
		if(wpid == -1)
			perror("wait");
		else if(ret != 0) // Child return code is an error
			printf("csc child (PID=%d) terminated with error code %d.\n", wpid, ret);
	}

	// Go back to the beginning of the directory
	rewinddir(dirp);

	// Clean files created by sw
	while ((direntp = readdir( dirp)) != NULL) {
		char* file_path = str_concat(fldr, direntp->d_name);
		if (stat(file_path, &stat_entry) == -1) {
			perror("stat error");
			closedir(dirp);
			exit(1);
		}

		if (S_ISREG(stat_entry.st_mode)) {
			// Files created by sw end in "_output.txt"
			if(strstr(direntp->d_name, "_output.txt") != NULL && direntp->d_name[strlen(direntp->d_name)-1] != '~'){
				unlink(file_path);
			}
		}

		free(file_path);
	}
	closedir(dirp);

	return 0;
}
