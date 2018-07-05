/*
 * SYSALERTER (c) 2004,2007 Emiel Kollof <emiel@ninth-circle-alliance.net>
 *
 * Door: Emiel Kollof <emiel@ninth-circle-alliance.net>
 *
 * $Id: check.c,v 1.1.1.1 2008/05/14 07:19:43 root Exp $
 */


#include "sysalert.h"
#include "ocate.h"

#define GB (1024 * 1024)


struct status   status;
struct status   status_alerted;

    void
check_mainloop(void)
{
    time_t          now = 0, then = 0;
    useconds_t      sleeptime;
    int         delta = 0;

    /* init */
    status_alerted.alert_load = false;
    status_alerted.alert_disk = false;
    status_alerted.alert_cpu = false;
    sleeptime = 100000;

    numcpus = num_cpus();
    http_fetch_url(hburl);

    vbprintf("Found %d cpus\n", numcpus);

    database_init();

    /*
     * XXX: Right here we're just spinning in place. The reason for this
     * is to be able to have different intervals for the checking process
     * (disk/cpu/load), and also for the heartbeat-process, which might
     * check at different intervals. I might separate this out into
     * another file so we have a rudimentary timer-based scheduler that
     * can shoot off different functions at variable intervals.
     */

    time(&then);
    while (1) {
        int sampletrig = 0, hbtrig = 0;

        time(&now);

        delta = (int) now - (int) then;
        sampletrig = delta % interval;
        hbtrig = delta % hbinterval;

        if (!sampletrig) {
            load_check();
            disk_check(diskpaths);
            cpu_check();
            check_alert();
            sleep(1); /* make sure trig status is over */
        }

        if (!hbtrig) {
            http_fetch_url(hburl);
            sleep(1); /* make sure trig status is over */
        }
        usleep(sleeptime);
    }
}

    void
load_check(void)
{
    /*
     * XXX: I know that checking loadavg is probably a bad yardstick to
     * measure system usage with, but it's a good indicator of how busy a
     * system really is, even though it can generate some nasty false
     * positives that are hard to explain. With measuring load avg a
     * system can seem loaded while it in fact is just doing nothing.
     */

    double          currload[3];
    char           *buf;

    getloadavg(currload, 3);

#if 0
    if (nodaemon) {
        fprintf(stderr, "Current load: %6.2f\n", currload[0]);
    }
#endif
    if (maxload == 0) {
        dbprintf("maxload was 0, setting it to %d\n", numcpus);
        maxload = numcpus;
    }

    if (currload[0] > maxload) {
        if (nodaemon) {
            dbprintf("%6.2f > %6.2f : maxload triggered\n",
                    currload[0],
                    maxload);
        }
        status.alert_load = true;
    } else {
        if (status.alert_load) {
            dbprintf("Alert passed, resetting things...\n");
        }
        status.alert_load = false;
        if (alert_once) {   /* reset toggle */
            status_alerted.alert_load = false;
        }
    }
    if ((asprintf(&buf, "%6.2f", currload[0])) < 0) {
        perror("load_check: asprintf");
    }
    if (!database_insert_stat("load", buf, status_alerted.alert_load)) {
        dbprintf("Skipping due to condition\n");
    }

    free(buf);
    return;
}

    void
