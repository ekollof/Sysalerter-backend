
#include "sysalert.h"

#ifdef LINUX


    struct process *
fillprocess(pid_t pid)
{
    struct process *pr;
    char *argv;

    pr = (struct process *) malloc(sizeof(struct process));
    if (!pr) {
        perror("fillprocess(linux):malloc");
        return NULL;
    }
    argv = getargv(pid,1024);
    strlcpy(pr->pargv, argv, 1024);

    /* now get the process size from /proc/%d/status */
    pr->procsize = getpidsize(pid);

    return pr;
}

    size_t
getpidsize(pid_t pid)
{

    char *buf;
    FILE *fp;
    char line[1024];
    size_t size = 0;

    if (asprintf(&buf, "/proc/%d/status", pid) < 0) {
        perror("getpidsize:asprintf");
        return 0;
    }

    fp = fopen(buf, "r");
    if (!fp) {
        perror("getpidsize:fopen");
    }

    while(fgets(line, sizeof(line) -1, fp)) {
        chomp(line);
        if (rx_match(line, "^VmSize:")) {
            char *tmp;
            tmp = split(line, " ");
            trim(tmp);
            printf("GOT: %s\n",tmp);
            size = atoi(tmp);
            break;
        }
    }
    fclose(fp);

    return size;
}

    char *
getargv(pid_t pid, int alen)
{
    char *buf;
    char cmd[1024];
    int len, fd, idx;

    if(asprintf(&buf, "/proc/%d/cmdline", pid) < 0) {
        perror("getargv:asprintf");
        return NULL;
    }

    fd = open(buf, O_RDONLY);

    if (fd < 0) {
        perror("getargv: open");
        return NULL;
    }


    len = read(fd, cmd, sizeof(cmd) -1);
    if (len < 0) {
        perror("getargv:read");
        return NULL;
    }
    close(fd);
    free(buf);

    /* replace \0 with space */
    for (idx=0; idx < len; idx++) {
        if (cmd[idx] == '\0') {
            cmd[idx] = ' ';
        }
    }

    cmd[len] = '\0';
    if (asprintf(&buf, "%s", cmd) < 0) {
        perror("getargv:asprintf");
        return NULL;
    }


    return buf;
}

    int
num_cpus(void)
{
    int cpu=0;
    char buf[80];
    FILE *fp;

    fp = fopen("/proc/cpuinfo", "r");
    if (!fp) {
        perror("num_cpus(linux): perror");
        return 0;
    }

    while(fgets(buf, sizeof(buf) -1, fp)) {
        if (rx_match(buf, "^processor\t:")) {
            cpu++;
        }
    }
    return cpu;
}

    struct cpu_state *
get_cpu_data(void)
{
    FILE *fp;
    struct cpu_state *cpudata;
    //int ticks = 0;
    int idle, user, system, iowait, junk, swap;
    char line[1024];
    char cjunk[255];


    idle = user = system = iowait = swap = 0;

    //ticks = sysconf(_SC_CLK_TCK);

    cpudata = (struct cpu_state *) malloc(sizeof(struct cpu_state));
    if (!cpudata) {
        perror("malloc");
        return NULL;
    }

    fp = fopen("/proc/stat", "r");
    if (!fp) {
        perror("couldn't open /proc/stat, is /proc mounted?");
        return NULL;
    }
    while(fgets(line, sizeof(line) -1, fp)) {
        if (rx_match(line, "^cpu ")) {
            sscanf(line, "%254s %d %d %d %d %d", cjunk, &user, &junk, &system, &idle, &iowait);
            dbprintf("/proc/stat: %s", line);
            break;
        }
    }

    cpudata->idle = idle;
    cpudata->total = cpudata->idle;

    cpudata->user = user;
    cpudata->total += cpudata->user;

    cpudata->syst = system;
    cpudata->total += cpudata->syst;

    cpudata->iowait = iowait;
    cpudata->total += cpudata->iowait;

    cpudata->swap = swap;
    cpudata->total += cpudata->swap;

    return cpudata;
}

#endif
