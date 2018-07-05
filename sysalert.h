

#ifndef _LOADCHECK_H
#define _LOADCHECK_H

#ifndef VERSION
#define VERSION "0.01"
#endif

#define MAXMSGSIZE 8192
#define MAILFROM "sysalert"
#define MAXCPUS 256

#if defined(__linux__) || defined(GLIBC) || defined (__CYGWIN__)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <getopt.h>
#endif

#ifdef SOLARIS
#define LASTLOG "/var/adm/lastlog"
#else
#define LASTLOG "/var/log/lastlog"
#endif

/* includes */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/statvfs.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <ctype.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <regex.h>
#include <pwd.h>
#ifdef SOLARIS
#include <sys/loadavg.h>
#include <kstat.h>
#include <sys/proc.h>
#include <sys/param.h>
#include <sys/user.h>
#include <sys/fcntl.h>
#define _STRUCTURED_PROC 1
#include <sys/procfs.h>
#include <sys/sysinfo.h>
#include <sys/swap.h>
#endif
#include <dirent.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include <utmp.h>
#if defined(LINUX) || defined(SOLARIS)
#include <utmpx.h>
#include <lastlog.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>

#if defined(__linux__) || defined(GLIBC)
#include <getopt.h>
#endif

#ifdef DB_TYPE_SQLITE
#include <sqlite3.h>

extern sqlite3 *db;
extern char   **dbresult;
extern unsigned int numresults;
#endif



/* macros */

#define bzero(b,len) memset(b,0,len)

/* function pointers */
typedef void    (*client_hndlr) (int);

/* defines */
#define CONFIGFILE "config"
#define SYSCONFIGDIR "/etc/sysalerter"
#define USRCONFIGDIR ".sysalerter"

#define true 1;
#define false 0;




/* prototypes */
struct status {
    int             alert_load;
    int             alert_disk;
    int             alert_cpu;
};

/* linked list stuff because I hate dealing with arrays */
struct linked_list {
    char           *data;
    struct linked_list *next;
    struct linked_list *prev;
}               item;

typedef struct linked_list node;
typedef unsigned long long counter_t;

/* process structure */
struct process {
    pid_t           ppid;
    char            pargv[1024];
    size_t          procsize;
};


struct cpu_state {
    counter_t       user;
    counter_t       syst;
    counter_t       idle;
    counter_t       iowait;
    counter_t       swap;
    counter_t       total;
};

struct cpu_state_pct {
    float           user;
    float           syst;
    float           idle;
    float           iowait;
    float           swap;
    float           total;
};

struct webaddr {
    char        host[255];
    char        proto[10];
    int     port;
    char        path[MAXPATHLEN];
};

struct person {
    char        *name;
    uid_t       uid;
    char        *pty;
    char        *where;
};

extern struct cpu_state cputhen, cpunow, cpudiff;

/* constanten/globals */
extern char    *optarg;
extern int      optind, opterr, optopt;
extern char     configfile[MAXPATHLEN];
extern char logfile[MAXPATHLEN];
extern int      nodaemon;
extern int      verbose;
extern int      debug;
extern int      alert_once;
extern char     dbdir[MAXPATHLEN];
extern char     location[MAXPATHLEN];
extern int      portnum;
extern int      logging;

#ifdef SOLARIS
/* global kstat structure, init in main */
extern kstat_ctl_t *kc;
extern kstat_t        *cpuksp[MAXCPUS];
extern kstat_t        *ksp_chain;
extern kid_t           kid;
#endif

extern char     load_shellcommand[1024];
extern char     disk_shellcommand[1024];
extern char     cpu_shellcommand[1024];
extern char     mail_server[1024];
extern char     mail_rcpt[1024];
extern char     listenaddr[512];

extern double   maxload;
extern int      interval;
extern char     diskpaths[8192];
extern float    disk_minfree;

extern char    *virtfs[];

extern struct linked_list *lresult;
extern struct linked_list *lresult_last;
extern struct linked_list *lresult_start;

extern int heartbeat;
extern char hburl[1024];
extern int hbinterval;

extern int numcpus;

#ifdef NO_UTENT
extern char *utmpfil;
extern FILE *ufp;
extern struct utmp ut;
#endif


/* main.c */
extern void     usage(char *progname);

/* check.c */
extern void     check_mainloop(void);
extern void     load_check(void);
extern void     disk_check(char *path);
extern void     cpu_check(void);
extern void     check_mail_alert(char *trigger, char *command);


/* config.c */
extern char    *config_read(char *config);
extern void     config_feed(char *config);
extern int      config_parse(char *line);
extern void     check_alert(void);

