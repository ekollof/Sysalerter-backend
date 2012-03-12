/*
 * SYSALERTER (c) 2004,2008 Emiel Kollof <emiel@ninth-circle-alliance.net>
 * 
 * Door: Emiel Kollof <emiel@ninth-circle-alliance.net>
 * 
 * $Id: check.c,v 1.1.1.1 2008/05/14 07:19:43 root Exp $
 */

#include "sysalert.h"

#ifdef DB_TYPE_SQLITE

#include "dbinit.h"

void
sqlite_init(void)
{
	char           *dberr = NULL;
	int             res;

	vbprintf("SQlite init: %s\n", dbdir);
	sqlite3_open(dbdir, &db);
	if (!db) {
		vbprintf("sqlite_init: unable to open db\n");
		exit(1);
	}
	/* Check if database contains any data or structure */
	res = sqlite3_exec(db, "select count(id) from status;", NULL, NULL, NULL);
	if (res != SQLITE_OK && res != SQLITE_BUSY) {
		res = sqlite3_exec(db, createdb, NULL, NULL, &dberr);
		if (res != SQLITE_OK && res != SQLITE_BUSY) {
			vbprintf("sqlite_init: PANIC: Couldn't create table: %s\n",
				dberr);
			free(dberr);
			exit(-1);
		}
	}
	/* Seems like all is okay */
	vbprintf("sqlite_init: Database ready\n");
	sqlite3_close(db);

	return;
}

int
sqlite_insert_stat(char *type, char *value, int status)
{
	int             res;
	char           *dberr;
	char           *query;
	time_t          now;

	time(&now);

	if (asprintf(&query, "PRAGMA journal_mode=OFF; INSERT INTO status (status, value, alertstatus, lastalert) VALUES ('%s', '%s', '%d', '%lu');",
		     type, value, status, (unsigned long int) now) < 0) {
		perror("sqlite_insert_stat: asprintf");
	}
	sqlite3_open(dbdir, &db);
	res = sqlite3_exec(db, query, NULL, NULL, &dberr);
	if (res != SQLITE_OK) {
		vbprintf("sqlite_insert_stat: ERROR: %s\n", dberr);
		if (res != SQLITE_OK) { /* trying again */
			while(res == SQLITE_BUSY) {
				res = sqlite3_exec(db, query, NULL, NULL, &dberr);
			}
		}
		sqlite3_close(db);
		return 0;
	}
	sqlite3_close(db);
	free(query);
	return 1;
}

int
sqlite_generic_callback(void *result, int argc, char **argv, char **colname)
{
	int             i;
	int             len = 0;
	char           *buf = NULL;
	char           *copy;

	for (i = 0; i < argc; i++) {

		/*
		 * dbprintf("colname %s, value %s, strlen = %d\n"ar **
		 * sqlite_return_rows(sqlite3 *db, char *sql, ...)
		 * colname[i], argv[i] ? argv[i] : "NULL", strlen(argv[i]));
		 * 
		 */

		if (asprintf(&copy, "%s;", argv[i]) < 0) {
			perror("sqlite_generic_callback: asprintf");
		}
		len += strlen(copy) + 1;

		if ((buf = (char *) realloc(buf, len)) == NULL) {
			perror("sqlite_generic_callback: realloc");
			exit(1);
		}
		/*
		 * XXX: Gross hack, I have to null out the realloced space
		 * first, but I will do that later. This works for now.
		 */
		if (i == 0) {
			if (strlcpy(buf, copy, len) >= len) {
				dbprintf("OVERFLOW: strlcpy");
				return 0;
			}
		} else {
			if (strlcat(buf, copy, len) >= len) {
				dbprintf("OVERFLOW: strlcat");
				return 0;
			}
		}

		free(copy);
	}

	/* add row to result */
	mylist_additem(result, buf, &lresult_start, &lresult_last);
	return 0;
}

node           *
sqlite_return_rows(sqlite3 * db, char *fmt,...)
{
	va_list         ap;
	char           *query, *dberr;
	int             ret;
	node           *result = NULL;

	va_start(ap, fmt);
	ret = vasprintf(&query, fmt, ap);
	va_end(ap);

	lresult_start = lresult_last = NULL;	/* these are global */
	sqlite3_open(dbdir, &db);
	ret = sqlite3_exec(db, query, sqlite_generic_callback, result, &dberr);

	/* XXX Ugly, I know. So what. It doesn't really matter */
	if (ret != SQLITE_OK && ret != SQLITE_BUSY) { /* Something else went wrong. Bail */
		vbprintf("sqlite_return_rows: SQL error: %s\n", dberr);
		sqlite3_close(db);
		free(dbresult);
		free(dberr);
		return NULL;
	}

	/* XXX There is probably a cleaner way to do this */
	if (ret == SQLITE_BUSY) {
		while (ret != SQLITE_OK) { /* try again! */
			ret = sqlite3_exec(db, query, sqlite_generic_callback, result, &dberr);
			vbprintf("sqlite_return_rows: SQL error: %s\n", dberr);
			sleep(1);
		}
	}
	sqlite3_close(db);
	free(query);
	return lresult_start;
}

#endif
