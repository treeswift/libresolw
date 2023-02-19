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

The asynchronous API (`res_*_async()`) is not yet available. Since Windows 8+, WinSock2 has `DnsQueryEx()` with asynchronous query support
that we can use once we have a pressing use case. We might face one soon, though.

All stateful query methods have (and forward to) their `res_n*()` counterparts. However, the implied `_res` state is thread local, so it's
safe to use the deprecated API (e.g. `res_query()`) as well.

`dn_comp()`, `dn_expand()` and `dn_skipname()` implementations (and teh corresonding `nameser.h` APIs, e.g.
`ns_name_compress()` and `ns_name_uncompress()`) are currently available in the BSD variant (not the public domain variant).

`getrrsetbyname()` and `freerrset()` are implemented and call through to WinDNS API.

The implementation of `res_randomid()`, however trivial, is placed in the public domain. To use the authentic `arc4random`-backed `res_randomid()`,
refer to: [from OpenBSD](https://github.com/treeswift/openbsd-src/blob/2023-02-13/lib/libc/net/res_random.c) (2-clause BSD license).

Normally expected system headers, such as `arpa/nameser.h` and `sys/socket.h` are installed unless found during configuration. The choice of
version to install obeys the `USE_BSD_SOURCE` CMake variable as well. There is no public domain equivalent of `arpa/nameser.h` at the moment,
but this may change in the future.

## Specifications

Since `libresolw` is a compatibility layer, rather than a complete DNS protocol stack implementation, users should
refer to (WinDNS help pages](https://learn.microsoft.com/en-us/windows/win32/api/_dns/) in order to be aware of its
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

The (not necessarily exhaustive, yet comprehensive) list of DNS networks protocol standards can be found at:
https://learn.microsoft.com/en-us/windows/win32/dns/dns-standards-documents

## Implementation notes

### WinDNS correspondences

… (TODO)

### OpenBSD borrowings

`dn_comp()`, `dn_expand()` and `dn_skipname()` implementations in [res_comp.c](src/xxbsd/res_comp.c) are borrowed
[from OpenBSD](https://github.com/treeswift/openbsd-src/blob/2023-02-13/lib/libc/net/res_comp.c).
There is no public domain version available yet.
(TODO) The respective `nameser.h` APIs are `ns_name_compress()` and `ns_name_uncompress()`.

[arpa/nameser.h](include/nameser_h/arpa/nameser.h) (for systems and environments lacking it) has been reused
[from Bionic (AOSP release 13)](https://github.com/aosp-mirror/platform_bionic/tree/android13-release/libc/include/arpa)
and is available under the 3-clause BSD license.

The BSD variant of `endian.h` is borrowed from Bionic: https://github.com/aosp-mirror/platform_bionic/blob/android13-release/libc/include/sys/endian.h

### WinSock2 forwards

Files in `include/resolw/ws2_fwd` contain no copyrightable code. Any of them can be substituted at build time with a single `touch` or `ln -s`.

## Code of Conduct

Be a good neighbor. Or fork, and then we don't care.
