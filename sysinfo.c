/* TODO: put license stuff here */

#include "sysalert.h"

#ifdef HAS_PROC

int
getnumprocs(void)
{
	unsigned int    counter, number, pidnum;
	struct dirent **d_entry;

#if 0
	struct process *sp;
#endif

	counter = scandir("/proc/", &d_entry, 0, alphasort);
	if (counter < 0) {
		perror("sysinfo.c:getnumprocs:scandir");
		return 0;
	}
	number = 0;

	while (counter--) {
		pidnum = atoi(d_entry[counter]->d_name);

		if (pidnum > 0) {
#if 0
			sp = (struct process *) fillprocess(pidnum);
			dbprintf("pid %d, name \"%s\" (%d), size %d\n",
				 pidnum,
				 sp->pargv,
				 strlen(sp->pargv),
				 sp->procsize);
			free(sp);
#endif
			number++;
		}
		free(d_entry[counter]);
	}
	return number;
}

struct process *
getprocbyname(char *name)
{
	unsigned int    counter, pidnum;
	struct dirent **d_entry;
	struct process *sp;

	counter = scandir("/proc/", &d_entry, 0, alphasort);
	if (counter < 0) {
		perror("sysinfo.c:getprocbyname:scandir");
		return (0);
	}
	while (counter--) {
		pidnum = atoi(d_entry[counter]->d_name);

		if (pidnum > 0) {
			sp = fillprocess(pidnum);
			if (rx_match(sp->pargv, name)) {
				return sp;
			}
			free(sp);
		}
	}
	return NULL;
}

struct process *
getprocbypid(pid_t pid)
{
	struct process *sp;

	sp = fillprocess(pid);
	return sp;
}


#endif

char           *
getlocation(void)
{
	FILE           *fp;
	char           *line;
	fp = fopen(location, "r");
	if (!fp) {
		return NULL;
	}
	if ((line = (char *) malloc(1024)) == NULL) {
		perror("sysinfo.c:getlocation:malloc");
		exit(1);
	}
	while (fgets(line,1023,fp)) {

		if (*line == '#' || *line == '\n')	/* skip comments and
							 * whitespace */
			continue;
		else {
			chomp(line);
			dbprintf("sysinfo.c:getlocation: Got %s\n", line);
			return line;	/* Only take first line, ignore
					 * others. */
		}
	}
	return NULL;		/* NOTREACHED */
}

int
setlocation(char *place)
{
	FILE           *fp;
	time_t          now;

	fp = fopen(location, "w");
	if (!fp) {
		return 0;
	}
	fprintf(fp, "# System location file, generated by sysalerter.\n"
		"# This file is automatically generated, but feel free to edit it\n");
	time(&now);
	fprintf(fp, "%s\n"
		"Last Changed: %s",
		place, ctime(&now));
	fflush(fp);
	fclose(fp);
	return 1;
}

struct cpu_state *
get_cpu_diff(struct cpu_state * now, struct cpu_state * then)
{
	struct cpu_state *cpudiff;

	cpudiff = (struct cpu_state *) malloc(sizeof(struct cpu_state));
	if (cpudiff == NULL) {
		perror("malloc");
	}
	cpudiff->idle = now->idle - then->idle;
	cpudiff->user = now->user - then->user;
	cpudiff->syst = now->syst - then->syst;
	cpudiff->iowait = now->iowait - then->iowait;
	cpudiff->swap = now->swap - then->swap;
	cpudiff->total = now->total - then->total;

	return cpudiff;
}


struct cpu_state_pct *
get_cpu_pct(struct cpu_state * state)
{
	struct cpu_state_pct *cpupct;

	cpupct = (struct cpu_state_pct *) malloc(sizeof(struct cpu_state_pct));
	if (cpupct == NULL) {
		perror("malloc");
	}
	cpupct->idle = ((float) state->idle / (float) state->total) * 100;
	cpupct->user = ((float) state->user / (float) state->total) * 100;
	cpupct->syst = ((float) state->syst / (float) state->total) * 100;
	cpupct->iowait = ((float) state->iowait / (float) state->total) * 100;
	cpupct->swap = ((float) state->swap / (float) state->total) * 100;
	cpupct->total = ((float) state->total / (float) state->total) * 100;

	return cpupct;
}

void
copy_state_values(struct cpu_state * to, struct cpu_state * from)
{
	to->idle = from->idle;
	to->user = from->user;
	to->syst = from->syst;
	to->iowait = from->iowait;
	to->swap = from->swap;
	to->total = from->total;
	return;
}

void
add_state_values(struct cpu_state * to, struct cpu_state * from)
{
	to->idle += from->idle;
	to->user += from->user;
	to->syst += from->syst;
	to->iowait += from->iowait;
	to->swap += from->swap;
	to->total += from->total;
	return;
}

int get_cpu_load(void)
{
		return 0;
}

