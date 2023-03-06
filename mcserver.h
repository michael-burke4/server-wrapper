#ifndef MCSERVER_H
#define MCSERVER_H

#include <sys/types.h>

#define READ_END 0
#define WRITE_END 1
#define SERVER_MSG_SIZE 33
#define MINECRAFT_PATH "./mc_server"

struct user_msg {
	char *time;
	char *user;
	char *message;
	size_t time_space;
	size_t user_space;
	size_t message_space;
};

int parse_server_output(char *buffer, struct user_msg *message);
void run_server_process(int server_in[], int server_out[], pid_t parent_pid);

#endif
