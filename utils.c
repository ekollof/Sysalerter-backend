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
 * the Emiel Kollof nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
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
 * $Id: utils.c,v 1.1.1.1 2008/05/14 07:19:43 root Exp $
 */

#ifndef lint
static const char copyright[] =
"utils.c, Copyright (c) 2003 Emiel Kollof <emiel@ninth-circle-alliance.net>";
#endif


#include "sysalert.h"

/**
 * Match regular expressions
 * @param string String to match on
 * @param pattern Regular expression
 * @return 1 on match, 0 on no match
 */
int
rx_match(const char *string, char *pattern)
{
	/* Matches simple regular espressions. Case sensitive */

	int             status;
	regex_t         re;


	if (regcomp(&re, pattern, REG_EXTENDED | REG_NOSUB) != 0) {
		return (0);	/* Report error. */
	}
	status = regexec(&re, string, (size_t) 0, NULL, 0);
	regfree(&re);
	if (status != 0) {
		return (0);	/* Report error. */
	}
	return (1);
}

/**
 * Match regular expressions case insensitively
 * @param string String to match on
 * @param pattern Regular expression
 * @return 1 on match, 0 on no match
 */
int
rx_imatch(const char *string, char *pattern)
{
	/* Case-insensitively matches simple regular expressions */

	int             status;
	regex_t         re;


	if (regcomp(&re, pattern, REG_EXTENDED | REG_NOSUB | REG_ICASE) != 0) {
		return (0);	/* Report error. */
	}
	status = regexec(&re, string, (size_t) 0, NULL, 0);
	regfree(&re);
	if (status != 0) {
		return (0);	/* Report error. */
	}
	return (1);
}


/**
 * Strip char from string.
 * @param buf String to process
 * @param strip character that needs to be stripped.
 */
void
stripchar(char *buf, int strip)
{
	/*
	 * strips every occurence of character buf from the string. is
	 * destructive, works in situ.
	 */

	char           *ptr = buf;
	int             len;
	char            c;
	len = strlen(buf) + 1;	/* adjust for \0 */
	while (--len > 0) {
		c = *ptr++;
		if (c != strip)
			*buf++ = c;
	}
	*buf++ = '\0';
	return;
}

/**
 * Strip char from string.
 * @param buf String to process
 * @param strip character that needs to be stripped.
 */
void
strupper(char *buf, int strip)
{
	/*
	 * strips every occurence of character buf from the string. is
	 * destructive, works in situ.
	 */

	char           *ptr = buf;
	int             len;
	char            c;
	len = strlen(buf) + 1;	/* adjust for \0 */
	while (--len > 0) {
		c = *ptr++;
		if (c != strip)
			*buf++ = c;
	}
	*buf++ = '\0';
	return;
}

/**
 * Clean out leading whitespace.
 * @param buf string to process
 */
void
cleading(char *buf)
{
	/*
	 * Nippy routine that strips leading whitespace from a string. Alters
	 * buf inplace. Be sure to strdup if you don't want to destroy the
	 * original.
	 */

	char           *ptr = buf;
	size_t          len;
	char            c;
	int             done = 0;

	len = strlen(buf) + 1;	/* adjust for trailing 0 */
	while (--len > 0) {

		c = *ptr++;

		switch (c) {
		case ' ':
			if (done)
				*buf++ = c;
			break;
		case '\t':
			if (done)
				*buf++ = c;
			break;
		default:
			*buf++ = c;
			done = 1;
			break;
		}
	}
	*buf++ = 0;
	return;
}

/**
 * Clean out trailing whitespace.
 * @param buf String to process
 */
void
ctrailing(char *buf)
{
	/*
	 * Routine that strips trailing whitespace from buf. Works back to
	 * front.
	 */

	int             len, done = 0;

	len = strlen(buf);
	while (len-- > 0) {
		switch (buf[len]) {
		case ' ':
			if (!done)
				buf[len] = 0;
			break;
		case '\t':
			if (!done)
				buf[len] = 0;
			break;
		default:
			done = 1;
			break;
		}
	}
}

/**
 * Trim trailing and leading whitespace from string
 * @param buf String to process
 */
void
trim(char *buf)
{
	/*
	 * Uses cleading and ctrailing to strip any leading and trailing
	 * whitespace from buf.
	 */
	cleading(buf);
	ctrailing(buf);
	return;
}

