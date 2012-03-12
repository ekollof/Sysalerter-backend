#include <stdio.h>
#include <sys/types.h>
#include <sys/mnttab.h>
#include <sys/statvfs.h>
#include <string.h>

#define MAXMNTSIZE 128
#define MEGABYTE (1024 * 1024)

char           *virtfs[] = {
	"ctfs",
	"lofs",
	"tmpfs",
	"hsfs",
	"fd",
	"objfs",
	"autofs",
	"nfs",
	"procfs",
	"proc",
	"devfs",
	"dev",
	"mntfs",
	"sharefs",
	NULL
};

int
checkvirtfs(char *type)
{
	char          **ptr;

	ptr = virtfs;
	while (*ptr != NULL) {
		if (!strncmp(*ptr, type, MAXMNTSIZE)) {
			return 1;
		}
		ptr++;
	}
	return 0;
}

int
main(int ac, char **av)
{
	struct mnttab   mt;
	struct statvfs  st;
	char           *mtab = "/etc/mnttab";
	FILE           *fp;
	int             ret;
	double          freesz;
	const int       megabyte = 1024 * 1024;

	/* open mtab file */
	fp = fopen(mtab, "r");
	if (!fp) {
		perror("fopen");
		return 1;
	}
	while ((ret = getmntent(fp, &mt)) == 0) {
		if (checkvirtfs(mt.mnt_fstype)) {
			continue;
		}
		/* Get stats */
		ret = statvfs(mt.mnt_mountp, &st);
		if (ret < 0) {
			perror("statvfs");
		}
		if (st.f_blocks != 0) {
			int             maxsize, cursize;

			cursize = (st.f_bfree * st.f_frsize) / MEGABYTE;
			maxsize = (st.f_blocks * st.f_frsize) / MEGABYTE;
			/* freesz = (100 * st.f_bfree * 1.0 )  / st.f_blocks; */
			freesz = (cursize / maxsize) * 100.0;
			printf("disk: test %s: (%d / %d ) * 100.0 = %lf %% free\n", mt.mnt_mountp, cursize, maxsize, freesz);
		}
		printf("%.02f %% free on %s on %s wih type %s\n",
		       freesz,
		       mt.mnt_special,
		       mt.mnt_mountp,
		       mt.mnt_fstype);
	}
}
