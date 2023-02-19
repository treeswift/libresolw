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

#ifndef _NETDB_H_
#define _NETDB_H_

/**
 * This header file specifies selected functions and function prototypes
 * present in ISC BIND v9 API and documented in OpenBSD Library Functions
 * Manual, e.g. at: https://man.openbsd.org/OpenBSD-7.2/getrrsetbyname.3
 * The contents of this header file is the bare minimum required to build
 * programs dependent on the documented API. libresolw implementation of
 * this API is public domain without strings attached; see `src/ndb.cpp`.
 */

#include <errno.h>

/* __BEGIN_DECLS */
#ifdef __cplusplus
extern "C" {
#endif

struct rdatainfo {
    unsigned int rdi_length;
    unsigned char *rdi_data;
};

enum {
    /**
     * From the documentation: "CAVEATS: The RRSET_VALIDATED flag in rri_flags is
     * set if the AD (authenticated data) bit in the DNS answer is set. This flag
     * should not be trusted unless the transport between the nameserver and the
     * resolver is secure (e.g. IPsec, trusted network, loopback communication)."
     */
    RRSET_VALIDATED = true,
};

struct rrsetinfo {
    unsigned int rri_flags;
    unsigned int rri_rdclass;
    unsigned int rri_rdtype;
    unsigned int rri_ttl;
    unsigned int rri_nrdatas;
    unsigned int rri_nsigs;
    char *rri_name;
    struct rdatainfo *rri_rdatas;
    struct rdatainfo *rri_sigs;
};

enum {
    ERRSET_SUCCESS  = 0,
    ERRSET_NOMEMORY = ENOMEM,
    ERRSET_FAIL     = EAGAIN,
    ERRSET_INVAL    = EINVAL,
    ERRSET_NONAME   = ENOENT,
    ERRSET_NODATA   = ENODATA,
};

int getrrsetbyname(const char *hostname, unsigned int rdclass, unsigned int rdtype, unsigned int flags, struct rrsetinfo **res);

void freerrset(struct rrsetinfo *rrset);

/* __END_DECLS */
#ifdef __cplusplus
}
#endif

#endif /* _NETDB_H_ */
