#include <stdio.h>

void printfile(char *filename);
void wc(char *filename);

int main(int argc, char *argv[]) {
	if (argc < 2) return 1;
	
	for (int i = 1; i < argc; i++) {
		printfile(argv[i]);
		wc(argv[i]);
		printf("\n");
	}
	printf("test\n");
	return 0;
}