/**
 * Remove newlines from string
 * @param buf String to process
 */
void
chomp(char *buf)
{
	/* strips every notion of a newline from a string */
	stripchar(buf, '\n');
	stripchar(buf, '\r');
}

/**
 * Returns uid from uname
 * @param uname User name
 * @return The uid of the user in uname. Returns uid for nobody if user not found
 */
uid_t
grabuid(char *uname)
{

	/*
	 * Returns uid of user named in uname. Returns the uid of 'nobody'
	 * when the user does not exist
	 */

	struct passwd  *pw;

	pw = getpwnam(uname);
	if (!pw) {
		pw = getpwnam("nobody");
	}
	return pw->pw_uid;
}

/**
 * Split up string. Allocates memory. Don't forget to free();
 * @param string string to split
 * @param delim On which char to split (delimiter)
 * @return Pointer to position in string where delim is at.
 */
char           *
split(char *string, char *delim)
{
	/* split up key and value pair */

	char           *buf;

	chomp(string);

	buf = (char *) strdup(string);
	buf = strtok(buf, delim);
	buf = strtok(NULL, delim);

	return buf;
}

/**
 * Strip non-ascii chars from string. Not unicode-safe. Beware.
 * @param buf String to process.
 */
void
stripjunk(char *buf)
{
	/*
	 * strips every occurence of non-ascii chars from the string. is
	 * destructive, works in situ. Probably not unicode-safe. Beware on
	 * platforms that process unicode with isprint() and friends.
	 */

	char           *ptr = buf;
	int             len;
	char            c;
	len = strlen(buf) + 1;	/* adjust for \0 */
	while (--len > 0) {
		c = *ptr++;
		if (isprint(c) || c == '\n') /* preserve newlines please... */
			*buf++ = c;
	}
	*buf++ = '\0';
	return;
}


void
replacenull(char *buf, size_t len)
{
	size_t          slen;

	while (len > (slen = strlen(buf))) {
		buf[slen - 1] = ' ';
		buf += slen;
		len -= slen;
	}

	return;
}

/**
 * Reads in file from file pointer and allocates enough memory to store it. Allocates memory, so don't forget to free() it.
 * @param fp File pointer to file.
 * @return pointer to allocated memory containing the configfile.
 */
char           *
suckfile(FILE * fp)
{
	/*
	 * gets file from fp, and automatically allocates storage for it.
	 * Make sure you free when you ever use this in a library or a
	 * daemon.
	 */

	char            buf[1024], *r;
	unsigned int    size = 0;

	if (fp == NULL) {
		perror("fp was NULL? Bailing!");
		exit(1);
	}
	r = NULL;
	while (fgets(buf, sizeof(buf) - 1, fp)) {
		size += strlen(buf);
		if ((r = (char *) realloc(r, size + 1)) == NULL) {
			perror("realloc");
			exit(1);
		}
		strlcat(r, buf, size);
	}
	if (!feof(fp)) {
		perror("suckfile");
		exit(1);
		/* NOTREACHED */
	}
	return r;
}

/**
 * Debug printf
 * @param fmt Stuff to print
 */
void
dbprintf(char *fmt,...)
{
	/*
	 * A "printf" for debugging. Only prints when debugging is turned on.
	 */

	va_list         ap;
	char           *buf;
	FILE           *fp;
	time_t		now;
	char		*snow;

	time(&now);
	snow = strdup(ctime(&now));
	chomp(snow);

	if (debug) {
		int             ret;
		va_start(ap, fmt);
		ret = vasprintf(&buf, fmt, ap);
		if (ret < 0) {
			perror("utils.c: dbprintf: vasprintf error");
			return;
		}
		va_end(ap);

		if (!logging) {
			printf("%s (pid%d) [DEBUG] %s", snow, getpid(), buf);
		} else {
			fp = fopen(logfile, "a");
			if (!fp) {
				perror("Opening logfile failed...\n");
			}
			fprintf(fp, "%s (pid%d) [DEBUG] %s", snow, getpid(), buf);
			fclose(fp);
		}
		free(buf);
	}
	free(snow);
	return;
}


/**
 * Verbose printf
 * @param fmt Stuff to print
 */