/* utils.c */
extern int      rx_match(const char *string, char *pattern);
extern int      rx_imatch(const char *string, char *pattern);
extern void     stripchar(char *buf, int strip);
extern void     cleading(char *buf);
extern void     ctrailing(char *buf);
extern void     trim(char *buf);
extern void     chomp(char *buf);
extern uid_t    grabuid(char *uname);
extern char    *split(char *string, char *delim);
extern void     stripjunk(char *buf);
extern void     replacenull(char *buf, size_t len);
extern char    *suckfile(FILE * fp);
extern void     dbprintf(char *fmt,...);
extern void     vbprintf(char *fmt,...);
extern void     putlog(char *fmt,...);
extern void     getisodate(char *date);
extern void     renice(pid_t pid, int value);
extern char   **strsplit(char *s, int ch, int qc);
extern void     hexdump(void *mybuff, size_t len);


/* compat.c */
#ifdef NO_ASPRINTF
extern int      asprintf(char **ret, const char *fmt,...);
extern int      vasprintf(char **ret, const char *fmt, va_list ap);
#endif
#ifdef NO_DAEMON
extern int      daemon(int nochdir, int noclose);
#endif
#ifdef NO_NATIVE_STRLCPY
extern size_t   strlcpy(char *dst, const char *src, size_t sz);
extern size_t   strlcat(char *dst, const char *src, size_t sz);
#endif
#ifdef NO_STRDUP
extern char    *strdup(char *buf);
#endif
#ifdef SOLARIS
/* kstat.c */
extern void     ks_init(void);
extern void     ks_chain_update(kstat_ctl_t * kc);
#endif
#ifdef NO_UTENT
extern struct utmp * getutent(void);
extern struct utmp * getutline(struct utmp *line);
extern void setutent(void);
extern void endutent(void);
extern void utmpname(char *file);
#endif
#ifdef NO_SETPROCTITLE
extern void init_setproctitle(int argc, char **argv);
extern void setproctitle(char *fmt, ...);
#endif

/* network.c */
extern void     network_mainloop(client_hndlr clientfunc);
extern int      network_init(struct sockaddr_in * local, socklen_t llen);
extern void
network_setaddress(char *host, char *service,
        struct sockaddr_in * sap);
extern int      network_connect(char *hname, char *sname);
extern int      network_send(int fd, char *buf);
extern int      nprintf(int sock, char *fmt,...);
extern int      network_read(int fd, char *bp, size_t len);
extern int      network_gets(int fd, char *bufptr, size_t len);

/* mail.c */
extern int      mail_sendmessage(char *to, char *from, char *subject, char *message);
extern int      mail_sendcmd(int mail_fd, char *data);

/* talk.c */
extern void     talk_interface(int sock);
extern int      talk_parse(int sock, char *command);

/* solaris.c */
#ifdef SOLARIS
extern struct process *fillprocess(pid_t pid);

extern struct cpu_state * get_cpu_data(kstat_t * kcpu);
extern int num_cpus(void);
#endif

/* linux.c */
#ifdef LINUX
extern struct process *fillprocess(pid_t pid);
extern  size_t getpidsize(pid_t pid);
extern char *getargv(int pid, int alen);
extern struct cpu_state * get_cpu_data(void);
extern int num_cpus(void);
#endif

/* freebsd.c */
#ifdef FREEBSD
extern struct process *fillprocess(pid_t pid);
extern  size_t getpidsize(pid_t pid);
extern char *getargv(int pid, int alen);
extern struct cpu_state * get_cpu_data(void);
extern int num_cpus(void);
#endif

/* sysinfo.c */
extern int      getnumprocs(void);
extern struct process *getprocbyname(char *name);
extern struct process *getprocbypid(pid_t pid);
extern char    *getlocation(void);
extern int      setlocation(char *place);
extern struct cpu_state * get_cpu_diff(struct cpu_state * now, struct cpu_state * then);
extern struct cpu_state_pct * get_cpu_pct(struct cpu_state * state);
extern void copy_state_values(struct cpu_state * to, struct cpu_state * from);


extern void  add_state_values(struct cpu_state * to, struct cpu_state * from);



/* database.c */
extern void     database_init(void);
extern int     database_insert_stat(char *type, char *value, int status);
extern node    *database_fetch_status(char *status);
extern void     database_clear_status(char *status);
extern void     database_vacuum(void);


/* sqlite.c */
extern void     sqlite_init(void);
extern int     sqlite_insert_stat(char *type, char *value, int status);
extern int      sqlite_generic_callback(void *result, int argc, char **argv, char **colname);
extern node    *sqlite_return_rows(sqlite3 *db, char *sql, ...);

/* list.c */
extern node    *mylist_malloc(void);
extern void     mylist_store(node * new, node ** start, node ** last);
extern void     mylist_destroy(node * start);
extern void     mylist_print(node * start);
extern node    *mylist_find(node * start, char *entry);
extern int      mylist_delete(char *entry, node ** start, node ** last);
extern int      mylist_save(node * start, char *filename);
extern int      mylist_load(node * start, node * last, char *filename);
extern void     mylist_additem(node * list, char *data, node ** start, node ** last);

/* http.c */
extern struct webaddr *http_parse_url(char *url);
extern void http_fetch_url(char *url);

/* users.c */
extern time_t users_lastlog(int uid);

#endif
