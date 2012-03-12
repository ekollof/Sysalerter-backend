#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <utmp.h>
#include <utmpx.h>
#include <pwd.h>
#include <lastlog.h>
#include <stdlib.h>

#ifdef __SunOS__
#define LASTLOG "/var/adm/lastlog"
#else 
#define LASTLOG "/var/log/lastlog"
#endif

int main(int argc, char **argv)
{
	struct passwd *pw;
	struct lastlog ll;
	off_t offset;
	FILE *lfp;
	time_t now,delta;
	char chtime[27];

	lfp = fopen(LASTLOG, "rb");
	if (!lfp) {
		perror(LASTLOG);
		exit(0);
	}	

	time(&now);

	while(pw = getpwent()) {
		offset = pw->pw_uid * sizeof(struct lastlog);
		fseek(lfp, offset, SEEK_SET);
		if (fread ((char *) &ll, sizeof(ll), 1, lfp) != 1)
			continue;
		if (ll.ll_time == 0) {
			continue;
		}
		delta = now - ll.ll_time;		
		if (delta == now) {
			delta = 0;
		}
		strncpy(chtime, ctime(&ll.ll_time), sizeof(chtime));
		printf("%s : %d %s", pw->pw_name, delta, delta ? chtime : "never\n");
	
	}
	exit(0);
}
