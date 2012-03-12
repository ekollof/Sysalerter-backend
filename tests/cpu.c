#include <stdio.h>
#include <kstat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/sysinfo.h>

#define MAXCPUS 256

typedef unsigned long long counter_t;

struct cpu_state {
	counter_t       user;
	counter_t       syst;
	counter_t       idle;
	counter_t       iowait;
	counter_t       swap;
	counter_t       total;
	time_t          systime;
};

struct cpu_state_pct {
	float           user;
	float           syst;
	float           idle;
	float           iowait;
	float           swap;
	float           total;
};

kid_t           kid;
kstat_ctl_t    *kc;
kstat_t        *ksp[MAXCPUS];
kstat_t        *ksp_chain;
int             numcpu;
struct cpu_state cputhen[MAXCPUS], cpunow[MAXCPUS], cpudiff[MAXCPUS];

void 
chain_update(kstat_ctl_t * kc)
{
	kid = kstat_chain_update(kc);
	if (kid < 0) {
		perror("kstat_chain_update");
		exit(0);
	}
}

int 
num_cpus(void)
{
	int             cpu;

	for (cpu = 0, ksp_chain = kc->kc_chain;
	     (numcpu < MAXCPUS) && (ksp_chain != NULL);
	     ksp_chain = ksp_chain->ks_next) {
		if (strncmp(ksp_chain->ks_module, "cpu_stat", 8) == 0) {
			ksp[cpu++] = ksp_chain;
		}
	}
	return cpu;
}

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

	cpudata->systime = now;

	return cpudata;
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
	if (cpudiff == NULL) {
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
	to->systime = from->systime;
	return;
}

int 
main(int argc, char **argv)
{
	int             cpu, error = 0;
	int             uninited = 1;
	time_t          now;


	kc = kstat_open();

	if (kc == NULL) {
		perror("kstat_open");
	}
	chain_update(kc);
	numcpu = num_cpus();


	printf("I counted %d active cpus\n", numcpu);


	while (!error) {
		printf("\033[2J");	/* clear screen */
		printf("\033[1;1H");	/* move cursor to upper left */

		time(&now);
		printf("CPU DATA GATHERING\n"
		       "-------------------\n%s\n", ctime(&now));

		if (uninited)
			printf("Sampling...\n");

		for (cpu = 0; cpu < numcpu; cpu++) {
			struct cpu_state *buf;
			struct cpu_state_pct *cpupct;


			buf = get_cpu_data(ksp[cpu]);
			if (buf == NULL) {
				error++;
			}
			memcpy(&cpunow[cpu], buf, sizeof(struct cpu_state));

			free(buf);

			if (cpunow[cpu].total == 0) {
				printf("cpunow[%d].total == %d", cpu, cpunow[cpu].total);
				exit(-1);
			}
#if 0

			printf("CPU: %d\tidle: %d user: %d syst: %d wait: %d swap: %d total: %d\n",
			       cpu, (int) cpunow[cpu].idle, (int) cpunow[cpu].user, (int) cpunow[cpu].syst,
			       (int) cpunow[cpu].iowait, (int) cpunow[cpu].swap, (int) cpunow[cpu].total);
#endif

			buf = get_cpu_diff(&cpunow[cpu], &cputhen[cpu]);
			if (buf == NULL) {
				error++;
			}
			memcpy(&cpudiff[cpu], buf, sizeof(struct cpu_state));
			free(buf);

#if 0
			if (!uninited) {
				printf("DIFF: %d\tidle: %d user: %d syst: %d wait: %d swap: %d total: %d init %d\n",
				       cpu, (int) cpudiff[cpu].idle, (int) cpudiff[cpu].user, (int) cpudiff[cpu].syst,
				       (int) cpudiff[cpu].iowait, (int) cpudiff[cpu].swap, (int) cpudiff[cpu].total, uninited);
			}
#endif

			cpupct = get_cpu_pct(&cpudiff[cpu]);

			if (!uninited) {
				printf("CPU[%d]:\tidle: %6.2f %% user: %6.2f %% syst: %6.2f %% wait: %6.2f %% swap: %6.2f %%\n",
				       cpu, cpupct->idle, cpupct->user, cpupct->syst, cpupct->iowait, cpupct->swap);
			}
		}
		memcpy(&cputhen, &cpunow, sizeof(cputhen));
		printf("\n");
		sleep(5);
		if (uninited)
			uninited--;
	}

	return 0;		/* NOTREACHED */
}
