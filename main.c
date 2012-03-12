/*
 * LOADCHECK (c) 2004,2007 Emiel Kollof <emiel@ninth-circle-alliance.net>
 * 
 * Door: Emiel Kollof
 * 
 */

#ifndef lint
static const char copyright[] =
"Copyright (c) 2004,2007 Emiel Kollof <emiel@ninth-circle-alliance.net>";
#endif

#ifndef lint
static const char rcsid[] =
"$Id: main.c,v 1.1.1.1 2008/05/14 07:19:43 root Exp $";
#endif

#include "sysalert.h"

int 
main(int argc, char **argv, char **environ)
{
	int             ch;
	char           *cwd;
	char           *cbuf;
	int             lpid, status;
	struct sigaction sa;


	while ((ch = getopt(argc, argv, "ndlvx")) != -1) {
		switch (ch) {
		case 'n':
			nodaemon = 1;
			break;
		case 'd':
			debug = 1;
			verbose = 1;
			break;
		case 'l':
			debug = 1;
			verbose = 1;
			logging = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'x':
			heartbeat = 0;
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;

#ifdef NO_SETPROCTITLE
	init_setproctitle(argc, argv);
	setproctitle("sysalert: main");
#endif

	cwd = getcwd(NULL, MAXPATHLEN);
	printf("System Alerter v%s, by Emiel Kollof\n"
	       "Current working directory: %s\n",
	       VERSION, cwd);

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
        if (sigaction(SIGABRT, &sa, NULL) == -1) {
                perror("sigaction");
                exit(1);
        }


	if (nodaemon == 0) {
		int             fd, ret = 0;
		char           *buf;

		dbprintf("Forking to background, see sysalert.pid for pid number\n");
		if (daemon(0, 0) < 0) {
			dbprintf("Daemonizing failed, aborting\n");
			exit(1);
		}
		ret = asprintf(&buf, "%d\n", getpid());
		if (ret < 0) {
			perror("main.c: main: asprintf failed");
			return 1;
		}
		if (buf == NULL) {
			perror("main: something screwed up...");
			exit(1);
		}
		fd = open("sysalert.pid", O_RDWR | O_CREAT, 0644);
		chmod("sysalert.pid", 0644);
		ret = write(fd, buf, strlen(buf) + 1);
		close(fd);
		free(buf);
	}
	cbuf = (char *) config_read(CONFIGFILE);
	if (cbuf) {
		config_feed(cbuf);
		free(cbuf);
	}

#ifdef SOLARIS
	ks_init();
#endif

	/* fork off the listener */
	lpid = fork();
	switch (lpid) {
	case 0:		/* child */
		vbprintf("Starting listener\n");
		setproctitle("sysalerter: listener thread");
		network_mainloop(talk_interface);
		exit(0);
		break; /* NOTREACHED */
	case -1:		/* error */
		perror("fork");
		exit(1);
		break;
	default:		/* parent */
		vbprintf("Forked listener with pid %d\n", lpid);
		while (waitpid(lpid, &status, WNOHANG) == 0) {	/* don't block the
								 * parent process */
			check_mainloop();
		}
		break;		/* NOTREACHED */
	}
	return (0);
}

void 
usage(char *progname)
{
	printf("%s usage:\n"
	       "%s [-c <configfile>] [-ndl]\n\n"
	       "-d             Debug mode (including verbose)\n"
	       "-l		Log mode. Implies daemon-mode + debug\n"
	       "-n		Don't become a daemon\n",
	       progname, progname);
	exit(0);
}