void
vbprintf(char *fmt,...)
{
	/*
	 * A "printf" for verbosity. Only prints when verbosity is turned on.
	 */

	va_list         ap;
	char           *buf;
	FILE           *fp;
	time_t		now;
	char		*snow;

	time(&now);
	snow = strdup(ctime(&now));
	chomp(snow);

	if (verbose) {
		int             ret;
		va_start(ap, fmt);
		ret = vasprintf(&buf, fmt, ap);
		if (ret < 0) {
			perror("utils.c: vbprintf: vasprintf error");
			return;
		}
		va_end(ap);

		if (!logging) {
			printf("%s (pid%d) %s", snow, getpid(), buf);
		} else {
			fp = fopen(logfile, "a");
			if (fp == NULL) {
				printf("Opening logfile failed...\n");
			}
			fprintf(fp, "%s (pid%d) %s", snow,  getpid(), buf);
			fclose(fp);
		}
		free(buf);
	}
	free(snow);
	return;
}

/**
 * Puts current date in ISO format (YYYYMMDD) in date
 * @param date container for date
 */
void
getisodate(char *date)
{
	/*
	 * Generates an ISO date from the current time and plonks it in date.
	 * No bounds checking, take care. Note that date must at least be 9
	 * bytes big (that's including \0)
	 */
	time_t          now;
	struct tm      *tmdate;

	time(&now);

	tmdate = localtime(&now);

	sprintf(date, "%.4d%.2d%.2d", tmdate->tm_year + 1900, tmdate->tm_mon,
		tmdate->tm_mday);

	return;
}

/**
 * Renice process priority
 * @param pid process id
 * @param value niceness
 */
void
renice(pid_t pid, int value)
{
	/*
	 * Renices a process ID. Prints a warning when unsuccessful.
	 */

	if (setpriority(PRIO_PROCESS, pid, value) < 0)
		vbprintf("Setting prio of pid %d to %d failed!\n", pid, value);
	else
		vbprintf("Priority of pid %d is now %d\n", pid, value);

	return;
}


/**
 *	Split a string delimited by ch up into an array. Allocates mem, be sure to free
 *	@param s string that needs to be split up
 *	@param ch delimiter
 *	@param qc terminate when this char is encountered.
 *	@return pointer to array of strings, terminated with NULL
 */
char          **
strsplit(char *s, int ch, int qc)
{
	char          **ivec;
	int             ic = 0;
	int             done = 0;

	ivec = (char **) malloc((ic + 1) * sizeof(char *));

	while (!done) {
		char           *v;
		/*
		 * skip to split char
		 */
		while (*s && (ch == ' ' ? (isascii(*s) && isspace(*s)) : *s == ch))
			*s++ = '\0';

		/*
		 * End of string?
		 */
		if (!*s)
			break;

		/*
		 * remember start of string
		 */
		v = s;

		/*
		 * skip to split char
		 */
		while (*s && !(ch == ' ' ? (isascii(*s) && isspace(*s)) : *s == ch)) {
			if (*s++ == qc) {
				/*
				 * Skip past string.
				 */
				s++;
				while (*s && *s != qc)
					s++;
				if (*s == qc)
					s++;
			}
		}

		if (!*s)
			done = 1;
		*s++ = '\0';

		/*
		 * save string in new ivec slot
		 */
		ivec[ic++] = v;
		ivec = (char **) realloc((void *) ivec, (ic + 1) * sizeof(char *));
	}

	ivec[ic] = 0;

	return ivec;
}

void
hexdump(void *mybuff, size_t len)
{
	int             i = 0, j = 0, k = 0;
	unsigned char  *buff = (unsigned char *) mybuff;
	if (!buff)
		return;
	printf("\n***************************** HEXDUMP *****************************\n");
	for (; i < len; i++) {
		printf("%02X", buff[i]);

		if (i == len - 1) {
			for (k = 0; k < 16 - (i % 16); k++)
				printf("   ");
			for (; j <= i; j++) {
				if ((buff[j] >= 32) && (buff[j] <= 126))
					printf("%c", buff[j]);
				else
					printf(".");
			}
		} else if (i % 16 == 15) {
			printf("   ");
			for (; j <= i; j++) {
				if ((buff[j] >= 32) && (buff[j] <= 126))
					printf("%c", buff[j]);
				else
					printf(".");
			}
			j = i + 1;
			printf("\n");
		} else {
			printf(" ");
		}
	}
	printf("\n*******************************************************************\n");
}
