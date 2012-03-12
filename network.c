/*
 * Copyright (c) 2003, Emiel Kollof <emiel@ninth-circle-alliance.net> All
 * rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution. Neither the name of
 * the <ORGANIZATION> nor the names of its contributors may be used to
 * endorse or promote products derived from this software without specific
 * prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * 
 * Created by Emiel Kollof on Tue Nov 25 2003. Copyright (c) 2003 All rights
 * reserved.
 * 
 * $Id: network.c,v 1.1.1.1 2008/05/14 07:19:43 root Exp $
 */

#include "sysalert.h"
#ifdef NO_ADDRINFO
#include "getaddrinfo.h"
#endif

#ifdef WANT_LIBWRAP
#include <tcpd.h>
#include <syslog.h>
static struct request_info request;
#endif

/**
 * \brief Signal handler voor network_mainloop
 * @param s
 */
void
sigchild_handler(int s)
{
	while (wait(NULL) > 0);
}

/**
 * \brief Mainloop voor de server
 * @param
 */
void
network_mainloop(client_hndlr clientfunc)
{
	int             csock, ssock, clen;
	struct sockaddr_in server;
	struct sockaddr_in client;
	int             pid;
	struct sigaction sa;

	ssock = network_init(&server, sizeof(server));

/* XXX: not needed? */
	sa.sa_handler = sigchild_handler;	/* reap zombies */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	while (1) {
		clen = sizeof(client);
		csock = accept(ssock, (struct sockaddr *) & client,
			       (unsigned int *) &clen);
		if (csock < 0) {
			/*
			 * FIXME: Somehow this gets called again, giving a
			 * non fatal, but very irritating error. Find out why
			 * accept gets triggered again and fix that. I
			 * suspect something in talk.c not closing the socket
			 * correctly which could make accept() bark.
			 *
			 * XXX: Is this still happening? 
			 */
			dbprintf("Weird case triggered in network_mainloop. See FIXME\n");
			continue;
		}
#ifdef WANT_LIBWRAP
		request_init(&request, RQ_DAEMON, "sysalert", RQ_FILE, csock, 0);
		fromhost(&request);

		if (!hosts_access(&request)) {
			/* denied by tcp wrappers (/etc/hosts.allow/deny) */
			syslog(LOG_ERR, "sysalert: connection refused for %s by tcp wrappers",
			       inet_ntoa(client.sin_addr));
			if (request.sink)	/* flush remaining data */
				request.sink(request.fd);

			sleep(5);	/* prevent DoS */
			close(csock);
			continue;
		}
#endif
		pid = fork();
		switch (pid) {
		case 0:	/* wij zijn kind */
			close(ssock);	/* listening socket niet nodig voor
					 * child */
			setproctitle("sysalerter: client thread connected to %s", inet_ntoa(client.sin_addr));
			vbprintf("network_mainloop: New caller on line %d from %s with pid %d\n", csock,
				 inet_ntoa(client.sin_addr), getpid());

			clientfunc(csock);
			exit(0);
			break;	/* NOTREACHED */
		case -1:	/* er ging wat fout */
			perror("fork");
			exit(1);
			break;	/* NOTREACHED */
		default:	/* wij zijn parent */
			close(csock);	/* parent doesn't need it */
			break;

		}
	}
}


/**
 * \brief Deze functie inititialiseert een listen socket en bind deze aan een poort.
 * @param local
 * @param llen
 * @return descriptor van socket, of stop programma als mislukt
 */
int
network_init(struct sockaddr_in * local, socklen_t llen)
{
	const int       enable = 1;
	int             sock, error, ret;
	char           *buf;

	ret = asprintf(&buf, "%d", portnum);
	if (ret < 0) {
		perror("network.c: network_init: asprintf error");
		return (-1);
	}
	if (strncmp(listenaddr, "all", sizeof(listenaddr) - 1))
		network_setaddress(listenaddr, buf,
				   (struct sockaddr_in *) & local);
	else
		network_setaddress(NULL, buf,
				   (struct sockaddr_in *) & local);

	free(buf);

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(1);
	}
	error = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable,
			   sizeof(enable));
	if (error) {
		perror("setsockopt");
		exit(1);
	}
	error = bind(sock, (struct sockaddr *) & local, sizeof(struct sockaddr));
	if (error < 0) {
		perror("bind");
		exit(1);
	}
	error = listen(sock, 120);
	if (error) {
		perror("listen");
		exit(1);
	}
	return sock;
}

/**
 * \brief Deze functie stoomt de listening socket klaar voor gebruik.
 * @param host
 * @param service
 * @param sap
 */
