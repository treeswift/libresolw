#include "resolv.h"

// Authoritative sources:
// https://learn.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2
// https://learn.microsoft.com/en-us/windows/win32/winsock/error-codes-errno-h-errno-and-wsagetlasterror-2

#include <cstdio>
#include <system_error>

/* __BEGIN_DECLS */
#ifdef __cplusplus
extern "C" {
#endif

void herror(const char *s) {
    int lastcode = h_errno;
    auto message = std::system_category().message(lastcode); // idiomatic
    std::fprintf(stderr, "%s: %s\n", s, message.c_str());
}

/* __END_DECLS */
#ifdef __cplusplus
}
#endif
