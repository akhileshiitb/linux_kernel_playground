#include <stdio.h>
#include <unistd.h>

int main()
{
	pid_t pid;

	printf("Welcome to Kekhale OS\n");

	printf("Launching program 1\n");

	pid = fork();

	if (pid == 0) {
		printf("This is child process\n");	
		execl("/program1", "irrelevant", NULL);
	}


	while(1) {
		printf("init process alive \n");
		sleep(3);
	}

	return 0;
}
