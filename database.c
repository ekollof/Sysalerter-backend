
/*
 * SYSALERTER (c) 2004,2007 Emiel Kollof <emiel@ninth-circle-alliance.net>
 * 
 * Door: Emiel Kollof <emiel@ninth-circle-alliance.net>
 * 
 * $Id: database.c,v 1.1.1.1 2008/05/14 07:19:43 root Exp $
 */

#include "sysalert.h"

void 
database_init(void)
{
#ifdef DB_TYPE_SQLITE
	sqlite_init();
#endif
	return;
}

int
database_insert_stat(char *type, char *value, int status)
{
	int ret;
#ifdef DB_TYPE_SQLITE
	ret = sqlite_insert_stat(type, value, status);
#endif
	return ret;
}

node           *
database_fetch_status(char *status)
{
#ifdef DB_TYPE_SQLITE
	node           *result = NULL;

	numresults = 0;

	if (!strcmp(status, "all")) {
		result = sqlite_return_rows(db,
				 "select * from status order by lastalert;");
	} else {
		result = sqlite_return_rows(db,
				 "select * from status where status='%s' order by lastalert;", status);
	}
	return result;
#endif
}


void
database_clear_status(char *status)
{
#ifdef DB_TYPE_SQLITE
	node           *result = NULL;

	numresults = 0;

	if (!strcmp(status, "all")) {
		result = sqlite_return_rows(db,
				   "delete from status;", status);
		mylist_destroy(result);

	} else {
		result = sqlite_return_rows(db,
				   "delete from status where status='%s';", status);
		mylist_destroy(result);
	}
	return;
#endif
}


void
database_vacuum(void)
{
#ifdef DB_TYPE_SQLITE
	int             ret;
	char           *dberr;
	sqlite3_open(dbdir, &db);
	ret = sqlite3_exec(db, "vacuum;", NULL, NULL, &dberr);
	if (ret != SQLITE_OK) {
		vbprintf("database_vacuum: ERROR: %s\n", dberr);
	}
	sqlite3_close(db);
	return;
#endif
}
