#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1

int dup_in, ispipe, ret;

int	ft_strlen(char *str) {
	int i = 0;

	while(str[i])
		i++;
	return (i);
}

void	ft_putstr_fd(char *str, int fd) {
	write(fd, str, ft_strlen(str));
}

void	fatal(void){
	ft_putstr_fd("error: fatal\n", 2);
	exit(1);
}

void	openpipe(int fd[2]) {
	if (close(fd[READ]) == -1)
		fatal();
	if (dup2(fd[WRITE], STDOUT_FILENO) == -1)
		fatal();
	if (close(fd[WRITE]) == -1)
		fatal();
}

void	closepipe(int fd[2]) {
	if (dup2(fd[READ], STDIN_FILENO) == -1)
		fatal();
	if (close(fd[READ]) == -1)
		fatal();
	if (close(fd[WRITE]) == -1)
		fatal();
}

void	restore_fd(void) {
	int tmp;

	tmp = dup(STDIN_FILENO);
	if (dup2(dup_in, STDIN_FILENO) == -1)
		fatal();
	if (close(tmp) == -1)
		fatal();
}

void	exec_fail(char *str) {
	ft_putstr_fd("error: can't execute ", 2);
	ft_putstr_fd(str, 2);
	ft_putstr_fd("\n", 2);
	exit(1);
}

void	ft_cd(char **argv) {
	int i = 0;
	while(argv[i])
		i++;
	if (i != 2) {
		ft_putstr_fd("error: cd: bad argumnets\n", 2);
		ret = 1;
		return ;
	}
	if (chdir(argv[1]) == -1) {
		ft_putstr_fd("error: cd: cannot change directory to ", 2);
		ft_putstr_fd(argv[1], 2);
		ft_putstr_fd("\n", 2);
		ret = 1;
		return ;
	}
}

void	final_exec(char **argv, char **envp) {
	pid_t pid;
	int fd[2];

	if (!strcmp(argv[0], "cd"))
		return (ft_cd(argv));
	if (ispipe)
		if (pipe(fd) == -1)
			fatal();
	pid = fork();
	if (pid == -1)
		fatal();
	if (!pid) {
		if (ispipe)
			openpipe(fd);
		if (execve(argv[0], argv, envp) == -1)
			exec_fail(argv[0]);
	}
	else if (ispipe)
		closepipe(fd);
}

void	ft_exec(char **argv, char **envp) {
	int i = 0;
	int begin = 0;
	int nproc = 0;

	while (argv[i]) {
		if (!strcmp(argv[i], "|") || !argv[i + 1]) {
			ispipe = 0;
			if (!strcmp(argv[i], "|")) {
				ispipe = 1;
				argv[i] = NULL;
			}
			final_exec(argv + begin, envp);
			begin = i + 1;
			nproc++;
		}
		i++;
	}
	while(nproc-- > 0)
		waitpid(-1, 0, 0);
}

int main(int argc, char **argv, char **envp) {
	int i = 1;
	int begin = 0;
	int nproc = 0;

	argv++;
	dup_in = dup(STDIN_FILENO);
	(void)argc;
	while (argv[i]) {
		if (!strcmp(argv[i], ";") || !argv[i + 1]) {
			if (!strcmp(argv[i], ";"))
				argv[i] = NULL;
			ft_exec(argv + begin, envp);
			begin = i + 1;
		}
		ret = 0;
		i++;
		restore_fd();
	}
	return (ret);
}