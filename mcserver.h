#ifndef MCSERVER_H
#define MCSERVER_H

#include <sys/types.h>

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

#endif
