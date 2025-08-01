/*
*  List hash algorithms to benchmark
*  Part of xxHash project
*  Copyright (C) 2019-2021 Yann Collet
*
*  GPL v2 License
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along
*  with this program; if not, write to the Free Software Foundation, Inc.,
*  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*
*  You can contact the author at:
*  - xxHash homepage: https://www.xxhash.com
*  - xxHash source repository: https://github.com/Cyan4973/xxHash
*/


/* ===   Dependencies   === */

#include <stddef.h>   /* size_t */


/* ==================================================
 *   Non-portable hash algorithms
 * =============================================== */


#ifdef HARDWARE_SUPPORT

/*
 * List any hash algorithms that depend on specific hardware support,
 * including for example:
 * - Hardware crc32c
 * - Hardware AES support
 * - Carryless Multipliers (clmul)
 * - AVX2
 */

#endif



/* ==================================================
 * List of hashes
 * ==================================================
 * Each hash must be wrapped in a thin redirector conformant with the BMK_benchfn_t.
 * BMK_benchfn_t is generic, not specifically designed for hashes.
 * For hashes, the following parameters are expected to be useless:
 * dst, dstCapacity, customPayload.
 *
 * The result of each hash is assumed to be provided as function return value.
 * This condition is important for latency measurements.
 */

 /* ===  xxHash  === */
 #define XXH_INLINE_ALL
 #include "xxhash.h"
 #include "rapidhash.h"

size_t rapidhash_wrapper(const void* src, size_t srcSize, void* dst, size_t dstCapacity, void* customPayload)
{
    (void)dst; (void)dstCapacity; (void)customPayload;
    return (size_t) rapidhash(src, srcSize);
}

size_t rapidhashMicro_wrapper(const void* src, size_t srcSize, void* dst, size_t dstCapacity, void* customPayload)
{
    (void)dst; (void)dstCapacity; (void)customPayload;
    return (size_t) rapidhashMicro(src, srcSize);
}

size_t rapidhashNano_wrapper(const void* src, size_t srcSize, void* dst, size_t dstCapacity, void* customPayload)
{
    (void)dst; (void)dstCapacity; (void)customPayload;
    return (size_t) rapidhashNano(src, srcSize);
}

size_t xxh3_wrapper(const void* src, size_t srcSize, void* dst, size_t dstCapacity, void* customPayload)
{
    (void)dst; (void)dstCapacity; (void)customPayload;
    return (size_t) XXH3_64bits(src, srcSize);
}

/* ==================================================
 * Table of hashes
 * =============================================== */

#include "bhDisplay.h"   /* Bench_Entry */

#  define NB_HASHES 4

Bench_Entry const hashCandidates[NB_HASHES] = {
    { "rapidhash" , rapidhash_wrapper },
    { "rapidhashMicro" , rapidhashMicro_wrapper },
    { "rapidhashNano" , rapidhashNano_wrapper },
    { "xxh3"  , xxh3_wrapper },
#ifdef HARDWARE_SUPPORT
    /* list here codecs which require specific hardware support, such SSE4.1, PCLMUL, AVX2, etc. */
#endif
};
