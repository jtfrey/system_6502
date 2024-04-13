
#ifndef __MEMORY_ADDR_RANGE_H__
#define __MEMORY_ADDR_RANGE_H__

#include "system_6502_config.h"

/*
 * @typedef memory_addr_range_t
 *
 * A type that represents a range of memory addresses.
 */
typedef struct memory_addr_range {
    uint16_t    addr_lo, addr_len;
} memory_addr_range_t;

extern const memory_addr_range_t memory_addr_range_undef;

/*
 * @function memory_addr_range_with_lo_and_hi
 *
 * Return a memory_addr_range_t structure initialized to represent
 * the address range [addr_lo, addr_hi] (inclusive).
 *
 */
static inline memory_addr_range_t
memory_addr_range_with_lo_and_hi(
    uint16_t        addr_lo,
    uint16_t        addr_hi
)
{
    memory_addr_range_t new_range = {
        .addr_lo = addr_lo,
        .addr_len = (addr_hi - addr_lo + 1)
    };
    return new_range;
}

/*
 * @function memory_addr_range_with_start_and_len
 *
 * Return a memory_addr_range_t structure initialized with the
 * provided starting address and length of the range.
 *
 */
static inline memory_addr_range_t
memory_addr_range_with_lo_and_len(
    uint16_t        addr_lo,
    uint16_t        addr_len
)
{
    memory_addr_range_t new_range = {
        .addr_lo = addr_lo,
        .addr_len = addr_len
    };
    return new_range;
}

/*
 * @function memory_addr_range_get_end
 *
 * Return the final address in the memory range r w/o wrap around.
 *
 */
static inline uint16_t
memory_addr_range_get_end(
    memory_addr_range_t *r
)
{
    uint32_t    addr_end = (uint32_t)r->addr_lo + (uint32_t)r->addr_len;
    return ( (addr_end < 0x00010000) ? addr_end : 0xFFFF );
}

/*
 * @function memory_addr_range_get_end_with_wrap
 *
 * Return the final address in the memory range r with wrap around.
 *
 */
static inline uint16_t
memory_addr_range_get_end_with_wrap(
    memory_addr_range_t *r
)
{
    uint32_t    addr_end = (uint32_t)r->addr_lo + (uint32_t)r->addr_len;
    return (addr_end & 0xFFFF);
}

/*
 * @functon memory_addr_range_does_include
 *
 * Is the value idx in the given memory address range, r1?
 */
static inline bool
memory_addr_range_does_include(
    memory_addr_range_t *r1,
    uint16_t            idx
)
{
    return ( r1->addr_len && (idx >= r1->addr_lo) && (idx < r1->addr_lo + r1->addr_len) );
}

/*
 * @function memory_addr_range_do_intersect
 *
 * Returns true if the two memory address ranges, r1 and r2, intersect.
 */
bool memory_addr_range_do_intersect(memory_addr_range_t *r1, memory_addr_range_t *r2);

/*
 * @function memory_addr_range_intersection
 *
 * If the two memory address ranges, r1 and r2, intersect then set the range rOut
 * to their intersection.  Otherwise, rOut is set to the undefined range.
 *
 * The pointer rOut is returned.
 */
memory_addr_range_t* memory_addr_range_intersection(memory_addr_range_t *r1, memory_addr_range_t *r2, memory_addr_range_t *rOut);

/*
 * @function memory_addr_range_union
 *
 * If the two memory address ranges, r1 and r2, intersect then set the range rOut
 * to their union.  Otherwise, rOut is set to the undefined range.
 *
 * The pointer rOut is returned.
 */
memory_addr_range_t* memory_addr_range_union(memory_addr_range_t *r1, memory_addr_range_t *r2, memory_addr_range_t *rOut);

#endif /* __MEMORY_ADDR_RANGE_H__ */
