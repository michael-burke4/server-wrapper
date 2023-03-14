#define READ_END 0
#define WRITE_END 1

#include <sys/types.h>

void run_puppet_process(int puppet_in[], int puppet_out[], pid_t parent_pid, char **argv, char *chdir_path);
void close_both_ends(int fd[2]);
