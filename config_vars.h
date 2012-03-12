/*
 * config_vars.h
 */

/*
 * EXPLANATION:
 * 
 * COMPARE_INT(cvar, fvar) COMPARE_BOOL(cvar, fvar) COMPARE_DOUBLE(cvar, fvar)
 * COMPARE_STRING(cvar, fvar)
 * 
 * PLEASE: Check what you are doing! Errors in this file means compilation
 * breakages.
 */

/* mail server settings */
COMPARE_STRING(mail_server, "mailserver");
COMPARE_STRING(mail_rcpt, "mailaddress");

/* network stuff */
COMPARE_STRING(listenaddr, "listen_address");
COMPARE_INT(portnum, "listen_port");

/* program flow vars */
COMPARE_STRING(logfile, "logfile");
COMPARE_BOOL(nodaemon, "nodaemon");
COMPARE_BOOL(verbose, "verbose");
COMPARE_BOOL(logging, "logging");
COMPARE_BOOL(debug, "debug");
COMPARE_BOOL(alert_once, "alert_once");
COMPARE_INT(interval, "interval");
COMPARE_STRING(dbdir, "dbdir");
COMPARE_BOOL(heartbeat, "heartbeat");
COMPARE_STRING(hburl, "heartbeat_url");
COMPARE_INT(hbinterval, "heartbeat_interval");

/* other checks */
COMPARE_DOUBLE(maxload, "maxload");
COMPARE_STRING(diskpaths, "disk_paths");
COMPARE_STRING(load_shellcommand, "load_shellcommand");
COMPARE_STRING(disk_shellcommand, "disk_shellcommand");
COMPARE_STRING(cpu_shellcommand, "cpu_shellcommand");
COMPARE_DOUBLE(disk_minfree, "disk_minfree");
