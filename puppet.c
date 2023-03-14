#include "puppet.h"
#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <unistd.h>

void close_both_ends(int fd[2])
{
	close(fd[0]);
	close(fd[1]);
}
void run_puppet_process(int puppet_in[], int puppet_out[], pid_t parent_pid, char **argv, char *chdir_path)
{
	int r;
	dup2(puppet_in[READ_END], STDIN_FILENO);
	dup2(puppet_out[WRITE_END], STDOUT_FILENO);
	close_both_ends(puppet_in);
	close_both_ends(puppet_out);

	r = prctl(PR_SET_PDEATHSIG, SIGTERM);
	if (r == -1)
		err(1, "parent process exited before prctl");
	if (chdir_path != NULL) {
		if (chdir(chdir_path))
			err(1, "could not change directory to provided path.");
	}
	if (getppid() != parent_pid)
		err(1, "child process adopted, not continuing.");
	if (execvp(argv[0], argv))
		err(1, "bad exec");
}
