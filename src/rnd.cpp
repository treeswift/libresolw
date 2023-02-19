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

#include <random>
#include <limits>

#include <time.h>

/**
 * One may wonder why we don't reuse a proven implementation from e.g.
 * https://github.com/openbsd/src/blob/master/lib/libc/net/res_random.c
 * (2-clause BSD license). There are multiple reasons, each sufficient
 * independently of others:
 * -- `res_random.c`'s measures to add external entropy to `arc4random`
 *     are made redundant by the very existence of `std::random_device`.
 * -- `libresolw` provides `res_randomid()` for compatibility. It's not
 *     used internally by the WinSock2 DNS API implementation.
 * -- `libresolw` is a public domain library. You are free to change it,
 *     or its API, as you wish, including plugging in your own RNG. Use
 *    `#define res_randomid another_rng_fn` before including `resolv.h`.
 */

namespace {
constexpr int RNG_BITS = 15;
constexpr int RESEED_S = 600;

struct RNGState {
    std::random_device rd;
    std::minstd_rand rng;
    std::uniform_int_distribution<int> dist, tm_d;
    time_t next_reseed_tm;

    void reseed() {
        time_t cur_tm;
        time(&cur_tm);
        if(cur_tm > next_reseed_tm) {
            rng.seed(rd());
            next_reseed_tm = cur_tm + tm_d(rng);
        }
    }

    RNGState() : dist(0, 1 << RNG_BITS), tm_d(RESEED_S, RESEED_S * 2),
                 next_reseed_tm(std::numeric_limits<time_t>::min()) {}

    int operator()() {
        reseed();
        return dist(rng);
    }
};

thread_local RNGState _tls_rnd;

} // anonymous

/* __BEGIN_DECLS */
#ifdef __cplusplus
extern "C" {
#endif

int res_randomid() { return _tls_rnd(); }

/* __END_DECLS */
#ifdef __cplusplus
}
#endif
