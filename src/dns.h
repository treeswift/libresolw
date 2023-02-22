#ifndef _SRC_DNS_H_
#define _SRC_DNS_H_

#include "resolv.h"
#include <windows.h>
#include <windns.h>
#include <versionhelpers.h>
#include <string>

// the following definitions are sadly missing in MinGW;
// we complement them according to WinDNS documentation.

#ifndef DNS_ADDR_MAX_SOCKADDR_LENGTH
#define DNS_ADDR_MAX_SOCKADDR_LENGTH 32

/* __BEGIN_DECLS */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _DnsAddr {
  CHAR  MaxSa[DNS_ADDR_MAX_SOCKADDR_LENGTH];
  DWORD DnsAddrUserDword[8];
} DNS_ADDR, *PDNS_ADDR;

typedef struct _DNS_ADDR_ARRAY {
    DWORD    MaxCount;
    DWORD    AddrCount;
    DWORD    Tag;
    WORD     Family;
    WORD     WordReserved;
    DWORD    Flags;
    DWORD    MatchFlag;
    DWORD    Reserved1;
    DWORD    Reserved2;
    DNS_ADDR AddrArray[];
} DNS_ADDR_ARRAY, *PDNS_ADDR_ARRAY;

typedef struct _DNS_QUERY_REQUEST {
    ULONG                         Version;
    PCWSTR                        QueryName;
    WORD                          QueryType;
    ULONG64                       QueryOptions;
    PDNS_ADDR_ARRAY               pDnsServerList;
    ULONG                         InterfaceIndex;
    PDNS_QUERY_COMPLETION_ROUTINE pQueryCompletionCallback;
    PVOID                         pQueryContext;
} DNS_QUERY_REQUEST, *PDNS_QUERY_REQUEST;

typedef LONG DNS_STATUS;

typedef struct _DNS_QUERY_CANCEL {
    CHAR Reserved[32];
} DNS_QUERY_CANCEL, *PDNS_QUERY_CANCEL;

WINAPI DNS_STATUS DnsQueryEx(
  PDNS_QUERY_REQUEST pQueryRequest,
  PDNS_QUERY_RESULT  pQueryResults,
  PDNS_QUERY_CANCEL  pCancelHandle
);

/* __END_DECLS */
#ifdef __cplusplus
}
#endif

#endif /* DNS_ADDR_MAX_SOCKADDR_LENGTH */


namespace resolw_impl {

void set_last_error(int last_error);

struct ImplPolicies
{
    bool custom;
    bool rotate;
    bool primary;
    bool recurse;
};

ULONG to_query_opts(u_long flags, ImplPolicies * pol);

std::wstring to_win_str(const char* posix_str, std::size_t in_len, bool einval_if_empty = true);

void resolw_nprep(res_state rs, const wchar_t* hostname, unsigned int rdclass, unsigned int rdtype, ULONG qo,
    DNS_ADDR_ARRAY* nsaddrs, DNS_QUERY_REQUEST* req);

} // resolw_impl

#endif /* _SRC_DNS_H_ */
