#ifndef MCSERVER_H
#define MCSERVER_H

#include <sys/types.h>

#define READ_END 0
#define WRITE_END 1
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
void run_child_process(int server_in[], int server_out[], pid_t parent_pid);

#endif
