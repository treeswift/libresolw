## What is this?

**libresolw**, or **libresolW**, is a `resolv.h` implementation on Windows under a permissive license.
You can either use it as a monolythic dependency according to the terms of a 3-clause BSD license or
use its Windows-specific parts freely without any restrictions whatsoever. All Windows-specific parts
of `libresolw` have been released into the public domain. In addition, building `libresolw` with the
following CMake variable assignment:

```
cmake -DUSE_BSD_SOURCE:Bool=OFF
```

ensures that only public domain code is compiled (the specific terms are as per the [Unlicense License](https://unlicense.org/)).

## Why another libresolv?

The primary reason we started `libresolW` was that [openssh-portable](https://github.com/openssh/openssh-portable) wanted either
`getrrsetbyname()` or `resolv.h`; our release of [llvm-mingw runtime](https://github.com/armdevvel/llvm-mingw/releases/tag/14.0)
didn't have it; we didn't want to contaminate the port with a more restrictive license (therefore rejecting `minires` from Cygwin
and `libresolv` from Darwin); and we wanted the implementation to be as thin a wrapper around
[WinDNS](https://learn.microsoft.com/en-us/windows/win32/api/_dns/)/[WinSock](https://learn.microsoft.com/en-us/windows/win32/api/_winsock/)
as possible to avoid introducing a parallel stack with a parallel configuration (a condition that plagues Mac environments, for example).

TL;DR we wanted to streamline the terms of use and make them as free as possible. 2023 may be waaay too late
(since most of the functionality settled down in the early 2000s), but better late than never.

## Is it feature complete?

No. The library is in active development stage.

## What would you consider "feature complete"?

The asynchronous API (`res_*_async()`) is not yet available. Since Windows 8+, WinSock2 has `DnsQueryEx()` with asynchronous query support
that we can use once we have a pressing use case. We might face one soon, though.

All stateful query methods have (and forward to) their `res_n*()` counterparts. However, the implied `_res` state is thread local, so it's
safe to use the deprecated API (e.g. `res_query()`) as well.

`res_send()` has no equivalent WinDNS API. Sending a raw DNS request message is accomplished by first parsing it and then issuing a regular
`DnsQuery*` request based on parsing results. Set the `Reserved` bit to 1 in `DNS_HEADER` to verify parsing output by calling `res_mkquery()`
again and comparing the message header and body for bit-exact match (the message buffer begins with `DNS_HEADER` — use a `reinterpret_cast`).

`dn_comp()`, `dn_expand()` and `dn_skipname()` implementations (and teh corresponding `nameser.h` APIs, e.g.
`ns_name_compress()` and `ns_name_uncompress()`) are currently available in the BSD variant (not the public domain variant).

`getrrsetbyname()` and `freerrset()` are implemented and call through to WinDNS API.

The implementation of `res_randomid()`, however trivial, is placed in the public domain. To use the authentic `arc4random`-backed `res_randomid()`,
refer to: [from OpenBSD](https://github.com/treeswift/openbsd-src/blob/2023-02-13/lib/libc/net/res_random.c) (2-clause BSD license).

Normally expected system headers, such as `arpa/nameser.h` and `sys/socket.h`, are installed unless found during configuration. The choice of
version to install obeys the `USE_BSD_SOURCE` CMake variable as well. The public domain equivalent of `arpa/nameser.h` is partially complete.

## Specifications

Since `libresolw` is a compatibility layer, rather than a complete DNS protocol stack implementation, users should
refer to [WinDNS help pages](https://learn.microsoft.com/en-us/windows/win32/api/_dns/) in order to be aware of its
functional limitations.

The domain name resolver API interface and implementation in `libresolw` follows the BIND 8.2 API description as per:

```
O'Reilly® DNS&BIND by Paul Albitz and Cricket Liu.
Fourth edition, published April 2001.
ISBN: 0-596-00158-4
Chapter 15.2. "C Programming with the Resolver Library Routines"
Copyright © 2001, 1998, 1997, 1992 O'Reilly & Associates, Inc. All rights reserved.
```

`getrrsetbyname()` is implemented according to: https://man.openbsd.org/OpenBSD-7.2/getrrsetbyname.3

The (not necessarily exhaustive, yet comprehensive) list of DNS network protocol standards can be found at:
https://learn.microsoft.com/en-us/windows/win32/dns/dns-standards-documents
It does not mention since-superseded RFCs, such as [RFC-883](https://www.rfc-editor.org/rfc/rfc883) and [RFC-974](https://www.rfc-editor.org/rfc/rfc973).

## Implementation notes

### WinDNS correspondence

(TODO)

There is no WinDNS implementation of the `res_send()` method that sends a pre-serialized (and possibly amended) DNS query with retrial.
The choice is between implementing it on top of UDP or TCP sockets from scratch (roughly what Bionic and older resolvers do) and parsing
it back into (higher-level) DnsQueryEx arguments (roughly what ASR does). Following the example of the proverbial mathematician facing
an already-boiling teakettle, we chose the latter method.

### Presumptions and shortcuts

The default [address sort list](https://unix.stackexchange.com/questions/332559/what-is-the-use-of-sortlist-option-in-etc-resolv-conf) is always empty.

### OpenBSD code reuse

`dn_comp()`, `dn_expand()` and `dn_skipname()` implementations in [res_comp.c](src/xxbsd/res_comp.c) are borrowed
[from OpenBSD](https://github.com/treeswift/openbsd-src/blob/2023-02-13/lib/libc/net/res_comp.c).
There is no public domain version available yet, and these methods aren't even declared if the library is built
in public domain mode.

(TODO) The respective `nameser.h` APIs are `ns_name_compress()` and `ns_name_uncompress()`.

[arpa/nameser.h](include/nameser_h/arpa/nameser.h) (for systems and environments lacking it) has been reused
[from Bionic (AOSP release 13)](https://github.com/aosp-mirror/platform_bionic/tree/android13-release/libc/include/arpa)
and is available under the 3-clause BSD license.

The BSD variant of `endian.h` is borrowed from Bionic: https://github.com/aosp-mirror/platform_bionic/blob/android13-release/libc/include/sys/endian.h

### WinSock2 forwards

Files in `include/resolw/ws2_fwd` contain no copyrightable code. Any of them can be substituted at build time with a single `touch` or `ln -s`.
Example:

```
INCLUDE=$PREFIX/include

mkdir -p $INCLUDE/sys && echo '#include <ws2tcpip.h>' \
    > $INCLUDE/sys/socket.h

mkdir -p $INCLUDE/arpa && echo '#include <winsock2.h>' \
    > $INCLUDE/arpa/inet.h

mkdir -p $INCLUDE/netinet && echo '#include <winsock2.h>' \
| tee $INCLUDE/netinet/in_systm.h \
| tee $INCLUDE/netinet/in.h \
    > $INCLUDE/netinet/ip.h
```

## Code of Conduct

Be a good neighbor. Or fork, and then we don't care.
