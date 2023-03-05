#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#define READ_END 0
#define WRITE_END 1
#define BUFFSIZE 2048




int main(void)
{
	char **argv;
	pid_t ppid_before_fork;
	pid_t pid;
	int server_input[2], server_output[2];
	char *hello_msg = "say hello\n";
	char buffer[BUFFSIZE];
	ssize_t num_read;


	argv = malloc(5 * sizeof (char *));
	argv[0] = "java";
	argv[1] = "-jar";
	argv[2] = "server.jar";
	argv[3] = "nogui";
	argv[4] = NULL;


	ppid_before_fork = getpid();
	if (pipe(server_input) == -1)
		err(1, "bad pipe!");
	if (pipe(server_output) == -1)
		err(1, "bad pipe!");
	pid = fork();
	if (pid == -1)
		err(1, "bad fork!");
	// took a bunch of code form this SO page for sending term signal to child process on parent exit
	// https://stackoverflow.com/questions/284325/how-to-make-child-process-die-after-parent-exits/17589555#17589555
	if (!pid) { // in child process...
		int r;
		dup2(server_input[READ_END], STDIN_FILENO);
		dup2(server_output[WRITE_END], STDOUT_FILENO);
		
		//close(STDOUT_FILENO);
		r = prctl(PR_SET_PDEATHSIG, SIGTERM);
		if (r == -1)
			err(1, "parent process exited before prctl...");
		if (chdir("./mc_server"))
			err(1, "could not change directory! do you have a directory called 'mc_server'?");
		if (getppid() != ppid_before_fork)
			err(1, "child process was adopted. not continuing...");
		if (execvp(argv[0], argv))
		 	err(1, "bad exec");
	}
	free(argv);
	close(server_input[READ_END]);
	close(server_output[WRITE_END]);


	while (1) {
		while(num_read = read(server_output[READ_END], buffer, BUFFSIZE)) {
			printf("%s", buffer);
			if(errno) printf("error: %s\n", strerror(errno));
		}

	}
}
