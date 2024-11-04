#include <stdio.h>
#include <stdlib.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define N 12

extern char **environ;

char *allowed[N] = {"cp","touch","mkdir","ls","pwd","cat","grep","chmod","diff","cd","exit","help"};

// deallocates argv elements
void freeMemory(char** argv) {
	for (int i = 0; argv[i] != NULL; i++) {
		if (i > 22) break;
		if (argv[i] == NULL) continue;
		free(argv[i]);
		argv[i] = NULL;
	}
}

int toSpawn(char* cmd) {
	for (int i = 0; i < 9; i++) {
		if (strcmp(cmd, allowed[i]) == 0) {
			return 1;
		}
	}

	return 0;
}

void help() {
	printf("The allowed commands are:\n");
	for (int i = 0; i < N; i++) {
		printf("%d: %s\n", i + 1, allowed[i]);
	}
}


int main() {
	char line[256];
	char **argv = (char**)malloc(22 * sizeof(char*));

	while (1) {
		fprintf(stderr,"rsh>");

		// skip empty lines
		if (fgets(line, 256, stdin)==NULL) continue;
		if (strcmp(line, "\n")==0) continue;

		unsigned int len = strlen(line);
		line[len - 1] = '\0';

		// parse line into cmd and arguents
		argv[0] = (char*)malloc(len * sizeof(char));
		int currentArg = 0; // argc = currentArg + 1
		int currentPos = 0;
		for (int i = 0; i < len; i++) {
			if (line[i] == '\0') break;
			if (line[i] == '\n') continue;
			if (line[i] == ' ') {
				argv[currentArg][currentPos] = '\0';
				currentArg++;
				currentPos = 0;
				argv[currentArg] = (char*)malloc((len - i) * sizeof(char));
				continue;
			}

			argv[currentArg][currentPos] = line[i];
			currentPos++;
		}
		argv[currentArg][currentPos] = '\0';
		argv[currentArg + 1] = NULL;

		// print argv debugging
		/* printf("cmd: %s\n", argv[0]);
		for (int i = 1; i < currentArg + 2; i++) {
			printf("argv[%d]: %s\n", i, argv[i]);
		}
		printf("\n"); */

		// 1-9
		if (toSpawn(argv[0])) {
			pid_t pid;
			int status;
			posix_spawnattr_t attr;
			posix_spawnattr_init(&attr);

			// get command path
			char path[16] = "/usr/bin/";
			int i;
			for (i = 0; argv[0][i] != '\0'; i++) {
				path[i + 9] = argv[0][i];
			}
			path[i + 9] = '\0';

			if (posix_spawnp(&pid, path, NULL, &attr, argv, environ) != 0) {
				perror("spawn failed\n");
			}

			if (waitpid(pid, &status, 0) == -1) {
				perror("waitpid failed\n");
			}

			if (WIFEXITED(status)) {
				// fprintf(stderr, "Process exited with status %d\n", WEXITSTATUS(status));
			}

			freeMemory(argv);
			continue;
		}

		// cd
		if (strcmp(argv[0], "cd") == 0) {
			if (currentArg > 1) {
				perror("-rsh: cd: too many arguments\n");
			} else if (currentArg == 1) {
				chdir(argv[1]);
			}

			freeMemory(argv);
			continue;
		}

		// exit
		if (strcmp(argv[0], "exit") == 0) {
			freeMemory(argv);
			break;
		}

		// help
		if (strcmp(argv[0], "help") == 0) {
			help();
			freeMemory(argv);
			continue;
		}

		printf("NOT ALLOWED!\n");
		freeMemory(argv);
	}

	free(argv);
	return 0;
}
