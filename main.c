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
#include <unistd.h>

#define READ_END 0
#define WRITE_END 1
#define BUFFSIZE 2048
#define SERVER_MSG_SIZE 33

struct user_msg {
	char *time;
	char *user;
	char *message;
	size_t time_space;
	size_t user_space;
	size_t message_space;
};

int process(char *buffer, struct user_msg *message);

int main(void)
{
	char **argv;
	pid_t ppid_before_fork, pid;
	int server_input[2], server_output[2];
	char buffer[BUFFSIZE];
	ssize_t num_read;
	struct user_msg msg = {
		.time = NULL,
		.user = NULL,
		.message = NULL,
		.time_space = 0,
		.user_space = 0,
		.message_space = 0
	};

	argv = malloc(5 * sizeof(char *));
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


	while ((num_read = read(server_output[READ_END], buffer, BUFFSIZE))) {
		printf("%s", buffer);

		if (0 == process(buffer, &msg)) {
			printf("%s said: %s", msg.user, msg.message);
		}
		for (int i = 0; i < num_read; ++i) {
			buffer[i] = '\0';
		}
		if (errno)
			printf("error: %s\n", strerror(errno));
	}
}

// [21:59:22] [Server thread/INFO]: <desayuno>

int process(char *buffer, struct user_msg *msg_struct)
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
