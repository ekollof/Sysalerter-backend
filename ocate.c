
/*
 * Copyright (c) 2004, Emiel Kollof <coolvibe@hackerheaven.org> All rights
 * reserved.
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
 * Created by Emiel Kollof on Fri May  7 16:15:05 CEST 2004. Copyright (c) 2004
 * All rights reserved.
 *
 * $Id: ocate.c,v 1.1 2004/05/09 13:15:51 coolvibe Exp $
 */

#define _GNU_SOURCE

#include "ocate.h"
#include "sysalert.h"

/* XXX: I consider this a hack. */
#ifdef SOLARIS
#define gethostbyname2(host, family) gethostbyname(host)
#endif


    char           *
ntoip(struct in_addr * addr)
{
    char           *buf = NULL;

    inet_ntop(AF_INET, &addr, buf, 256);
    if (buf == NULL) {
        perror("inet_pton");
        return NULL;
    }
    return buf;
}

#ifdef WANT_IPV6
    char           *
ntoip6(struct in6_addr * addr)
{
    char           *buf = NULL;

    inet_ntop(AF_INET6, &addr, buf, 256);
    if (buf == NULL) {
        perror("inet_pton");
        return NULL;
    }
    return buf;
}
#endif

    struct in_addr *
ipton(char *host)
{
    struct in_addr *in;

    in = (struct in_addr *) malloc(sizeof(struct in_addr));

    inet_pton(AF_INET, host, in);

    return (struct in_addr *) in;
}

#ifdef WANT_IPV6
    struct in6_addr *
ip6ton(char *host)
{
    struct in6_addr *in;

    in = (struct in6_addr *) malloc(sizeof(struct in6_addr));

    inet_pton(AF_INET6, host, in);

    return (struct in6_addr *) in;
}
#endif

    char           *
iptohost(char *ipnum)
{
    struct hostent *hent;
    struct in_addr  in;
    char           *buf;
    int             ret;

    memcpy(&in, ipton(ipnum), sizeof(in));
    hent = gethostbyaddr((const char *) &in, sizeof(in), AF_INET);

    if (hent == NULL) {
        herror("iptohost");
        return NULL;
    }
    ret = asprintf(&buf, "%s", hent->h_name);
    if (ret < 0) {
        perror("ocate.c: iptohost: asprintf error");
        return NULL;
    }
    return buf;
}

#ifdef WANT_IPV6
    char           *
ip6tohost(char *ipnum)
{
    struct hostent *hent;
    struct in6_addr in;
    char           *buf;

    memcpy(&in, ip6ton(ipnum), sizeof(in));
    hent = gethostbyaddr((const char *) &in, sizeof(in), AF_INET6);

    if (hent == NULL) {
        herror("ip6tohost");
        return NULL;
    }
    asprintf(&buf, "%s", hent->h_name);

    return buf;
}
#endif

    char           *
htoip(char *host)
{
    struct sockaddr_in sap;
    struct hostent *hent;
    char           *buf;
    int             ret;

    hent = gethostbyname(host);

    if (hent == NULL) {
        herror("htoip");
        return NULL;
    }
    sap.sin_addr = *(struct in_addr *) hent->h_addr_list[0];

    ret = asprintf(&buf, "%s", inet_ntoa(sap.sin_addr));
    if (ret < 0) {
        perror("ocate.c: htoip: asprintf error");
        return NULL;
    }
    return buf;
}

#ifdef WANT_IPV6
    char           *
htoip6(char *host)
{
    struct sockaddr_in6 sap;
    struct hostent *hent;
    char           *buf = NULL;

    hent = gethostbyname2(host, AF_INET6);
    if (hent == NULL) {
        herror("htoip6");
        return NULL;
    }
    sap.sin6_addr = *(struct in6_addr *) hent->h_addr_list[0];

    /* better too much than too little, eh? */
    buf = (char *) malloc(256);
    if (buf == NULL) {
        perror("malloc");
        return NULL;
    }
    inet_ntop(AF_INET6, &sap.sin6_addr, buf, 255);

    if (buf == NULL) {
        perror("inet_ntop");
        return NULL;
    }
    return buf;
}
#endif
