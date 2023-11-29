#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "chip.h"

char *strdup(const char *s) {
	size_t len = strlen(s) + 1;
	void *new = malloc(len);
	if (new == NULL) {
		return NULL;
	}
	return (char *) memcpy(new, s, len);
}

char *read_file(char *file) {
	FILE *infp = fopen(file, "rb");
	if(!infp) {
		printf("Cannot open %s\n", file);
		exit(0);
	}

	fseek(infp, 0, SEEK_END);
	long fsize = ftell(infp);
	char *p = malloc(fsize + 1);
	fseek(infp, 0, SEEK_SET);

	if(fread(p, 1, fsize, infp) != fsize) {
		printf("unable to load file\n");
		exit(1);
	}
	fclose(infp);
	*(p + fsize) = '\0';

	return p;
}