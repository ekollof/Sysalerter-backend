/*
 * SYSALERTER (c) 2004,2007 Emiel Kollof <emiel@ninth-circle-alliance.net>
 * 
 * Door: Emiel Kollof <emiel@ninth-circle-alliance.net>
 * 
 * $Id: vars.c,v 1.1.1 2008/05/14 07:19:43 root Exp $
 */

#ifndef lint
static const char copyright[] =
"Copyright (c) 2004 Emiel Kollof (http://www.ninth-circle-alliance.net)";
#endif

#include "sysalert.h"

/* niet-extern versies van globale vars/consts */

char            configfile[MAXPATHLEN] = "sysalertrc";
int             nodaemon = 0;	/* 0 == false, init done like this to shut up
				 				 * sunpro compiler */
int             verbose = 1;
int             debug = 1;
int             alert_once = 0;
char            dbdir[MAXPATHLEN] = "/tmp/sysalert.db";
char		logfile[MAXPATHLEN] = "/var/log/sysalert.log";
char            location[MAXPATHLEN] = "/etc/syslocationrc";
int             logging = 0;

struct cpu_state cputhen, cpunow, cpudiff;

#ifdef SOLARIS
kstat_ctl_t    *kc;
kstat_t        *cpuksp[MAXCPUS];
kstat_t        *ksp_chain;
kid_t		kid;
#endif

#ifdef DB_TYPE_SQLITE
sqlite3        *db = NULL;
char          **dbresult;
unsigned int    numresults;
#endif

#ifdef SOLARIS
char            load_shellcommand[1024] = "prstat -c 1 1";
char            disk_shellcommand[1024] = "df -kha";
char            cpu_shellcommand[1024] = "vmstat";
#endif
#ifdef LINUX
char            load_shellcommand[1024] = "top -c -n 1";
char            disk_shellcommand[1024] = "df -kha";
char            cpu_shellcommand[1024] = "vmstat";
#endif
#ifdef FREEBSD
char            load_shellcommand[1024] = "top -a -d 1";
char            disk_shellcommand[1024] = "df -kha";
char            cpu_shellcommand[1024] = "vmstat";
#endif 
#if !defined(SOLARIS) && !defined(LINUX) && !defined(FREEBSD)
#error Not Supported
#endif

char            mail_server[1024] = "mx.ninth-circle-alliance.net";
char            mail_rcpt[1024] = "emiel.kollof@gmail.com";

double          maxload = 0;
int             interval = 120;
char            diskpaths[8192] = "/";
float           disk_minfree = 20.00; /* gigabytes */

char            listenaddr[512] = "all";
int             portnum = 5432;

int		heartbeat = 1;
char		hburl[1024] = "http://sysalert.ninth-circle-alliance.net/submit.php";
int		hbinterval = 60;

struct linked_list *lresult = NULL;
struct linked_list *lresult_last = NULL;
struct linked_list *lresult_start = NULL;

int numcpus = 0;

/* List of 'fake' filesystems. Anything listed here will *not* be
 * checked for space issues to prevent false positives.
 */

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
	"mntfs",
	"dev",
	"sharefs",
	"securityfs",
	"lrm",
	"fusectl",
	"udev",
	"varrun",
	"varlock",
	"sysfs",
	"devpts",
	NULL
};

#ifdef NO_UTENT
char *utmpfil = "/var/run/utmp";	/* default utmp file */
FILE *ufp = NULL;		/* file pointer to utmp file */
					/* NULL = no utmp file open  */
struct utmp ut;			/* buffer for utmp record */
#endif
