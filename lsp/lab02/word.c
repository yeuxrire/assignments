#include <stdio.h>
#include <ctype.h>

void wc(char *filename) {
	FILE *fp = fopen(filename, "r");	
	if (fp == NULL) return;
      
	int chars = 0, words = 0, lines = 0;
	int c, in_word = 0;
	
	while ((c = getc(fp)) != EOF ) {
		chars++;
		if (c == '\n') lines++;
		if (isspace(c)) {
			in_word = 0;
		} else if (in_word == 0) {
			in_word = 1;
			words++;
		}
	}
	
	printf("characters: %d, words: %d, lines: %d", chars, words, lines);
	
	fclose(fp);
}			
