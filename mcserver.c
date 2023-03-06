#include "mcserver.h"
#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <unistd.h>

int parse_server_output(char *buffer, struct user_msg *msg_struct)
{
	size_t i; // index of the username
	size_t j; // index of the end of the message statement
	size_t user_message_size;
	size_t server_msg_len;

	if (buffer == NULL || buffer[0] != '[') {
		return -1;
	}

	if (msg_struct->time_space == 0) {
		msg_struct->time = malloc(9 * sizeof(char));
		msg_struct->time_space = 9;
	}
	strncpy(msg_struct->time, &(buffer[1]), 8);
	msg_struct->time[8] = '\0';

	// checks if this is a valid user message
	if (buffer[SERVER_MSG_SIZE] != '<') {
		return -1;
	}

	for (i = SERVER_MSG_SIZE; buffer[i] != '>'; i++)
		;
	user_message_size = i - SERVER_MSG_SIZE - 1;
	if (user_message_size > msg_struct->user_space) {
		printf("reallocing user pointer...\n");
		msg_struct->user = realloc(msg_struct->user, (user_message_size + 1) * sizeof(char));
		msg_struct->user_space = user_message_size + 1;
	}
	strncpy(msg_struct->user, &(buffer[SERVER_MSG_SIZE + 1]), user_message_size);
	msg_struct->user[user_message_size] = '\0';

	i += 2; // moves index from end of username to beginning of message
	for (j = i; buffer[j] != '\0'; j++)
		;
	server_msg_len = j - i;
	if (server_msg_len > msg_struct->message_space) {
		printf("reallocing message pointer...\n");
		msg_struct->message = realloc(msg_struct->message, (server_msg_len + 1) * sizeof(char));
		msg_struct->message_space = (server_msg_len + 1);
	}
	strncpy(msg_struct->message, &(buffer[i]), server_msg_len);
	msg_struct->message[server_msg_len] = '\0';
	return 0;
}

void run_server_process(int server_in[], int server_out[], pid_t parent_pid)
{
	char **argv;
	int r;
	// this is dynamic b/c we will eventually support non-hardcoded
	// stuff. Different flags, etc.
	argv = malloc(5 * sizeof(char *));
	argv[0] = "java";
	argv[1] = "-jar";
	argv[2] = "server.jar";
	argv[3] = "nogui";
	argv[4] = NULL;

	dup2(server_in[READ_END], STDIN_FILENO);
	dup2(server_out[WRITE_END], STDOUT_FILENO);

	// took a bunch of code form this SO page for sending term signal to child process on parent exit
	// https://stackoverflow.com/questions/284325/how-to-make-child-process-die-after-parent-exits/17589555#17589555
	r = prctl(PR_SET_PDEATHSIG, SIGTERM);

	if (r == -1)
		err(1, "parent process exited before prctl...");
	if (chdir(MINECRAFT_PATH))
		err(1, "could not change directory! do you have a directory called 'mc_server'?");
	if (getppid() != parent_pid)
		err(1, "child process was adopted. not continuing...");
	if (execvp(argv[0], argv))
		err(1, "bad exec");
}
