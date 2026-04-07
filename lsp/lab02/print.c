#include <stdio.h>

void printfile(char *filename) {
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) return;

	char buffer[1024];
	int line_num = 1;
	
	printf("filename: %s\n", filename);
	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		printf("%d %s", line_num++, buffer);
	}
	fclose(fp);
	printf("changed\n");
}	
