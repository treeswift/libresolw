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
#include <iphlpapi.h> // adapter list
#include <algorithm> // std::min
#include <string>

#include "dns.h"

namespace resolw_impl {

void set_last_error(int last_error) {
#ifdef ERRNO_IS_LVALUE
    errno = last_error; // POSIX way
#else
    _set_errno(last_error); // cannonical native Windows way
#endif
}

constexpr unsigned int RESOLW_UTF8 = 65001; // CP_UTF8 per <winnls.h>

// ROADMAP reuse
std::wstring to_win_str(const char* posix_str, std::size_t in_len, bool einval_if_empty) {
    if(!in_len) {
        if(einval_if_empty) {
            set_last_error(EINVAL);
        }
        return {};
    }
    std::wstring w_str(in_len, L'\0'); // a conservative estimate
    w_str.resize(MultiByteToWideChar(RESOLW_UTF8, 0 /* flags */, posix_str, in_len, &w_str[0], in_len));
    if(w_str.empty()) {
        set_last_error(EINVAL);
    }
    return w_str;
}

ULONG to_query_opts(u_long rs_options, ImplPolicies * pol_out) {
    ImplPolicies pol = {};

    ULONG qo = DNS_QUERY_STANDARD;
    if(rs_options & RES_AAONLY)
        qo |= DNS_QUERY_BYPASS_CACHE; // near equivalent
    if(rs_options & RES_USEVC)
        qo |= DNS_QUERY_USE_TCP_ONLY; // strict equivalent
    if(rs_options & RES_PRIMARY)
        pol.custom = pol.primary = true; // not implemented; the plan is to inject nsaddr as custom server.
    if(rs_options & RES_IGNTC)
        qo |= DNS_QUERY_ACCEPT_TRUNCATED_RESPONSE; // strict equivalent
    if(!(rs_options & RES_RECURSE))
        qo |= DNS_QUERY_NO_RECURSION; // strict inverse equivalent
    if(rs_options & RES_DEFNAMES)
        qo |= DNS_QUERY_NO_LOCAL_NAME; // near equivalent
    if(!(rs_options & RES_DNSRCH))
        qo |= DNS_QUERY_TREAT_AS_FQDN; // strict equivalent
    if(rs_options & RES_NOALIASES)
        qo |= DNS_QUERY_NO_HOSTS_FILE; // near equivalent
    if(rs_options & RES_ROTATE)
        pol.custom = pol.rotate = true; // not implemented; the plan is to shuffle default-configured servers.
    if(rs_options & RES_KEEPTSIG)
        qo |= DNS_QUERY_RETURN_MESSAGE;
    pol.recurse = rs_options & RES_RECURSE; // for reserialization

    if(pol_out) { *pol_out = pol; }
    return qo;
}

// ROADMAP return basic diagnostics
void resolw_nprep(res_state rs, const wchar_t* hostname, unsigned int rdclass, unsigned int rdtype, ULONG qo,
        DNS_ADDR_ARRAY* nsaddrs, DNS_QUERY_REQUEST* req) {
    req->Version = 1;
    req->QueryName = hostname;
    req->QueryType = rdtype;
    req->QueryOptions = qo;
    req->pDnsServerList = nsaddrs;
    // now take RES_PRIMARY and RES_ROTATE into account
    int rotation_index = 0;
    int n_server_count = rs->nscount;
    if(rs->options & RES_ROTATE) {
        char& robin = rs->unused[0];
        if(++robin >= rs->nscount) {
            robin = 0;
        }
        rotation_index = robin;
    }
    if(rs->options & RES_PRIMARY) {
        n_server_count = 1;
    }
    nsaddrs->AddrCount = n_server_count;
    nsaddrs->Family = AF_INET;
    for(int i = 0; i < n_server_count; ++i, ++rotation_index) {
        sockaddr_in * sa = reinterpret_cast<sockaddr_in*>(nsaddrs->AddrArray[i].MaxSa);
        (*sa) = rs->nsaddr_list[rotation_index % rs->nscount];
    }

    req->InterfaceIndex = 0; // all interfaces
}

} // resolw_impl

