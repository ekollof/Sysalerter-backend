/*
 * MONITORD (c) 2004,2008 Emiel Kollof <emiel@ninth-circle-alliance.net>
 * 
 * Door: Emiel Kollof
 * 
 */

#include "sysalert.h"


time_t users_lastlog(int uid)
{
	struct lastlog ll;
	off_t offset;
	FILE *lfp;

	lfp = fopen(LASTLOG, "rb");
	if (!lfp) {
		dbprintf("users_lastlog: Couldn't open %s: %s\n", LASTLOG, strerror(errno));
		return 0;
	}
	
	offset = uid * sizeof(struct lastlog);
	fseek(lfp, offset, SEEK_SET);
	if (fread((char *) &ll, sizeof(struct lastlog), 1, lfp) != 1) 
		return 0;
	fclose(lfp);	
	if (!ll.ll_time) {
		return 0;
	}
	return ll.ll_time;
}

