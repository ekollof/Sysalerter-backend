/*
 * MONITORD (c) 2004,2008 Emiel Kollof <emiel@ninth-circle-alliance.net>
 * 
 * Door: Emiel Kollof
 * 
 */

#include "sysalert.h"


struct webaddr *
http_parse_url(char *url)
{
	struct webaddr *address;

	address = malloc(sizeof(struct webaddr));
	if (!address) {
		perror("http_parse_url: malloc");
		return NULL;
	}

	// XXX: Could be an off by one here, investigate and fix
	sscanf(url, "%9[^:]://%255[^/]%1024[^\r\n]", address->proto, address->host, address->path);

	dbprintf("Proto: %s, Host: %s, Path: %s\n", address->proto, address->host, address->path);

	if (strcmp("http", address->proto)) {
		dbprintf("We support only standard http");
		return NULL;
	}
	address->port = 80;

	return address;
}

void 
http_fetch_url(char *url)
{
	struct webaddr *address;
	int sock;
	char *buf = NULL;
	char line[1024];
	struct utsname myname;
	#ifndef DONTFORK
	int hpid;
	#endif

	if (!heartbeat) {
		return;
	}

	uname(&myname);

	address = http_parse_url(url);
	if (!address) {
		free(address);
		return;
	}


#ifndef DONTFORK
	hpid = fork();
	switch(hpid) {
		case 0: /* child */
#endif
			setproctitle("sysalert: fetching from %s", url);
			if (!asprintf(&buf, "%d", address->port)) {
				dbprintf("Eep! asprintf in http_fetch_url went awry!\n");
				free(address);
				free(buf);
				dbprintf("http client exiting\n");
				exit(-1);
			}

			sock = network_connect(address->host, buf);
			if (sock < 0) {
				dbprintf("http_fetch_url: Whoops! Not good!\n");
				free(address);
				free(buf);
				dbprintf("http client exiting\n");
				exit(-1);
			}

			dbprintf("http_fetch_url: Connected to %s.\n", address->host);
			nprintf(sock, "GET %s?hostname=%s&port=%d HTTP/1.1\n", address->path, myname.nodename, portnum);
			nprintf(sock, "Host: %s\n", address->host);
			nprintf(sock, "User-Agent: Sysalerter running on %s\n\n", myname.nodename);

			while (network_gets(sock, line, sizeof(line) -1)) {
				chomp(line);
                                /* This is handy for debugging */
				dbprintf("HTTP: <<< %s\n", line);
			}

			close(sock);
			free(address);
			free(buf);
			dbprintf("http client exiting\n");
#ifndef DONTFORK
			exit(0);
			break; /* NOTREACHED */
		case -1: /* error */
			perror("http_fetch_url:fork");
			break;
		default: /* parent */
			dbprintf("Forked off http client with pid %d\n", hpid);
			waitpid(hpid, NULL, 0); /* wait for child to finish */
			break;
	}	
#endif	
	return;
}
