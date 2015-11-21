#include <string.h>
#include <stdlib.h>

#include "str_concat.h"

// Returns a new string with the 2 strings concatenated
char* str_concat(char* str1, char* str2) {
	int i;

	int length = strlen(str1) + strlen(str2);

	char* str = (char*) malloc(length + 1);

	if(str == NULL)
		return NULL;

	for (i = 0; str1[i] != '\0'; i++)
		str[i] = str1[i];

	int j;
	for (j = 0; str2[j] != '\0'; j++, i++)
		str[i] = str2[j];

	str[i] = '\0';

	return str;
}