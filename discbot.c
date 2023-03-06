#include "discbot.h"
#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <unistd.h>

void run_discord_process(int discord_in[], int discord_out[], pid_t parent_pid)
{
	char **argv;
	int r;
	// this is dynamic b/c we will eventually support non-hardcoded
	// stuff. Different flags, etc.
	argv = malloc(3 * sizeof(char *));
	argv[0] = "node";
	argv[1] = "app.js";
	argv[2] = NULL;

	dup2(discord_in[READ_END], STDIN_FILENO);
	dup2(discord_out[WRITE_END], STDOUT_FILENO);

	// took a bunch of code form this SO page for sending term signal to child process on parent exit
	// https://stackoverflow.com/questions/284325/how-to-make-child-process-die-after-parent-exits/17589555#17589555
	r = prctl(PR_SET_PDEATHSIG, SIGTERM);

	if (r == -1)
		err(1, "parent process exited before prctl...");
	if (chdir(DISCORD_PATH))
		err(1, "could not change directory! do you have a directory called 'mc_discord'?");
	if (getppid() != parent_pid)
		err(1, "child process was adopted. not continuing...");
	if (execvp(argv[0], argv))
		err(1, "bad exec");
}
