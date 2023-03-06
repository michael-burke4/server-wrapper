#include "discbot.h"
#include "mcserver.h"
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define BUFFSIZE 2048

void set_read_nonblocking(int fd[]);

int main(void)
{
	struct timespec read_wait = { 0, 1000L };
	pid_t ppid_before_fork, pid;
	int server_input[2], server_output[2];
	int discord_input[2], discord_output[2];
	char buffer[BUFFSIZE + 1], discord_buffer[BUFFSIZE + 1];
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
	set_read_nonblocking(server_output);
	pid = fork();
	if (pid == -1)
		err(1, "bad fork!");
	if (!pid) { // in child process...
		run_server_process(server_input, server_output, ppid_before_fork);
	}
	close(server_input[READ_END]);
	close(server_output[WRITE_END]);

	ppid_before_fork = getpid();
	if (pipe(discord_input) == -1)
		err(1, "bad pipe!");
	if (pipe(discord_output) == -1)
		err(1, "bad pipe!");
	set_read_nonblocking(discord_output);
	pid = fork();
	if (pid == -1)
		err(1, "bad fork!");
	if (!pid) { // in child process...
		run_discord_process(discord_input, discord_output, ppid_before_fork);
	}
	close(discord_input[READ_END]);
	close(discord_output[WRITE_END]);

	while (1) {
		// FIXME: using this (old) implementation prints the occasional garbage char.
		// printf("%s", buffer);
		nanosleep(&read_wait, NULL);

		num_read = read(discord_output[READ_END], discord_buffer, BUFFSIZE);
		if (num_read > 0) {
			if (strncmp(MC_TAG, discord_buffer, strlen(MC_TAG)) == 0) {
				// size_t cast is ok because we checked if num_read > 0
				write(server_input[WRITE_END], &(discord_buffer[4]), (size_t)num_read - 4);
			} else {
				for (int i = 0; i < num_read; ++i) {
					// band-aid fix to above FIXME
					printf("%c", discord_buffer[i]);
					// printf("i: %d buffer[i]: %c %d\n", i, buffer[i], buffer[i]); // debug stuff ignore
					discord_buffer[i] = '\0';
				}
			}
		}

		num_read = read(server_output[READ_END], buffer, BUFFSIZE);
		// printf("ERRNO: %d\n", errno);
		if (num_read <= 0)
			continue;

		if (0 == parse_server_output(buffer, &msg)) {
			printf("%s said: %s", msg.user, msg.message);
		}
		for (int i = 0; i < num_read; ++i) {
			// band-aid fix to above FIXME
			printf("%c", buffer[i]);
			// printf("i: %d buffer[i]: %c %d\n", i, buffer[i], buffer[i]); // debug stuff ignore
			buffer[i] = '\0';
		}
	}
}

void set_read_nonblocking(int fd[])
{
	fcntl(fd[READ_END], F_SETFL, O_NONBLOCK);
	// fcntl(fd[READ_END], F_SETFL, O_NONBLOCK);
}
