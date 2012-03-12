/*
 * Some code Copyright (C) 2001 Federico Di Gregorio <fog@debian.org>
 * Portions of code are from the OpenBSD project
 * 
 * This source is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#include "sysalert.h"

#ifdef NO_ASPRINTF
int
asprintf(char **ret, const char *fmt,...)
{
	va_list         ap;
	int             retval;

	va_start(ap, fmt);
	retval = vasprintf(ret, fmt, ap);
	va_end(ap);

	return retval;
}

int
vasprintf(char **ret, const char *fmt, va_list ap)
{
	char           *buf, *new_buf;
	size_t          len;
	int             retval;

	len = 128;
	buf = malloc(len);
	if (buf == NULL) {
		*ret = NULL;
		return -1;
	}
	retval = vsnprintf(buf, len, fmt, ap);
	if (retval < 0) {
		free(buf);
		*ret = NULL;
		return -1;
	}
	if (retval < len) {
		new_buf = realloc(buf, retval + 1);
		if (new_buf == NULL)
			*ret = buf;
		else
			*ret = new_buf;
		return retval;
	}
	len = (size_t) retval + 1;
	free(buf);
	buf = malloc(len);
	if (buf == NULL) {
		*ret = NULL;
		return -1;
	}
	retval = vsnprintf(buf, len, fmt, ap);
	if (retval != len - 1) {
		free(buf);
		*ret = NULL;
		return -1;
	}
	*ret = buf;
	return retval;
}
#endif

#ifdef NO_DAEMON
int 
daemon(int nochdir, int noclose)
{
	int             fd;

	switch (fork()) {
	case -1:
		return -1;
	case 0:
		break;
	default:
		exit(0);
	}
	if (setsid() == -1)
		return -1;
	if (!nochdir)
		chdir("/");

	if (!noclose && (fd = open("/dev/null", O_RDWR, 0)) != -1) {
		dup2(fd, 0);
		dup2(fd, 1);
		dup2(fd, 2);
		if (fd > 2)
			close(fd);
	}
	return 0;
}
#endif

#ifdef NO_NATIVE_STRLCPY
/**
 * Size-bounded string copy
 * @param dst Destination string
 * @param src Source string
 * @param sz Size
 * @return Amount of data copied.
 */
size_t
strlcpy(char *dst, const char *src, size_t sz)
{
	char           *d = dst;
	const char     *s = src;
	size_t          n = sz;

	if (n) {
		--n;
		while (n && *s)
			*d++ = *s++, --n;
		*d = 0;
	}
	while (*s++);

	return s - src - 1;
}

/**
 * Size-bounded string concatenation
 * @param dst Destination string
 * @param src Source string
 * @param sz Size
 * @returns Amount of data concatenated
 */
size_t
strlcat(char *dst, const char *src, size_t sz)
{
	char           *d = dst;
	const char     *s = src;
	size_t          n = sz;

	if (n) {
		--n;
		while (n && *d)
			++d, --n;
		if (n) {
			while (n && *s)
				*d++ = *s++, --n;
			*d = 0;
		}
		n = d - dst + (*d != 0);
	}
	src = s;
	while (*s++);

	return n + (s - src - 1);
}

#endif

#ifdef NO_STRDUP
char           *
strdup(char *buf)
{
	size_t          len;
	char           *ret;

	if (!buf)
		return (NULL);

	len = strlen(buf) + 1;

	ret = (char *) malloc(len);

	if (!ret)
		return (NULL);

	memcpy(ret, buf, len);
	return (ret);
}
#endif

#ifdef NO_SETPROCTITLE

#endif

#ifdef NO_UTENT
struct utmp *
getutent(void)
{

	if (ufp == NULL) {
		if ((ufp = fopen(utmpfil, "r")) == NULL) {
			dbprintf("Couldn't open utmp (%s): %s\n", utmpfil, strerror(errno));
			return((struct utmp *)NULL);
		} 
	}
	do {
		if (fread(&ut, sizeof(ut), 1, ufp) != 1) {
			return((struct utmp *)NULL);
		}
	} while (ut.ut_name[0] == 0);		/* valid entry? */

	return(&ut);
}

struct utmp *
getutline(struct utmp *line)
{
	do {
		if (strcmp(ut.ut_line, line->ut_line) == 0) {
			return(&ut);
		}
	} while (getutent() != (struct utmp *)NULL);

	return((struct utmp *)NULL);
}

void
setutent(void)
{
	if (ufp != NULL) rewind(ufp);
}
	
void
endutent(void)
{
	if (ufp != NULL) fclose(ufp);
}

void
utmpname(char *file)
{
	utmpfil = file;
}
#endif
#ifdef NO_SETPROCTITLE

void 
init_setproctitle(int argc, char **argv)
{
}

void
setproctitle(char *fmt, ...)
{

}
#endif
