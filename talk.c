/*
 * MONITORD (c) 2004-2007 Hackerheaven dot ORG
 * 
 * Door: Emiel Kollof <coolvibe@hackerheaven.org>
 * 
 * $Id: talk.c,v 1.11 2004/10/12 08:42:19 gp Exp $
 * 
 */

#ifndef lint
static const char copyright[] =
"talk.c, Copyright (c) 2004-2007 Emiel Kollof (http://www.hackerheaven.org)";
#endif

#ifndef lint
static const char rcsid[] =
"$Id: talk.c,v 1.11 2004/10/12 08:42:19 gp Exp $";
#endif

#include "sysalert.h"

/**
 * \brief Handelt communicatie met de client af
 * @param sock
 */
void
talk_interface(int sock)
{
	int             disc = 0;
	char            command[1024];

	nprintf(sock, "sysalerter OK - on line %d - version v%s\n",
		sock, VERSION);

	database_init();	/* needed, different instance than parent */

	while (1) {
		network_gets(sock, command, sizeof(command) - 1);
		chomp(command);	/* remove newlines */
		stripjunk(command);	/* and unprintables */
		if (strncmp(command, "", 1)) {
			dbprintf("%d: <<< %s\n", sock, command);
			disc = talk_parse(sock, command);
		}
		if (disc)
			break;
	}
	shutdown(sock, SHUT_WR);
	close(sock);
	return;
}

