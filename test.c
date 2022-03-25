#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1

int dup_in, nproc, ispipe;

int	ft_strlen(char *str) {
	int i = 0;

	while(str[i])
		i++;
	return (i);
}

void	ft_putstr_fd(char *str, int fd) {
	write(fd, str, ft_strlen(str));
}

void	error_fatal(void) {
	ft_putstr_fd("error: fatal\n", 2);
	exit(1);
}

// void	openpipe(int fd[2]) {
// 	close(fd[READ]);
// 	dup2(fd[WRITE], STDOUT_FILENO);
// 	close(fd[WRITE]);
// }

// void	closepipe(int fd[2]) {
// 	dup2(fd[READ], STDIN_FILENO);
// 	close(fd[READ]);
// 	close(fd[WRITE]);
// }

void	ft_exec(char **argv, char **envp) {
	pid_t	pid;
	int		fd[2];

	if (ispipe)
		if (pipe(fd) == -1)
			error_fatal();
	pid = fork();
	if (pid == -1)
		error_fatal();
	if (!pid) {
		if (ispipe) {
			close(fd[READ]);
			dup2(fd[WRITE], STDOUT_FILENO);
			close(fd[WRITE]);
		}
		if (execve(argv[0], argv, envp) == -1) {
			ft_putstr_fd("error: can't execute ", 1);
			ft_putstr_fd(argv[0], 1);
			ft_putstr_fd("\n", 1);
		}
	}
	else if (ispipe) {
		dup2(fd[READ], STDIN_FILENO);
		close(fd[READ]);
		close(fd[WRITE]);
	}
}

void	restore_fd(void) {
	int tmp;

	tmp = dup(STDIN_FILENO);
	dup2(dup_in, STDIN_FILENO);
	close(tmp);
}

int main(int argc, char **argv, char **envp) {
	int	i = 1;
	int	begin = 0;

	nproc = 0;
	argv++;
	dup_in = dup(STDIN_FILENO);
	while (argv[i]) {
		if (!strcmp(argv[i], "|") || !argv[i + 1]) {
			ispipe = 0;
			if (!strcmp(argv[i], "|")) {
				ispipe = 1;
				argv[i] = NULL;
			}
			ft_exec(argv + begin, envp);
			begin = i + 1;
			nproc++;
		}
		i++;
	}
	while (nproc-- > 0)
		waitpid(-1, 0, 0);
	// restore_fd();
	return (0);
}