#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "logging.h"
#include "str_concat.h"

#define BUF_SIZE 128

FILE* fptr = NULL;

char* cur_date_time_str() {
	// Get current time
	time_t t = time(0);
	struct tm *t_now = localtime(&t);

	// Get the date and time values
	int year = t_now->tm_year + 1900;
	int month = t_now->tm_mon + 1;
	int day = t_now->tm_mday;
	int hour = t_now->tm_hour;
	int minute = t_now->tm_min;
	int second = t_now->tm_sec;

	// Create string
	char* buf = (char*)malloc(sizeof(char) * 20);
	sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);

	return buf;
}

int print_cols(char* col1, char* col2, char* col3, char* col4, char* col5) {
	if(fptr != NULL) {
		// Create line string
		char buf[BUF_SIZE];
		sprintf(buf, "%-19s | %-6s | %-7s | %-20s | %-17s \n", col1, col2, col3, col4, col5);

		// Write it to file
		fwrite(buf, 1, strlen(buf), fptr);
		return 0;
	}

	return 1;
}

int log_open(char* filename) {
	char* name = str_concat(filename, ".log");
	fptr = fopen(name, "a");
	free(name);

	if(fptr == NULL)
		return 1;

	long int fpos = ftell(fptr);

	if(fpos == 0) {
		// Log didn't exist, print header
		print_cols("quando", "quem", "balcao", "o_que", "canal_criado/usado");

		// Print header separator
		unsigned int i;
		for(i = 0; i < 82; i++)
			fwrite("-", 1, 1, fptr);
		fwrite("\n", 1, 1, fptr);
	}

	return 0;
}

int log_close() {
	if(fptr != NULL) {
		int res =  fclose(fptr);
		fptr = NULL;
		return res;
	}

	return 1;
}

int log_write(char* quem, char* balcao, char* o_que, char* canal) {
	char* date_str = cur_date_time_str();
	int res = print_cols(date_str, quem, balcao, o_que, canal);
	free(date_str);
	return res;
}
