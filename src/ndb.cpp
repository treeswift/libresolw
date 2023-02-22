#include <netdb.h>
#include <stdlib.h>
#include <limits>
#include <vector>
#include "dns.h"

namespace {

using namespace resolw_impl;

int resolw_parserrs(struct rrsetinfo ** res, const char* hostname, unsigned int rdclass, unsigned int rdtype, unsigned int options, DNS_RECORDA* recs) {
    struct rrsetinfo retval = {};
    retval.rri_rdclass = rdclass;
    retval.rri_rdtype = rdtype;
    retval.rri_ttl = std::numeric_limits<int>::max();
    if(!(options & (RES_INSECURE1 | RES_INSECURE2))) {
        retval.rri_flags |= RRSET_VALIDATED;
    }
    std::string cname = hostname;
    DNS_RECORD* dnsrec = recs;
    // TODO refactor and de-duplicate
    std::size_t datasz = 0u;
    std::size_t sigssz = 0u;
    while(dnsrec) {
        if(dnsrec->wType == rdtype) {
            // answer
            retval.rri_nrdatas++;
            retval.rri_ttl = std::min(retval.rri_ttl, (unsigned int) dnsrec->dwTtl);
            datasz += dnsrec->wDataLength;
        }
        if(dnsrec->wType == T_RRSIG) {
            // signature
            retval.rri_nsigs++;
            sigssz += dnsrec->wDataLength;
        }
        if(dnsrec->wType == T_CNAME) {
            cname.assign(dnsrec->Data.CNAME.pNameHost, dnsrec->Data.CNAME.pNameHost + dnsrec->wDataLength);
        }
        dnsrec = dnsrec->pNext;
    }
    // TODO handle ERRSET_NODATA here

    std::size_t metasz = retval.rri_nrdatas * sizeof(struct rdatainfo);
    std::size_t msigsz = retval.rri_nsigs * sizeof(struct rdatainfo);
    unsigned char* data_block = static_cast<unsigned char*>(malloc(datasz + metasz));
    unsigned char* sigs_block = static_cast<unsigned char*>(malloc(sigssz + msigsz));
    retval.rri_rdatas = reinterpret_cast<struct rdatainfo *>(data_block);
    retval.rri_sigs = reinterpret_cast<struct rdatainfo *>(sigs_block);
    dnsrec = recs;
    data_block += metasz;
    sigs_block += msigsz;
    std::size_t di = 0u, si = 0u;
    while(dnsrec) {
        if(dnsrec->wType == rdtype) {
            struct rdatainfo &rd = retval.rri_sigs[di++];
            rd.rdi_data = data_block;
            rd.rdi_length = dnsrec->wDataLength;
            memcpy(rd.rdi_data, &dnsrec->Data, rd.rdi_length);
            data_block += rd.rdi_length;
        }
        if(dnsrec->wType == T_RRSIG) {
            struct rdatainfo &rd = retval.rri_sigs[si++];
            rd.rdi_data = sigs_block;
            rd.rdi_length = dnsrec->wDataLength;
            memcpy(rd.rdi_data, &dnsrec->Data, rd.rdi_length);
            sigs_block += rd.rdi_length;
        }
        // MOREINFO how is pDataPtr treated?
        dnsrec = dnsrec->pNext;
    }
    // free incoming data
    DnsRecordListFree(recs, DnsFreeRecordList);
    // copy, transfer ownership:
    char* with_cname = static_cast<char*>(malloc(sizeof(struct rrsetinfo) + cname.size() + 1));
    *res = reinterpret_cast<struct rrsetinfo*>(with_cname);
    memcpy(with_cname + sizeof(struct rrsetinfo), cname.c_str(), cname.size() + 1);
    (**res) = retval;
    return ERRSET_SUCCESS;
}

} // anonymous

/* __BEGIN_DECLS */
#ifdef __cplusplus
extern "C" {
#endif

int getrrsetbyname(const char *hostname, unsigned int rdclass, unsigned int rdtype, unsigned int flags, struct rrsetinfo **res)
{
    if(flags) return ERRSET_INVAL;
    if(!hostname || !*hostname) return ERRSET_INVAL;

    ImplPolicies pol;
    auto opts = _res.options;
    ULONG qo = to_query_opts(opts, &pol);
    qo |= DNS_QUERY_RETURN_MESSAGE;
#ifdef DNS_ADDR_MAX_SOCKADDR_LENGTH // newer Win8+ API
    if(IsWindows8OrGreater()) {
        std::wstring wdname = to_win_str(hostname, strlen(hostname));
        constexpr std::size_t maxBytes = sizeof(DNS_ADDR_ARRAY) + MAXNS * sizeof(DNS_ADDR);
        std::vector<char> nsbuf(maxBytes, 0);
        DNS_ADDR_ARRAY* nsadd = reinterpret_cast<DNS_ADDR_ARRAY*>(nsbuf.data());
        nsadd->MaxCount = maxBytes;
        DNS_QUERY_REQUEST req;
        DNS_QUERY_RESULT resp;
        resolw_nprep(&_res, wdname.c_str(), rdclass, rdtype, qo, nsadd, &req);
        if(ERROR_SUCCESS == DnsQueryEx(&req, &resp, nullptr)) {
            return resolw_parserrs(res, hostname, rdclass, rdtype, opts, resp.pQueryRecords);
        }
    } else
#endif
    {
        // fall back to DnsQuery_UTF8
        DNS_RECORDA* recs = nullptr;
        constexpr std::size_t maxBytes = sizeof(IP4_ARRAY) + (MAXNS - 1) * sizeof(IP4_ADDRESS);
        std::vector<char> nsbuf(maxBytes, 0);
        IP4_ARRAY* srvs = reinterpret_cast<IP4_ARRAY*>(nsbuf.data());
        if(DnsQuery_UTF8(hostname, rdtype, qo, srvs, &recs, nullptr)) {
            return resolw_parserrs(res, hostname, rdclass, rdtype, opts, recs);
        }
    }

    // TODO handle ERRSET_NONAME
    return ERRSET_FAIL;
}

void freerrset(struct rrsetinfo *rrset)
{
    if(rrset) {
        if(rrset->rri_nrdatas && rrset->rri_rdatas) {
            free(rrset->rri_rdatas);
        }
        if(rrset->rri_nsigs && rrset->rri_sigs) {
            free(rrset->rri_sigs);
        }
        free(rrset);
    }
}

/* __END_DECLS */
#ifdef __cplusplus
}
#endif