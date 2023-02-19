/**
 * This file has no copyright assigned and is placed in the public domain
 * according to the terms of the Unlicense License: https://unlicense.org
 * The entirety of it or any part of it may be used by anyone and for any
 * purpose, commercial or noncommercial, with or without attribution.
 *
 * This file is part of the libresolw compatibility library:
 *   https://github.com/treeswift/libresolw
 *
 * The complete libresolw library reuses a substantial amount of code from
 * the OpenBSD project and is therefore distributed under the 3-clause BSD
 * license. Refer to the LICENSE file in the project root.
 */
#ifndef _RESOLV_H_
#define _RESOLV_H_

#include "_bsd_types.h"
#include "sys/socket.h"
#include "arpa/nameser.h"
#include <stdint.h>

/* arbitrary constants governing failover */
#define	MAXNS	10
#define	MAXDNSRCH	10
#define	MAXRESOLVSORT	10

/* constants governing fixed string sizes */
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX MAX_PATH
#endif
#ifndef MAXDNAME
#define MAXDNAME HOST_NAME_MAX
#endif

struct _res_state {
    int retrans; /* retransmission time interval */
    int retry; /* number of times to retransmit */
    u_long options; /* option flags - see below. */
    int nscount; /* number of name servers */
    struct sockaddr_in nsaddr_list[MAXNS]; /* address of name server */
#define nsaddr nsaddr_list[0] /* for backward compatibility */
    u_short id; /* current packet id */
    char *dnsrch[MAXDNSRCH+1]; /* components of domain to search */
    char defdname[MAXDNAME]; /* default domain */
    u_long pfcode; /* RES_PRF_ flags - see below. */
    unsigned ndots:4; /* threshold for initial abs. query */
    unsigned nsort:4; /* number of elements in sort_list[] */
    char unused[3];
    struct {
        struct in_addr addr; /* address to sort on */
        uint32_t mask;
    } sort_list[MAXRESOLVSORT]; 
};

/* __BEGIN_DECLS */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _res_state *res_state;
extern res_state _resolw_res_state();

/* now use a macro similar to errno: */
#define _res (*_resolw_res_state())

int res_init(void);
int res_search(const char *dname, int rq_class, int type, u_char *answer, int anslen);
int res_query(const char *dname, int rq_class, int type, u_char *answer, int anslen);
int res_mkquery(int op, const char *dname, int rq_class, int type, const u_char *data, 
                int datalen, const u_char *newrr, u_char *buf, int buflen);
int res_send(const u_char *msg, int msglen, u_char *answer, int anslen);

/**
 * The following stateless equivalents of the above methods are specified in Bind 8.2.2
 * and available in multiple *xes such as GNU/Linux and Solaris.
 */
int res_ninit(res_state rs);
int res_nsearch(res_state rs, const char *dname, int rq_class, int type, u_char *answer, int anslen);
int res_nquery(res_state rs, const char *dname, int rq_class, int type, u_char *answer, int anslen);
int res_nmkquery(res_state rs, int op, const char *dname, int rq_class, int type, const u_char *data, 
                int datalen, const u_char *newrr, u_char *buf, int buflen);
int res_nsend(res_state rs, const u_char *msg, int msglen, u_char *answer, int anslen);

#ifdef __BSD_VISIBLE
/**
 * `dn_{expand|comp|...}()` methods are present in BSD distributions
 *  and map to `ns_name_[un]compress()` methods in `nameser.h` API.
 *  OpenSSH, for instance, expects `dn_expand()` to exist.
 * `dn_skipname()` is public API in FreeBSD libc.
 */
int dn_expand(const u_char *msg, const u_char *eomorig, const u_char *comp_dn, char *exp_dn, int length);
int dn_comp(const char *exp_dn, u_char *comp_dn, int length, u_char **dnptrs, u_char **lastdnptr);
int dn_skipname(const u_char *comp_dn, const u_char *eom);

/* a few more DN parsing//validation methods common in BSD distros: */
int res_dnok(const char *dn);
int res_hnok(const char *dn);
int res_ownok(const char *dn);
int res_mailok(const char *dn);

#endif /* __BSD_VISIBLE */

/* bonus/start dust */
#ifndef res_randomid
#define res_randomid resolw_randomid
#endif
int resolw_randomid(void);

/**
 * WinSock2: h_errno expands to WSAGetLastError()
 * h.*error() is implemented with FormatMessage()
 */
void herror(const char *s);

/* __END_DECLS */
#ifdef __cplusplus
}
#endif

#endif /* _RESOLV_H_ */