#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <error.h>

#define ERROR_EXIT(_errmsg_)	error(EXIT_FAILURE, errno, _errmsg_)
#define FILE_PATH	"/tmp/gamemesg.info"

typedef struct _mesg_ {
	pid_t pida,
		  pidb;
	int value;
}mesg_st;

void signal_hangler(int signo);

int fd;
mesg_st mesg;
int pid;
	
int main()
{
	if (SIG_ERR == signal(SIGUSR1, signal_hangler))
		ERROR_EXIT("signal");
	
	umask(0);
	if (-1 == (fd = open(FILE_PATH, O_CREAT | O_RDWR | O_EXCL, 0666))) {
		if (EEXIST == errno) {
			if (-1 == (fd = open(FILE_PATH, O_RDWR)))
				ERROR_EXIT("main.open");
		}else
			ERROR_EXIT("main.create");
	}

	pid = getpid();
	raise(SIGUSR1);

	while (1)
		pause();
	
	return 0;
}

void signal_hangler(int signo)
{
	if (-1 == lseek(fd, 0, SEEK_SET))
		ERROR_EXIT("signal_hangler.lseek");

	if (-1 == read(fd, &mesg, sizeof(mesg)))
		ERROR_EXIT("signal_hangler.read");
	
	printf("game B: %d\n", -- mesg.value);
	if (0 >= mesg.value) {
		kill(mesg.pida, SIGKILL);
		exit(EXIT_SUCCESS);
	}
	
	if (-1 == lseek(fd, 0, SEEK_SET))
		ERROR_EXIT("signal_hangler.lseek");
	
	mesg.pidb = pid;
	if (-1 == write(fd, &mesg, sizeof(mesg)))
		ERROR_EXIT("signal_hangler.write");

	if (-1 == kill(mesg.pida, SIGUSR1))
		ERROR_EXIT("signal_hangler.kill");
}
