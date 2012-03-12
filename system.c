
#include "sysalert.h"

size_t
freedisk(char *path)
{
	struct statvfs  st;
	size_t          freespace;
	int             error;

	error = statvfs(path, &st);
	if (error < 0) {
		fprintf(stderr, "statvfs: %s: %s", path, strerror(errno));
	}
	freespace = st.f_bsize * st.f_bavail;

	dbprintf("freespace %s KB: %d", path,
		 (size_t) (freespace / 1024));

	return (size_t) (freespace / 1024);
}

size_t
totaldisk(char *path)
{
	struct statvfs  st;
	size_t          totalspace;
	int             error;

	error = statvfs(path, &st);
	if (error < 0) {
		fprintf(stderr, "statvfs: %s: %s", path, strerror(errno));
	}
	totalspace = st.f_bsize * st.f_blocks;

	dbprintf("totalspace %s KB: %d", path,
		 (size_t) (totalspace / 1024));

	return (size_t) (totalspace / 1024);
}