int
talk_parse(int sock, char *command)
{
	if (rx_match(command, "^getnumprocs$")) {
		int             nprocs;
		nprocs = getnumprocs();
		nprintf(sock, "%d processes\n", nprocs);
		return 0;
	}
	if (rx_match(command, "^isprocalive.*")) {
		struct process *myproc;
		char           *buf, *bufptr;
		char            space[] = " ";

		buf = strdup(command);
		bufptr = split(buf, space);	/* XXX: memleak? */

		if (bufptr != NULL) {
			myproc = getprocbyname(bufptr);
			if (myproc != NULL)
				nprintf(sock, "yes - (pid %d) %s\n",
					myproc->ppid,
					myproc->pargv);
			else
				nprintf(sock, "no\n");
			free(buf);
			free(myproc);
		} else {
			nprintf(sock, "isprocalive <regex>\n");
			free(buf);
		}
		return 0;
	}
	if (rx_match(command, "^getpidsize.*")) {
		struct process *myproc;
		char           *buf, *bufptr;
		char            space[] = " ";

		buf = strdup(command);
		bufptr = split(buf, space);	/* XXX: memleak? */

		if (bufptr != NULL) {
			myproc = getprocbypid(atoi(bufptr));
			if (myproc != NULL)
				nprintf(sock, "%d KB (%s)\n",
					myproc->procsize,
					myproc->pargv);
			else
				nprintf(sock, "pid does not exist\n");
			free(buf);
			free(myproc);
		} else {
			nprintf(sock, "getpidsize <pid>\n");
			free(buf);
		}
		return 0;
	}
	if (rx_match(command, "^getprocsize.*")) {
		struct process *myproc;
		char           *buf, *bufptr;
		char            space[] = " ";

		buf = strdup(command);
		bufptr = split(buf, space);	/* XXX: memleak? */

		if (bufptr != NULL) {
			myproc = getprocbyname(bufptr);
			if (myproc != NULL)
				nprintf(sock, "%d KB (%s)\n",
					myproc->procsize,
					myproc->pargv);
			else
				nprintf(sock, "process does not exist\n");
			free(buf);
			free(myproc);
		} else {
			nprintf(sock, "getprocsize <regex>\n");
			free(buf);
		}
		return 0;
	}
	if (rx_match(command, "^getlocation$")) {
		char           *loc;

		loc = getlocation();
		if (loc == NULL) {
			nprintf(sock, "ERR: No location known. Please set one with setlocation command\n");
			return 0;
		}
		chomp(loc);
		nprintf(sock, "LOC: %s\n", loc);
		free(loc);
		return 0;
	}
	if (rx_match(command, "^setlocation.*")) {
		char           *buf, *bufptr;
		char            space[] = " ";

		buf = strdup(command);
		bufptr = split(buf, space);

		if (bufptr != NULL) {
			if ((rx_match(bufptr, "^[a-zA-Z][0-9]{2}:[0-9]{2}$") ||
			     rx_match(bufptr, "^[a-zA-Z][0-9]{2}$")) == 0) {
				/* Invalid format */
				nprintf(sock, "Invalid format: should be <rowletter><racknumber>:<which> or <rowletter><racknumber>\nExample: g03:12 or d05\n");
				free(buf);
				return 0;
			}
			nprintf(sock, "Writing to %s\n", location);
			setlocation(bufptr);
			free(buf);
			return 0;
		} else {
			nprintf(sock, "setlocation <row><cabinet>:[count]\n");
			free(buf);
		}

	}
	if (rx_match(command, "dump.*")) {
		char           *buf, *bufptr;
		node           *result;
		char            space[] = " ";
		node           *start;
		int		items = 0;

		buf = strdup(command);
		bufptr = split(buf, space);

		if (bufptr != NULL) {
			nprintf(sock, "# Fetching data. Please be patient.\n");

			if ((result = database_fetch_status(bufptr)) != NULL) {;

				start = result;
				while (start) {
					nprintf(sock, "%s\n", start->data);
					free(start->data);	/* clean up */
					start = start->next;
					items++;
				}
				mylist_destroy(result);
				nprintf(sock, "# end of data\n");
				dbprintf("talk:dump: sent %d item(s)\n", items);
				return 0;

			} else {
				nprintf(sock, "no results\n");
			}

			return 0;
		} else {
			nprintf(sock, "dump <type>\n");
			free(buf);
		}
		return 0;
	}
	if (rx_match(command, "clear.*")) {
		char           *buf, *bufptr;
		char            space[] = " ";

		buf = strdup(command);
		bufptr = split(buf, space);

		if (bufptr != NULL) {
			database_clear_status(bufptr);
			database_vacuum();
			return 0;
		} else {
			nprintf(sock, "clear <type>\n");
			free(buf);
		}
		return 0;
	}
	if (rx_match(command, "getinterval")) {
		nprintf(sock, "%d\n", interval);
		return 0;
	}
	if (rx_match(command, "numcpus")) {
		int ncpu;
		ncpu = num_cpus();
		nprintf(sock, "%d\n", ncpu);
		return 0;
	}
	if (rx_match(command, "listusers")) {
		struct utmp *u;
		int users = 0;
		#ifndef NO_UTENT
		time_t recent = 0;
		#endif

		setutent();		
		while(1) {
			u=getutent();
			if (u == NULL)
				break;
		
			#ifndef NO_UTENT	
			if (u->ut_type != USER_PROCESS) {
				continue;	
			}

			if (u->ut_time > recent) {
				recent = u->ut_time;
				nprintf(sock, "%s:%s:%d;", u->ut_user, u->ut_line, u->ut_time);
				users++;
			}
			#else 
			nprintf(sock, "%s:%s:%d;", u->ut_name, u->ut_line, u->ut_time);
			users++;
			#endif
		}
		endutent();
		if (users == 0) {
			nprintf(sock, "no users\n");
		} else {
			nprintf(sock, "\n");
		}
		return 0;
	}

	if (rx_match(command, "lastlogin")) {
		struct passwd *pw;
		time_t logintime;
		int users = 0;

		setpwent();
		while((pw = getpwent())) {
			logintime = users_lastlog(pw->pw_uid);
			if (logintime == 0)
				continue;
			nprintf(sock, "%s:%u;", pw->pw_name, logintime);
			users++;
		}
		endpwent();

		if (users == 0) 
			nprintf(sock, "no records\n");
		else 
			nprintf(sock, "\n");

		return 0;
	}
	if (rx_match(command, "os")) {
		#ifdef SOLARIS
		nprintf(sock, "SunOS\n");
		#endif
		#ifdef LINUX
		nprintf(sock, "Linux\n");
		#endif
		#ifdef FREEBSD
		nprintf(sock, "FreeBSD\n");
		#endif
		return 0;
	}
#ifdef SOLARIS
	if (rx_match(command, "prtdiag")) {
		FILE *my_stdout;
		char line[1024];
		
		if ((my_stdout = (FILE *) popen("/usr/sbin/prtdiag 2>&1", "r")) != NULL) {
			while (fgets(line, 1024, my_stdout)) {
				chomp(line);
				dbprintf("%s\n", line);
				nprintf(sock, "%s\n", line);
			}
			pclose(my_stdout);
			dbprintf("\n", line);
		} else {
			dbprintf("talk_parse: something went wrong... %s\n",
				 strerror(errno));
		}
	

		return 0;
	}
#endif
	if (rx_match(command, "cleanup")) {
		database_vacuum();
		nprintf(sock, "cleanup done.\n");
		return 0;
	}
	if (!strncmp(command, "exit", 1023)) {
		return 1;
	}
	nprintf(sock, "Unknown command.\n");
	return 0;
}
