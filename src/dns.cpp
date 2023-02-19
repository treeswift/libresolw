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

#include "resolv.h"
#include <errno.h>
#include <windns.h>

namespace {

struct ResState : public _res_state {
    ResState() { res_ninit(this); }
};

static thread_local ResState _resolw_state;

} // anonymous

/* __BEGIN_DECLS */
#ifdef __cplusplus
extern "C" {
#endif

res_state _resolw_res_state() { return &_resolw_state; }

int res_init(void)
{
    // special case -- implicit res_ninit called on first access to _res
    return _res, 0;
}

int res_search(const char *dname, int rq_class, int type, u_char *answer, int anslen)
{
    return res_nsearch(_resolw_res_state(), dname, rq_class, type, answer, anslen);
}

int res_query(const char *dname, int rq_class, int type, u_char *answer, int anslen)
{
    return res_nquery(_resolw_res_state(), dname, rq_class, type, answer, anslen);
}

int res_mkquery(int op, const char *dname, int rq_class, int type, const u_char *data, 
                int datalen, const u_char *newrr, u_char *buf, int buflen)
{
    return res_nmkquery(_resolw_res_state(), op, dname, rq_class, type, data, datalen, newrr, buf, buflen);
}

int res_send(const u_char *msg, int msglen, u_char *answer, int anslen)
{
    return res_nsend(_resolw_res_state(), msg, msglen, answer, anslen);
}

int res_ninit(res_state rs)
{
    memset(rs, 0, sizeof(_res_state));
    rs->options |= DNS_RRSET_INIT;
    return 0;
}

int res_nsearch(res_state rs, const char *dname, int rq_class, int type, u_char *answer, int anslen)
{
    // TODO
    return 0;
}

int res_nquery(res_state rs, const char *dname, int rq_class, int type, u_char *answer, int anslen)
{
    // TODO
    return 0;
}

int res_nmkquery(res_state rs, int op, const char *dname, int rq_class, int type, const u_char *data, 
                int datalen, const u_char *newrr, u_char *buf, int buflen)
{
    // TODO
    return 0;
}

int res_nsend(res_state rs, const u_char *msg, int msglen, u_char *answer, int anslen)
{
    // TODO
    return 0;
}

/* __END_DECLS */
#ifdef __cplusplus
}
#endif