namespace {

using namespace resolw_impl;

constexpr int kErrNotImp = -1;

constexpr int kRetranSec = 5; // retransmission time
constexpr int kRetryCount = 3; // retry count

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

// Correspondence:
// rq_class is one of DNS_CLASS_*
// type is one of DNS_TYPE_*
// DNS_MESSAGE_BUFFER* is an interpretation of `u_char* buf`
// - DnsWriteQuestionToBuffer writes query to it;
// - DnsExtractRecordsFromMessage extracts resource records
// DNS_QUERY_RESULT is result of DnsQueryEx
// DNS_RECORD is result of DnsQuery (backward-compatible)
// DnsQueryConfig reads systemwide cfg for res_init [free memory w/LocalFree]:
// - DnsConfigDnsServerList
// - DnsConfigPrimaryDomainName_UTF8
// DNS_BYTE_FLIP_HEADER_COUNTS is used to flip byte ordering for DnsExtractRecordsFromMessage_UTF8
// DnsGetApplicationSettings retrieves servers for res_init (free w/ DnsFreeCustomServers)
// DnsSetApplicationSettings configures per-app settings (e.g. DNS_APP_SETTINGS_EXCLUSIVE_SERVERS)
// res_mkquery is equivalent to DnsWriteQuestionToBuffer_UTF8

int res_init(void)
{
    // special case -- implicit res_ninit called on first access to _res
    return _res, 0;
}

// DnsQuery expects an FQDN
// DNS_QUERY_STANDARD => use the resolver cache and repeat request


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

int res_querydomain(const char *name, const char *domain, int rq_class, int type, u_char *answer, int anslen)
{
    return res_nquerydomain(_resolw_res_state(), name, domain, rq_class, type, answer, anslen);
}

int res_ninit(res_state rs)
{
    memset(rs, 0, sizeof(_res_state));
    rs->options = RES_INIT | RES_DEFAULT;
    rs->retry = kRetryCount;
    rs->retrans = kRetranSec;
    rs->id = res_randomid();
    // fill ns_count, ns_addr_list from DnsQueryConfig:
    IP4_ARRAY* buffer = nullptr;
    DWORD buf_len = sizeof(&buffer); // size of pointer
    if(!DnsQueryConfig(DnsConfigDnsServerList, 0 /* DNS_CONFIG_FLAG_ALLOC */, nullptr, nullptr, &buffer, &buf_len) && buffer) {
        rs->nscount = std::min((DWORD) MAXNS, buffer->AddrCount);
        for(int nsi = 0; nsi < rs->nscount; ++nsi) {
            sockaddr_in &sin = rs->nsaddr_list[nsi];
            sin.sin_family = AF_INET;
            INLINE_HTONL(sin.sin_addr.S_un.S_addr, buffer->AddrArray[nsi]);
            sin.sin_port = NAMESERVER_PORT;
        }
        LocalFree(buffer);
    }
    buf_len = MAXDNAME;
    if(DnsQueryConfig(DnsConfigPrimaryDomainName_UTF8, 0, nullptr, nullptr, &rs->defdname, &buf_len)) {
        // nonzero result => failure
        rs->defdname[0] = '\0';
    }
    rs->ndots = 1; // one dot qualifies for a suffixless query
    // ROADMAP fill in `dnsrch` for user inspection (WinDNS will use its own search list anyway)
    return 0;
}

int res_nsearch(res_state rs, const char *dname, int rq_class, int type, u_char *answer, int anslen)
{
    set_last_error(0); // errno, not h_errno
    // TODO
    return kErrNotImp;
}

int res_nquery(res_state rs, const char *dname, int rq_class, int type, u_char *answer, int anslen)
{
    // We decide here whether https://learn.microsoft.com/en-us/windows/win32/api/windns/nf-windns-dnsquery_utf8
    // is good enough or we need DnsQueryEx (+DNS_QUERY_REQUEST3 rather than DNS_QUERY_REQUEST) for extra tuning.
    // Note, however, that DNS_QUERY_REQUEST3 is only available since Windows build 22000 (Win11; extremely new).
    // Let's calculate a summary of flags here to know for sure.

    // Note: https://learn.microsoft.com/en-us/windows/win32/api/windns/nf-windns-dnssetapplicationsettings is stateful.

    if(!(rs->options & RES_INIT)) { res_ninit(rs); } // see comment to RES_INIT
    // TODO check for Windows version (mappers pre-Win8; since Win8 all or nearly all documented flags are supported)
    ImplPolicies pol;
    ULONG qo = to_query_opts(rs->options, &pol);
    rs->id = res_randomid();

    // MOREINFO the documentation is not definitive regarding the use of search lists. Does DnsQuery_*() append the suffix?
    // simple path. valid if: (!need_custom_servers && (rq_class==C_IN))
    PDNS_RECORD record;
    auto result = DnsQuery_UTF8(dname, type, qo, nullptr, &record, nullptr);
    if(DNS_ERROR_RCODE_NO_ERROR == result) {
        // success; now, unfortunately, re-serialize the response.
        // first, write the original query into the output buffer:
        // res_nmkquery(rs, QUERY, dname, rq_class, type, )
        DWORD buf_sz = anslen;
        DNS_MESSAGE_BUFFER * mb = reinterpret_cast<DNS_MESSAGE_BUFFER*>(answer);
        if(DnsWriteQuestionToBuffer_UTF8(mb, &buf_sz, const_cast<char*>(dname), type, rs->id, pol.recurse)) {
            // MOREINFO we assume numeric fields to be in host byte order. is that true???
            // nevertheless, now append records; validate with DnsExtractRecordsFromMessage_UTF8
            mb->MessageHead.IsResponse = true;
            mb->MessageHead.ResponseCode = NOERROR;
            // response record count:
            mb->MessageHead.AnswerCount = 0; // if wType == type; as a special case,
            // A counts for AAAA and vice versa if DNS_QUERY_DUAL_ADDR, but we don't support it
            mb->MessageHead.NameServerCount = 0; // if DNS_SOA_DATA
            mb->MessageHead.AdditionalCount = 0; // everything else
            auto rcursor = record;
            while(rcursor) {
                // TODO
            }
            // TODO
            return kErrNotImp;
        }
    }
    return -1;
}

int res_nquerydomain(res_state rs, const char *name, const char *rawdom, int rq_class, int type, u_char *answer, int anslen)
{
    // same as nquery, but concatenates name and domain
    std::string domain;
    std::string concat;
    if(!rawdom || !*rawdom) {
         rawdom = name;
    } else if(name && *name) {
        domain = rawdom;
        concat = name;
        if(concat.back() != '.' && domain.front() != '.') {
            concat.push_back('.');
        }
        concat.append(domain);
        rawdom = concat.c_str();
    }
    return res_nquery(rs, rawdom, rq_class, type, answer, anslen);
}

int res_nmkquery(res_state rs, int op, const char *dname, int rq_class, int type, const u_char *data, 
                int datalen, const u_char *newrr, u_char *buf, int buflen)
{
    // TODO
    return kErrNotImp;
}

int res_nsend(res_state rs, const u_char *msg, int msglen, u_char *answer, int anslen)
{
    // TODO
    return kErrNotImp;
}

/* __END_DECLS */
#ifdef __cplusplus
}
#endif
