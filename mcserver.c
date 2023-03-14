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
