
#include "sysalert.h"

#ifdef SOLARIS

/**
 * Fills a process struct with proces info from arg pid. Solaris-specific.
 * Allocates memory, be sure to free it when done.
 */
struct process *
fillprocess(pid_t pid)
{
	FILE           *fp;
	psinfo_t        psi;
	char            procfile[MAXPATHLEN];
	struct process *retproc;
	int             proclen, i;

	retproc = (struct process *) malloc(sizeof(struct process));
	if (retproc == NULL) {
		perror("solaris.c:fillprocess: malloc");
		exit(1);
	}
	memset(retproc, 0, sizeof(struct process));

	/* we know this anyway */
	retproc->ppid = pid;

	snprintf(procfile, sizeof(procfile) - 1,
		 "/proc/%d/psinfo", pid);
	fp = fopen(procfile, "rb");
	if (fp == NULL) {
		free(retproc);	/* Memleak otherwise */
		return NULL;
	}
	fread(&psi, sizeof(psinfo_t), 1, fp);
	fclose(fp);

	/* put info in process structure */
	retproc->procsize = psi.pr_size;
	strlcat(retproc->pargv, psi.pr_psargs, sizeof(retproc->pargv));

	return retproc;
}

/**
 * Gets cpu usage statistics from kstat. Not portable.
 */
struct cpu_state *
get_cpu_data(kstat_t * kcpu)
{
	cpu_stat_t      cs;
	struct cpu_state *cpudata;
	time_t          now;

	if (kstat_read(kc, kcpu, &cs) == -1) {
		perror("kstat_read");
		return NULL;
	}
	time(&now);

	cpudata = (struct cpu_state *) malloc(sizeof(struct cpu_state));
	if (cpudata == NULL) {
		perror("malloc");
	}
	cpudata->idle = cs.cpu_sysinfo.cpu[CPU_IDLE];
	cpudata->total = cpudata->idle;

	cpudata->user = cs.cpu_sysinfo.cpu[CPU_USER];
	cpudata->total += cpudata->user;

	cpudata->syst = cs.cpu_sysinfo.cpu[CPU_KERNEL];
	cpudata->total += cpudata->syst;

	cpudata->iowait = cs.cpu_sysinfo.wait[W_IO] + cs.cpu_sysinfo.wait[W_PIO];
	cpudata->total += cpudata->iowait;

	cpudata->swap = cs.cpu_sysinfo.wait[W_SWAP];
	cpudata->total += cpudata->swap;

	return cpudata;
}

int 
num_cpus(void)
{
	int             cpu;

	for (cpu = 0, ksp_chain = kc->kc_chain;
	     (cpu < MAXCPUS) && (ksp_chain != NULL);
	     ksp_chain = ksp_chain->ks_next) {
		if (strncmp(ksp_chain->ks_module, "cpu_stat", 8) == 0) {
			cpuksp[cpu++] = ksp_chain;
		}
	}
	return cpu;
}

#endif
