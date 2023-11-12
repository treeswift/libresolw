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

#ifndef _NAMESER_H_
#define _NAMESER_H_

#include <windns.h>

#define __BIND 19960801     /* BIND API level */

#define NAMESERVER_PORT	53  /* well-known DNS protocol port */

/* RFC 833 definitions follow: */
#define MAXDNAME 1025   /* maximum unpacked (expanded) domain name size */
#define MAXCDNAME 255   /* maximum compressed domain name size */
#define MAXLABEL  63    /* maximum domain label length */
#define HFIXEDSZ  12    /* fixed data bytes in header */
#define QFIXEDSZ  4     /* fixed data bytes in query */
#define RRFIXEDSZ 10    /* fixed data bytes in r record */

/* Request opcodes: */
enum {
    QUERY = DNS_OPCODE_QUERY,
    IQUERY = DNS_OPCODE_IQUERY,
    STATUS = DNS_OPCODE_SERVER_STATUS,
    UNKNOWN = DNS_OPCODE_UNKNOWN,
    NOTIFY = DNS_OPCODE_NOTIFY,
    UPDATE = DNS_OPCODE_UPDATE,
};

/* Response opcodes: */
enum {
    NOERROR = DNS_RCODE_NOERROR,
    FORMERR = DNS_RCODE_FORMERR,
    SERVFAIL = DNS_RCODE_SERVFAIL,
    NXDOMAIN = DNS_RCODE_NXDOMAIN,
    NOTIMPL = DNS_RCODE_NOTIMPL,
    REFUSED = DNS_RCODE_REFUSED,
    YXDOMAIN = DNS_RCODE_YXDOMAIN,
    YXRRSET = DNS_RCODE_YXRRSET,
    NXRRSET = DNS_RCODE_NXRRSET,
    NOTAUTH = DNS_RCODE_NOTAUTH,
    NOTZONE = DNS_RCODE_NOTZONE,
    // MAX = DNS_RCODE_MAX 15
    BADVERS = DNS_RCODE_BADVERS,
    BADSIG = DNS_RCODE_BADSIG,
    BADKEY = DNS_RCODE_BADKEY,
    BADTIME = DNS_RCODE_BADTIME,
    NO_ERROR = DNS_RCODE_NOERROR,
    FORMAT_ERROR = DNS_RCODE_FORMAT_ERROR,
    SERVER_FAILURE = DNS_RCODE_SERVER_FAILURE,
    NAME_ERROR = DNS_RCODE_NAME_ERROR,
    NOT_IMPLEMENTED = DNS_RCODE_NOT_IMPLEMENTED,
};

/* DNS record types: */
enum {
    T_ZERO = DNS_TYPE_ZERO,
    T_A = DNS_TYPE_A,
    T_NS = DNS_TYPE_NS,
    T_MD = DNS_TYPE_MD,
    T_MF = DNS_TYPE_MF,
    T_CNAME = DNS_TYPE_CNAME,
    T_SOA = DNS_TYPE_SOA,
    T_MB = DNS_TYPE_MB,
    T_MG = DNS_TYPE_MG,
    T_MR = DNS_TYPE_MR,
    T_NULL = DNS_TYPE_NULL,
    T_WKS = DNS_TYPE_WKS,
    T_PTR = DNS_TYPE_PTR,
    T_HINFO = DNS_TYPE_HINFO,
    T_MINFO = DNS_TYPE_MINFO,
    T_MX = DNS_TYPE_MX,
    T_TEXT = DNS_TYPE_TEXT,
    T_RP = DNS_TYPE_RP,
    T_AFSDB = DNS_TYPE_AFSDB,
    T_X25 = DNS_TYPE_X25,
    T_ISDN = DNS_TYPE_ISDN,
    T_RT = DNS_TYPE_RT,
    T_NSAP = DNS_TYPE_NSAP,
    T_NSAPPTR = DNS_TYPE_NSAPPTR,
    T_SIG = DNS_TYPE_SIG,
    T_KEY = DNS_TYPE_KEY,
    T_PX = DNS_TYPE_PX,
    T_GPOS = DNS_TYPE_GPOS,
    T_AAAA = DNS_TYPE_AAAA,
    T_LOC = DNS_TYPE_LOC,
    T_NXT = DNS_TYPE_NXT,
    T_EID = DNS_TYPE_EID,
    T_NIMLOC = DNS_TYPE_NIMLOC,
    T_SRV = DNS_TYPE_SRV,
    T_ATMA = DNS_TYPE_ATMA,
    T_NAPTR = DNS_TYPE_NAPTR,
    T_KX = DNS_TYPE_KX,
    T_CERT = DNS_TYPE_CERT,
    T_A6 = DNS_TYPE_A6,
    T_DNAME = DNS_TYPE_DNAME,
    T_SINK = DNS_TYPE_SINK,
    T_OPT = DNS_TYPE_OPT,
    T_DS = DNS_TYPE_DS,
    T_RRSIG = DNS_TYPE_RRSIG,
    T_NSEC = DNS_TYPE_NSEC,
    T_DNSKEY = DNS_TYPE_DNSKEY,
    T_DHCID = DNS_TYPE_DHCID,
    T_NSEC3 = DNS_TYPE_NSEC3,
    T_NSEC3PARAM = DNS_TYPE_NSEC3PARAM,
    T_TLSA = DNS_TYPE_TLSA,
    T_UINFO = DNS_TYPE_UINFO,
    T_UID = DNS_TYPE_UID,
    T_GID = DNS_TYPE_GID,
    T_UNSPEC = DNS_TYPE_UNSPEC,
    T_ADDRS = DNS_TYPE_ADDRS,
    T_TKEY = DNS_TYPE_TKEY,
    T_TSIG = DNS_TYPE_TSIG,
    T_IXFR = DNS_TYPE_IXFR,
    T_AXFR = DNS_TYPE_AXFR,
    T_MAILB = DNS_TYPE_MAILB,
    T_MAILA = DNS_TYPE_MAILA,
    T_ALL = DNS_TYPE_ALL,
    T_ANY = DNS_TYPE_ANY,
    T_WINS = DNS_TYPE_WINS,
    T_WINSR = DNS_TYPE_WINSR,
    T_NBSTAT = DNS_TYPE_NBSTAT
};

enum {
    C_IN = DNS_CLASS_INTERNET,
    C_CSNET = DNS_CLASS_CSNET,
    C_CHAOS = DNS_CLASS_CHAOS,
    C_HS = DNS_CLASS_HESIOD,
    C_NONE = DNS_CLASS_NONE,
    C_ALL = DNS_CLASS_ALL,
    C_ANY = DNS_CLASS_ANY,
    C_UNICAST_RESPONSE = DNS_CLASS_UNICAST_RESPONSE,
};

// Constants
// Keyflags
// Signatures
// Algorithms

enum {
    DNS_MESSAGEEXTFLAG_DO = 0x8000,
};

typedef DNS_HEADER HEADER;

enum {
    CONV_SUCCESS = 0,
    CONV_OVERFLOW = -1,
    CONV_BADFMT = -2,
    CONV_BADCKSUM = -3,
    CONV_BADBUFLEN = -4,
};

/* Handling compressed domain names: enum { INDIR_MASK = 0xc0, }; */

#endif /* _NAMESER_H_ */
