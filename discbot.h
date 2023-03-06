#ifndef DISCBOT_H
#define DISCBOT_H

#define MC_TAG "<MC>"
#include <sys/types.h>

#define READ_END 0
#define WRITE_END 1

#define DISCORD_PATH "./discord_bot"

void run_discord_process(int discord_in[], int discord_out[], pid_t parent_pid);

#endif
