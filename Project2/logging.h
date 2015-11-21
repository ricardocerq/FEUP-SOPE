#ifndef LOGGING_H_
#define LOGGING_H_

// Get a string with the current date and time
// Format: YYYY-MM-DD HH:MM:SS
char* cur_date_time_str();

// Prints the formatted columns to the log file
// Returns 0 on success
int print_cols(char* col1, char* col2, char* col3, char* col4, char* col5);

// Opens the log file
// If the file didn't exist yet, adds the table header (column titles + separator)
// Adds the .log extension automatically
// Returns 0 on success
int log_open(char* filename);

// Closes the log file
// Returns 0 on success
int log_close();

// Returns 0 on success
int log_write(char* quem, char* balcao, char* o_que, char* canal);

#endif