disk_check(char *paths)
{

    struct statvfs  st;
    char           *line = strdup(paths);   /* make a copy I can
                                             * obliterate */
    char          **pathlist = strsplit(line, ':', '\"');
    char          **tmp;
    char           *buf;
    char          **dir;
    int             ret;
    int             count = 0;
    int             fail = 0;
    int             i;
    float           freesz = 0;
    float           *sizelist = NULL;

    /* first count the amount of things in the pathlist */
    tmp = pathlist;
    for (tmp = pathlist; *tmp; tmp++) {
        count++;
    }

    sizelist = (float *) realloc(sizelist, count);
    if (sizelist == NULL) {
        perror("disk_check: realloc");
    }

    /*
     * XXX: ZFS might break this? Runs into issues with large filesystems
     * > 4 TB, compile as 64-bit as a workaround
     */
    for (dir = pathlist, i = 0; *dir; dir++, i++) {
        ret = statvfs(*dir, &st);
        if (ret < 0) {
            printf("%s: statvfs: %s\n", *dir, strerror(errno));
            sizelist[i] = -1;
            continue;
        }

        if (st.f_blocks != 0) { /* prevent divide by zero */

            freesz = (100 * (float) st.f_bfree) / st.f_blocks;
            sizelist[i] = freesz;
        }
    }

    for (i=0; i < count; i++) {
        if (sizelist[i] < disk_minfree && sizelist[i] > -1) {
            dbprintf("%6.2f < %6.2f : disk minfree hit on %s\n", sizelist[i], disk_minfree, pathlist[i]);
            fail++;
        }
        if (!asprintf(&buf, "%s: %6.2f %%", pathlist[i], sizelist[i])) {
            perror("disk_check:asprintf");
        }
        if (!database_insert_stat("disk", buf, fail>0?1:0)) {
            dbprintf("Skipping due to condition\n");
        }
        free(buf);

    }

    if (fail > 0) {
        status.alert_disk = true;
    } else {
        if (status.alert_disk) {
            dbprintf("Alert passed (disk), resetting status\n");
        }
        status.alert_disk = false;
        if (alert_once) {   /* reset toggle */
            status_alerted.alert_disk = false;
        }
    }


    // free(sizelist);
    free(pathlist);
    free(line);
    return;
}

    void
cpu_check(void)
{
    /*
     * TODO: - Get numcpus check user load on cpus (not sysload, as it
     * includes the idle-loop) report if higher than treshold
     */
    struct cpu_state *buf;
    //struct cpu_state_pct *cpupct;
    //struct cpu_state *total;

#ifdef SOLARIS
    int cpu=0, error=0;

    ks_chain_update(kc);
    for (cpu=0;cpu<numcpus;cpu++) {
        buf = get_cpu_data(cpuksp[cpu]);
        if (buf == NULL)
            error++;

    }
#else
    buf = get_cpu_data();
    dbprintf("CPU: idle: %d user: %d system: %d total %d\n",
            buf->idle,
            buf->user,
            buf->syst,
            buf->total
            );

    free(buf);
#endif

    return;
}

    void
check_alert(void)
{

    if (status.alert_load && !status_alerted.alert_load) {
        dbprintf("alert_load event caught\n");
        status_alerted.alert_load = true;
        check_mail_alert("load", load_shellcommand);
    }
    if (status.alert_disk && !status_alerted.alert_disk) {
        dbprintf("alert_disk event caught\n");
        status_alerted.alert_disk = true;
        check_mail_alert("disk", disk_shellcommand);
    }
    if (status.alert_cpu && !status_alerted.alert_cpu) {
        dbprintf("alert_cpu event caught\n");
        status_alerted.alert_cpu = true;
        check_mail_alert("cpu", cpu_shellcommand);
    }
}

    void
check_mail_alert(char *trigger, char *command)
{
    FILE           *my_stdout;
    char           *message = NULL; /* stop compiler from complaining */
    char            line[1024];
    char           *subject;
    char           *date;
    int             size = 0;
    time_t          now;
    struct utsname  myname;

    time(&now);
    uname(&myname);
    date = (char *) strdup(ctime(&now));
    chomp(date);

    if (asprintf(&subject, "%s event triggered at %s on %s (%s)", trigger, date,
                myname.nodename, htoip(myname.nodename)) < 0) {
        perror("Alloc error...\n");
        return;
    }

    if ((my_stdout = (FILE *) popen(command, "r")) != NULL) {
        message = NULL;
        while (fgets(line, 1024, my_stdout)) {
            size += strlen(line);
            if ((message = (char *) realloc(message,
                            size + 1)) == NULL) {
                perror("realloc");
                return;
            }
            strlcat(message, line, size);
        }

        pclose(my_stdout);

    } else {
        dbprintf("check_mail_alert: something went wrong... %s\n",
                strerror(errno));
    }

    stripjunk(message);
    mail_sendmessage(mail_rcpt, MAILFROM, subject, message);

    free(message);
    free(subject);
    free(date);

    return;
}
