#include "mcserver.h"
#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFSIZE 2048

int main(void)
{
	pid_t ppid_before_fork, pid;
	int server_input[2], server_output[2];
	char buffer[BUFFSIZE + 1];
	ssize_t num_read;
	struct user_msg msg = {
		.time = NULL,
		.user = NULL,
		.message = NULL,
		.time_space = 0,
		.user_space = 0,
		.message_space = 0
	};

	ppid_before_fork = getpid();
	if (pipe(server_input) == -1)
		err(1, "bad pipe!");
	if (pipe(server_output) == -1)
		err(1, "bad pipe!");
	pid = fork();
	if (pid == -1)
		err(1, "bad fork!");
	if (!pid) { // in child process...
		run_child_process(server_input, server_output, ppid_before_fork);
	}
	close(server_input[READ_END]);
	close(server_output[WRITE_END]);

	while ((num_read = read(server_output[READ_END], buffer, BUFFSIZE))) {
		// FIXME: using this (old) implementation prints the occasional garbage char.
		// printf("%s", buffer);

		if (0 == process(buffer, &msg)) {
			printf("%s said: %s", msg.user, msg.message);
		}
		for (int i = 0; i < num_read; ++i) {
			// band-aid fix to above FIXME
			printf("%c", buffer[i]);
			// printf("i: %d buffer[i]: %c %d\n", i, buffer[i], buffer[i]); // debug stuff ignore
			buffer[i] = '\0';
		}
		if (errno)
			printf("error: %s\n", strerror(errno));
	}
}
