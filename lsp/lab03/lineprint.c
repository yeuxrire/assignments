#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINES 1000
#define MAX_LEN 1024

char *lines[MAX_LINES];
int line_count = 0;
char original_filename[MAX_LEN];

void load_file(const char *filename) {
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		perror("File Opening is Failed.");
		exit(1);
	}
	
	char buffer[MAX_LEN];
	while(fgets(buffer, MAX_LEN, fp) && line_count < MAX_LINES) {
		char *ptr = strchr(buffer, '\n');
		if (ptr) *ptr = '\0';
		lines[line_count] = strdup(buffer);
		line_count++;
	}
	fclose(fp);
}

void print_line(int n) {
	if (n >= 1 && n <= line_count) {
		printf("[%d] %s\n", n, lines[n-1]);
	}
}

void delete_line(int n) {
	if (n >= 1 && n <= line_count) {
		free(lines[n-1]);
		for (int i = n-1; 0 < line_count-1; i++) {
			lines[i] = lines[i+1];
		}
		line_count--;
	}
}

void append_line(int n) {
	char new_text[MAX_LEN];
	if (fgets(new_text, MAX_LEN, stdin) == NULL) return;
	new_text[strcspn(new_text, "\n")] ='\0';

	for (int i = line_count; i > n; i--) {
		lines[i] = lines[i-1];
	}

	lines[n] = strdup(new_text);
	line_count++;
}

void substitute_line(int n) {
	char new_text[MAX_LEN];
	if (fgets(new_text, MAX_LEN, stdin) == NULL) return;
	new_text[strcspn(new_text, "\n")] = '\0';

	free(lines[n-1]);
	lines[n-1] = strdup(new_text);
}

void save_file(char *target_filename) {
	char *filename = (target_filename != NULL && strlen(target_filename) > 0) ? target_filename : original_filename;

	FILE *fp = fopen(filename, "w");
	if (fp == NULL) {
		perror("File opening is failed.");
		return;
	}

	for (int i = 0; i < line_count; i++) {
		fprintf(fp, "%s\n", lines[i]);
	}
	fclose(fp);
}

void process_command(char cmd, char *target) {
	if (target == NULL || strlen(target) == 0) return;

	if (strchr(target, '*')) {
		if (cmd == 'p') {
			for (int i = 1; i <= line_count; i++) print_line(i);
		} else if (cmd == 'd') {
			for (int i = line_count; i >= 1; i--) delete_line(i);
		}
	} else if (strchr(target, '-')) {
		char *temp = strdup(target);
		int start = atoi(strtok(temp, "-"));
		int end = atoi(strtok(NULL, "-"));

		if (cmd == 'p') {
			for (int i = start; i <= end; i++) print_line(i);
		} else if (cmd == 'd') {
			for (int i = end; i >= start; i--) delete_line(i);
		}
		free(temp);
	} else if (strchr(target, ',')) {
		char *temp = strdup(target);
		char *ptr = strtok(temp, ",");
		int list[MAX_LINES];
		int count = 0;

		while(ptr != NULL) {
			list[count++] = atoi(ptr);
			ptr = strtok(NULL, ", ");
		}

		if (cmd == 'p') {
			for (int i = 0; i < count; i++) print_line(list[i]);
		} else if (cmd == 'd') {
			for (int i = 0; i < count-1; i++) {
				for (int j = i+1; j < count; j++) {
					if (list[i] < list[j]) {
						int tmp = list[i]; list[i] = list[j]; list[j] = tmp;
					}
				}
			}
			for (int i = 0; i < count; i++) delete_line(list[i]);
		}
		free(temp);
	} else {
		int n = atoi(target);
		if (n > 0 && n <= line_count) {
			if (cmd == 'p') print_line(n);
			else if (cmd == 'd') {
				delete_line(n);
			}
		}
	}
}

int main(int argc, char *argv[]){
	if (argc < 2) return 1;
	
	strcpy(original_filename, argv[1]);
	load_file(original_filename);
	
	char input[MAX_LEN];
	
	while(1) {
		printf(">> ");
		if (fgets(input, MAX_LEN, stdin) == NULL) break;
		input[strcspn(input, "\n")] ='\0';

		if (strlen(input) == 0) continue;
		if (strcmp(input, "q") == 0) break;

		char cmd = input[0];
		char *target = input + 1;

		while(*target == ' ') target++;

		if (cmd == 'p' || cmd == 'd') {
			process_command(cmd, target);
		} else if (cmd == 'a') {
			append_line(atoi(target));
		} else if (cmd == 's') {
			substitute_line(atoi(target));
		} else if (cmd == 'w') {
			save_file(target);
		} else if ((cmd >= '0' && cmd <= '9') || cmd == '*') {
			process_command('p', input);
		} else {
			printf("Try other commands.\n");
		}
	}
	
	for (int i = 1; i < line_count; i++) free(lines[i]);
	return 0;
}
