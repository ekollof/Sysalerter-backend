/*
 * Copyright (c) 2003, Emiel Kollof <emiel@ninth-circle-alliance.net> All
 * rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution. Neither the name of
 * the <ORGANIZATION> nor the names of its contributors may be used to
 * endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Created by Emiel Kollof on Tue Nov 25 2003. Copyright (c) 2003 All rights
 * reserved.
 *
 * $Id: mail.c,v 1.1.1.1 2008/05/14 07:19:43 root Exp $
 */

int             indata;

#include "sysalert.h"

    int
mail_sendmessage(char *to, char *from, char *subject, char *message)
{
    /**
     * TODO: Must test with different MTA's other than postfix and Sendmail
     *       because those two tend to work. Can't say anything about freaks
     *       of nature like qmail, exim and others ;)
     */

    int             mail_fd, ret;
#ifndef DONTFORK
    int mpid;
#endif
    char            myhostname[MAXHOSTNAMELEN];
    char            buf[2048];
    char           *fullmsg;


    /* fork so if this gets a broken pipe, we're still running */

#ifndef DONTFORK
    mpid=fork();

    switch(mpid) {
        case 0:         /* child */
#endif
            setproctitle("sysalert: sending mail to %s via %s", to, mail_server);
            indata = 0;
            gethostname(myhostname, sizeof(myhostname));

            mail_fd = network_connect(mail_server, "25");
            if (mail_fd < 0) {
                dbprintf("mail_sendmessage: could not connect to mailserver.\n");
                return -1;
            }
            snprintf(buf, sizeof(buf) - 1, "EHLO %s\r\n", myhostname);
            mail_sendcmd(mail_fd, buf);

            snprintf(buf, sizeof(buf) - 1, "MAIL FROM:<%s@%s>\r\n", from,
                    myhostname);
            mail_sendcmd(mail_fd, buf);

            snprintf(buf, sizeof(buf) - 1, "RCPT TO:<%s>\r\n", to);
            mail_sendcmd(mail_fd, buf);

            snprintf(buf, sizeof(buf) - 1, "DATA\r\n");
            mail_sendcmd(mail_fd, buf);

            indata = 1;
            ret = asprintf(&fullmsg, "From: \"System Alerter\" <%s>\r\n"
                    "To: %s\r\n"
                    "Subject: %s\r\n"
                    "X-Mailer: System Alerter by Emiel Kollof\r\n"
                    "\r\n"
                    "%s\r\n"
                    ".\r\n",
                    from,
                    to,
                    subject,
                    message);
            mail_sendcmd(mail_fd, fullmsg);
            if (ret < 0) {
                perror("mail.c: mail_sendmessage: asprintf error");
                exit(-1);
            }
            free(fullmsg);
            indata = 0;

            snprintf(buf, sizeof(buf) - 1, "QUIT\r\n");
            mail_sendcmd(mail_fd, buf);
            vbprintf("mail_sendmessage: Mail sent to %s\n", mail_rcpt);
#ifndef DONTFORK
            exit(0);
            break;      /* NOTREACHED */
        case -1:        /* error */
            perror("mail_sendmessage: fork");
            break;
        default:        /* parent */
            dbprintf("Forked off mail client with pid %d\n", mpid);
            waitpid(mpid, NULL, 0);
            break;
    }
#endif
    return 0;
}

    int
mail_sendcmd(int mail_fd, char *data)
{
    char            buf[8192], junk[8192];
    int             statuscode;

    nprintf(mail_fd, "%s", data);

    while (network_gets(mail_fd, buf, sizeof(buf) - 1)) {
        chomp(buf);
        dbprintf("mail_sendcmd: <<< %s\n", buf);
        if (rx_match(buf, "^[0-9]{3} .*$")) {   /* numbers with a space
                                                 * instead of a - */
            /* we're done */
            break;
        }
    }
    dbprintf("mail_sendcmd: >>> %s", data);

    if (!indata) {
        sscanf(buf, "%d %8191s", &statuscode, junk);

        if (statuscode > 200 && statuscode < 400) {
            dbprintf("mail_sendcmd: Command ok\n");
            return statuscode;
        }
        dbprintf("mail_sendcmd: command failed: \n", data);
        return -1;
    } else {
        return 0;
    }
}
