#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

int dup_stdin, ret, haspipe;

int	ft_strlen(char *str)
{
	int i = 0;

	while (str[i])
		i++;
	return (i);
}

void	ft_putstr_fd(char *str, int fd)
{
	write(fd, str, ft_strlen(str));
}

void	error_fatal(void)
{
	ft_putstr_fd("error: fatal\n", 2);
	exit(1);
}

void	error_exec(char *str)
{
	ft_putstr_fd("error: cannot execute ", 2);
	ft_putstr_fd(str, 2);
	ft_putstr_fd("\n", 2);
	exit(1);
}

void	ft_cd(char **argv)
{
	int i = 0;

	while(argv[i])
		i++;
	if (i != 2)
	{
		ft_putstr_fd("error: cd: bad arguments\n", 2);
		ret = 1;
		return ;
	}
	if (chdir(argv[1]) == -1)
	{
		ft_putstr_fd("error: cd: cannot change directory to ", 2);
		ft_putstr_fd(argv[1], 2);
		ft_putstr_fd("\n", 2);
		ret = 1;
		return ;
	}
}

void	ft_openpipe(int fd[2])
{
	if (haspipe)
	{
		if (close(fd[0] == -1))
			error_fatal();
		if (dup2(fd[1], 0) == -1)
			error_fatal();
		if (close(fd[1]) == -1)
			error_fatal();
	}
}

void	ft_restore_fd(void)
{
	int tmp;
	tmp = dup(0);
	if (dup2(dup_stdin, 0) == -1)
		error_fatal();
	if (close(tmp) == -1)
		error_fatal();
}

void	ft_closepipe(int fd[2])
{
	if (dup2(fd[0], 0) == -1)
		error_fatal();
	if (close(fd[0]) == -1)
		error_fatal();
	if (close(fd[1]) == -1)
		error_fatal();
}

void	ft_exec(char **argv, char **env)
{
	pid_t	pid;
	int		fd[2];

	if (!strcmp(argv[0], "cd"))
		return(ft_cd(argv));
	if (haspipe)
		if (pipe(fd) == -1)
			error_fatal();
	pid = fork();
	if (!pid)
	{
		ft_openpipe(fd);
		if (execve(argv[0], argv, env) == -1)
			error_exec(argv[0]);
	}
	else
		ft_closepipe(fd);
}

void	pre_exec(char **cmd, char **env)
{
	int i = 0;
	int begin = 0;
	int nproc = 0;

	while (cmd[i])
	{
		if (!strcmp(cmd[i], "|") || !cmd[i + 1])
		{
			haspipe = 0;
			if (!strcmp(cmd[i], "|"))
			{
				haspipe = 1;
				cmd[i] = NULL;
			}
			ft_exec(cmd + begin, env);
			begin = i + 1;
			nproc++;
		}
		i++;
	}
	while(nproc-- > 0)
		waitpid(-1, 0, 0);
}

int main(int argc, char **argv, char **env)
{
	int i = 1;
	int	begin = 1;

	dup_stdin = dup(0);
	while(argv[i])
	{
		if (!strcmp(argv[i], ";") || !argv[i + 1])
		{
			if (!strcmp(argv[i], ";"))
				argv[i] = NULL;
			pre_exec(argv + begin, env);
			begin = i + 1;
		}
		ret = 0;
		i++;
		ft_restore_fd();
	}
	return (ret);
}