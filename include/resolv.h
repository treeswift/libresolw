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

/* arbitrary constant governing "wholesale" queries */
#define	MAXDNSRCH       16

/* arbitrary constants governing failover */
#define	MAXNS           10
#define	MAXRESOLVSORT   3

/* semi-arbitrary constants governing fixed string sizes */
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX MAX_PATH
#endif
#ifndef MAXDNAME
#define MAXDNAME HOST_NAME_MAX
#endif

struct _res_state {
    int retrans; /* interval between query attempts (unit=second) */
    int retry; /* number of query attempts */
    u_long options; /* option flags; default is RES_INIT | RES_DEFAULT */
    int nscount; /* number of configured name servers */
    struct sockaddr_in nsaddr_list[MAXNS]; /* array of configured name servers, per DnsConfigDnsServerList */
#define nsaddr nsaddr_list[0] /* primary/first-chance configured name server */
    u_short id; /* packet id. may or may not be real, as we don't have access to WinDNS internals */
    char *dnsrch[MAXDNSRCH+1]; /* components of domain to search */
    char defdname[MAXDNAME]; /* default domain, per DnsConfigPrimaryDomainName_UTF8 */
    u_long pfcode; /* RES_PRF_ flags (currently ignored) */
    unsigned ndots:4; /* number of dots expected in fqdn and qualifying it for a suffixless query */
    unsigned nsort:4; /* number of elements in the preferred network list; unused and ignored */
    char unused[3]; /* unused[0] is (ab)used for name server rotation */
    struct {
        struct in_addr addr; /* preferred subnet address */
        uint32_t mask; /* preferred subnet mask */
    } sort_list[MAXRESOLVSORT];
};

/**
 * SOME of the resolver options (not all) have DNS_QUERY_* equivalents.
 * However, in absence of 1:1 correspondence and the possibility that
 * certain client programs expect certain numeric flag values, we do
 * the mapping in `dns.cpp` at run time rather than hardcode it here.
 * For each individual flag it's safer to assume it's not implemented
 * than to assume that it is.
 */
enum {
    RES_INIT    = 1 << 0, /* Clear this flag to trigger implicit `res_ninit()`. Options will be reset to RES_DEFAULT. */
    RES_DEBUG   = 1 << 1, /* Ad-hoc WIP, but no systemic effort at implementation. Feel free to contribute yours. */
    RES_AAONLY  = 1 << 2, /* DNS_QUERY_BYPASS_CACHE (kind of); see also ..._WIRE_ONLY, !DNS_QUERY_{CACHE_ONLY,NO_WIRE_QUERY} */
    RES_USEVC   = 1 << 3, /* DNS_QUERY_USE_TCP_ONLY */
    RES_PRIMARY = 1 << 4, /* no equivalent flag, but setting primary server as single custom server is possible */
    RES_IGNTC   = 1 << 5, /* DNS_QUERY_ACCEPT_TRUNCATED_RESPONSE */
    RES_RECURSE = 1 << 6, /* !DNS_QUERY_NO_RECURSION; also fRecursionDesired in DnsWriteQuestionToBuffer */
    RES_DEFNAMES= 1 << 7, /* DNS_QUERY_TREAT_AS_FQDN wb an overkill, but then we only have DNS_QUERY_NO_LOCAL_NAME */
    RES_STAYOPEN= 1 << 8,
    RES_DNSRCH  = 1 << 9, /* !DNS_QUERY_TREAT_AS_FQDN */
    RES_INSECURE1=1 << 10,
    RES_INSECURE2=1 << 11,
    RES_NOALIASES=1 << 12, /* no equivalent option, but can be used to toggle DNS_QUERY_NO_HOSTS_FILE */
    RES_USE_INET6=1 << 13,
    RES_ROTATE  = 1 << 14, /* has to be implemented logically via custom servers */
    RES_NOCHECKNAME=1<<15,
    RES_KEEPTSIG= 1 << 16, /* implemented with DNS_QUERY_RETURN_MESSAGE */
    RES_BLAST   = 1 << 17,
    RES_DEFAULT = RES_RECURSE | RES_DEFNAMES | RES_DNSRCH,
};

/* __BEGIN_DECLS */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _res_state *res_state;
extern res_state _resolw_res_state();

/* _res is a macro similar to errno: */
#define _res (*_resolw_res_state())

/**
 * Legacy Bind 8.2 resolver API, still widely used. The implied state in `libresolw` is thread local.
 */

/* Prepares the resolver state structure. */
int res_init(void);

/* Qualifies the domain name, applies aliases, and calls res_query() on each FQDN generated. */
int res_search(const char *dname, int rq_class, int type, u_char *answer, int anslen);

/* Concatenates name and domain into a fully qualified domain name, then executes res_query(). */
int res_querydomain(const char *name, const char *domain, int rq_class, int type, u_char *answer, int anslen);

/* The "workhorse" function. Treats dname as is, but implements retrial internally. */
int res_query(const char *dname, int rq_class, int type, u_char *answer, int anslen);

/* Creates the query message, but does not send it as of yet. */
int res_mkquery(int op, const char *dname, int rq_class, int type, const u_char *data, 
                int datalen, const u_char *newrr, u_char *buf, int buflen);

/* Sends the query message. */
int res_send(const u_char *msg, int msglen, u_char *answer, int anslen);

/**
 * The following stateless equivalents of the above methods are specified in Bind 8.2.2.
 */
int res_ninit(res_state rs);
int res_nsearch(res_state rs, const char *dname, int rq_class, int type, u_char *answer, int anslen);
int res_nquerydomain(res_state rs, const char *name, const char *domain, int rq_class, int type,
                    u_char *answer, int anslen);
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