void
network_setaddress(char *host, char *service, struct sockaddr_in * sap)
{
	struct hostent *hp;
	char           *endptr;
	short           port;

	memset(sap, 0, sizeof(struct sockaddr_in));
	sap->sin_family = AF_INET;

	if (host) {
		if (!inet_aton(host, &sap->sin_addr)) {
			hp = gethostbyname(host);
			if (!hp) {
				perror("gethostbyname");
				exit(1);
			}
			sap->sin_addr = *(struct in_addr *) hp->h_addr;
		}
	} else {
		sap->sin_addr.s_addr = htonl(INADDR_ANY);
	}

	vbprintf("Listening on: %s\n", listenaddr);
	vbprintf("On port: %s\n", service);
	port = strtol(service, &endptr, 0);
	if (*endptr == 0) {
		sap->sin_port = htons(port);
	}
	return;
}




/**
 * @brief Connects to hostname on port sname (is ipv6 compatibe)
 * @param hname Host name or IP to connect to
 * @param sname Service port or name
 * @return Connection socket
 */
int
network_connect(char *hname, char *sname)
{
	struct addrinfo *res, *aip, hints;
	int             error;
	int             s = -1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	error = getaddrinfo(hname, sname, &hints, &res);
	if (error != 0) {
		vbprintf("network_connect: getaddrinfo: %s \n", gai_strerror(error));
		exit(1);
	}
	for (aip = res; aip != NULL; aip = aip->ai_next) {
		s = socket(aip->ai_family, aip->ai_socktype, aip->ai_protocol);
		if (s < 0)
			continue;

		if (connect(s, aip->ai_addr, aip->ai_addrlen) == -1) {
			(void) close(s);
			s = -1;
			continue;
		}
	}
	freeaddrinfo(res);
	return s;
}

/**
 * \brief Send raw string to network socket
 * @param fd File descriptor to write to
 * @param buf Buffer to send over the network
 * @return Amount of bytes sent
 */
int
network_send(int fd, char *buf)
{
	int             bout;
	bout = 0;

	if (fd < 0) {
		vbprintf("Socket not connected...\n");
		return -1;
	}
	if ((bout = send(fd, buf, strlen(buf), 0)) < 0) {
		if (bout == 0) {
			vbprintf("netsend: socket disconnected.\n");
			close(fd);
			exit(0);
		} else {
			perror("netsend");
			exit(1);
		}
	}
	return (bout);
}

/**
 * \brief Network printf
 * @param sock File descriptor to send to
 * @param fmt fmt buffer (printf style)
 * @return Amount of bytes sent
 */
int
nprintf(int sock, char *fmt,...)
{

	va_list         ap;
	char           *buf;
	int             ret;

	va_start(ap, fmt);
	ret = vasprintf(&buf, fmt, ap);
	if (ret < 0) {
		perror("network.c: nprintf: vasprintf error");
		return -1;
	}
	va_end(ap);

	network_send(sock, buf);
	free(buf);
	return 0;

}

/**
 * \brief Read len bytes from socket fd
 * @param fd File descriptor to read from
 * @param bp Buffer to fill
 * @param len length of buffer
 * @return Amount of bytes read
 */
int
network_read(int fd, char *bp, size_t len)
{
	int             cnt;
	int             rc;

	cnt = len;
	while (cnt > 0) {
		rc = recv(fd, bp, cnt, 0);
		if (rc < 0) {	/* read error? */
			if (errno == EINTR)	/* interruption */
				continue;
			return -1;
		}
		if (rc == 0)	/* EOF */
			return len - cnt;
		bp += rc;
		cnt -= rc;
	}
	return (len);
}

/**
 * \brief Read a line from fd until end of file or \n is encountered
 * @param fd Filedescriptor to read from
 * @param bufptr Buffer to fill
 * @param len buffer length
 * @return Amount of bytes read
 */
int
network_gets(int fd, char *bufptr, size_t len)
{
	char           *bufx = bufptr;
	static char    *bp;
	static int      cnt = 0;
	static char     b[1500];
	char            c;

	while (--len > 0) {
		if (--cnt <= 0) {
			cnt = recv(fd, b, sizeof(b), 0);
			if (cnt < 0) {
				if (errno == EINTR) {
					len++;
					continue;
				}
				return -1;
			}
			if (cnt == 0)
				return 0;
			bp = b;
		}
		c = *bp++;
		*bufptr++ = c;
		if (c == '\n') {
			*bufptr = '\0';
			return bufptr - bufx;
		}
	}
	errno = (EMSGSIZE);
	return -1;
}
