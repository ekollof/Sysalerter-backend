/*
 * Copyright (c) 2003, Emiel Kollof <coolvibe@hackerheaven.org> All rights
 * reserved.
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
 * $Id: config.c,v 1.1.1.1 2008/05/14 07:19:43 root Exp $
 */

#ifndef lint
static const char copyright[] =
"Copyright (c) 2003 Emiel Kollof <coolvibe@hackerheaven.org>";
#endif

#ifndef lint
static const char rcsid[] =
"$Id: config.c,v 1.1.1.1 2008/05/14 07:19:43 root Exp $";
#endif

unsigned int    matched;

#include "sysalert.h"

/*
 * Only for use in config_parse. Will give compile errors if used _ANYWHERE_
 * else.
 * 
 * FIXME: There must be a better way to do this...
 */

#define COMPARE_INT(cvar, fvar) 			\
	if (!strcasecmp(var, fvar)) {			\
		match++;				\
		cvar = atoi(arg);			\
	}

#define COMPARE_BOOL(cvar, fvar) 			\
	if (!strcasecmp(var, fvar)) {			\
		match++;				\
		cvar = atoi(arg);			\
	}

#define COMPARE_DOUBLE(cvar, fvar) 			\
	if (!strcasecmp(var, fvar)) {			\
		match++;				\
		cvar = (double) atof(arg);		\
	}

#define COMPARE_STRING(cvar, fvar) 			\
	if (!strcasecmp(var, fvar)) {			\
		match++;				\
		strlcpy(cvar, arg, sizeof(cvar)); 	\
	}

/**
 * Read configuration file.
 * @param config Path to config file
 * @return pointer to allocated memory containing the configuration file.
 */
char           *
config_read(char *config)
{
	/*
	 * This function checks if a configfile exists in the system and user
	 * directories. If a configuration is found, memory is allocated and
	 * the configuration file is sucked in. Returns a pointer to the
	 * configfile in memory.
	 * 
	 * Don't forget to free() after you're done with this, so we won't get
	 * any memory leaks.
	 */

	char            dbuf[MAXPATHLEN], *buf;
	struct passwd  *pw;
	FILE           *fp;

	/* Try directory in $HOME first, then try system config. */
	pw = getpwuid(getuid());
	strlcpy(dbuf, pw->pw_dir, MAXPATHLEN);
	strlcat(dbuf, "/", MAXPATHLEN - strlen(dbuf));
	strlcat(dbuf, USRCONFIGDIR, MAXPATHLEN - strlen(dbuf));
	strlcat(dbuf, "/", MAXPATHLEN - strlen(dbuf));

	/* Try USRCONFIGDIR first */
	strlcat(dbuf, config, MAXPATHLEN - strlen(dbuf));

	fp = fopen(dbuf, "r");
	if (!fp) {
		dbprintf("Couldn't open %s, trying %s/%s\n", dbuf,
			 SYSCONFIGDIR, config);

		/* Try SYSCONFIGDIR next */

		strlcpy(dbuf, SYSCONFIGDIR, MAXPATHLEN - 1);
		strlcat(dbuf, "/", MAXPATHLEN - strlen(dbuf));
		strlcat(dbuf, config, MAXPATHLEN - strlen(dbuf));

		fp = fopen(dbuf, "r");
		if (!fp) {
			fprintf(stderr, "No configuration file found, using built-in defaults.");
			return NULL;
		}
	}
	buf = suckfile(fp);

	dbprintf("Using configuration for `%s' in %s\n", config, dbuf);
	return buf;
}

/**
 * Feeds the configuration file contained in memory to the parser line by line.
 * @param config String containing configuration directives
 */
void
config_feed(char *config)
{
	/*
	 * This function feeds the configfile line by line to the configfile
	 * parser.
	 */

	char            newline[] = "\n";	/* delimiter for strtok() */
	unsigned int    line, matched;
	char           *buf, *ptr;

	buf = (char *) strdup(config);

	line = 0;
	matched = 0;
	for (;;) {

		/* First strtok needs buf, consecutive ones need NULL */
		if (line == 0) {
			ptr = (char *) strtok(buf, newline);
		} else {
			ptr = (char *) strtok(NULL, newline);
		}
		if (ptr == NULL) {	/* End of buffer */
			break;
		}
		matched += config_parse(ptr);

		line++;
	}
	dbprintf("Parsed %d lines. %d matched rules.\n", line, matched);

}

/**
 * Parses the line in 'line', setting vars when encountered
 * @param line Line containing config-line.
 */
int
config_parse(char *line)
{
	/*
	 * Configfile 'parser'. Checks the fed line for comments, empty lines
	 * and lines that have an '=' character. It splits up the line into a
	 * part before and after the equal sign, and then assigns the values
	 * to global variables with a macro.
	 */

	char           *var, *arg;
	int             match = 0;

	if (*line == '#')	/* Comment */
		return 0;
	if (*line == '\n')	/* Empty line */
		return 0;

	var = line;
	arg = (char *) strchr(var, '=');


	if (arg) {
		*arg++ = 0;	/* Nuke the '=' */
		trim(arg);	/* Nuke leading and trailing whitespace */
	} else {
		return 0;
	}

#include "config_vars.h"	/* what a weird place for an include ;) */

	if (!match)
		return 0;

	stripchar(arg, '"');

	dbprintf("Set: %s = '%s'\n", var, arg);
	return 1;
}
