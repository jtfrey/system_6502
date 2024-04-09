
#ifndef __MEMBUS_H__
#define __MEMBUS_H__

#include "../system_6502_config.h"

/*
 * @defined MEMORY_ADDR_NMI_VECTOR
 *
 * Address of the NMI jump vector.
 */
#define MEMORY_ADDR_NMI_VECTOR  ((uint16_t)0xFFFA)

/*
 * @defined MEMORY_ADDR_RES_VECTOR
 *
 * Address of the RESET jump vector.
 */
#define MEMORY_ADDR_RES_VECTOR  ((uint16_t)0xFFFC)

/*
 * @defined MEMORY_ADDR_IRQ_VECTOR
 *
 * Address of the IRQ jump vector.
 */
#define MEMORY_ADDR_IRQ_VECTOR  ((uint16_t)0xFFFE)


/*
 * @typedef memory_page_t
 *
 * A single page (256 bytes) of memory.
 */
typedef uint8_t memory_page_t[0x100];


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



typedef void (*membus_free_fn_t)(const void *module);
typedef bool (*membus_read_addr_t)(const void *module, uint16_t addr, uint8_t *value);
typedef bool (*membus_write_addr_t)(const void *module, uint16_t addr, uint8_t value);

enum {
    membus_module_mode_rw = 0,
    membus_module_mode_ro = 1,
    membus_module_mode_wo = 2,
    membus_module_mode_mask = 3
};
typedef unsigned int membus_module_mode_t;


typedef struct membus_module {
    const char              *module_id;
    memory_addr_range_t     addr_range;
    membus_free_fn_t        free_callback;
    membus_read_addr_t      read_callback;
    membus_write_addr_t     write_callback;
} membus_module_t;


typedef struct membus {
    const void      *modules;
    
    uint16_t        nmi_vector, res_vector, irq_vector;
    
#ifdef ENABLE_MEMORY_CACHE
    uint8_t         rcache_idx, wcache_idx;
    uint8_t         rcache[8], wcache[8];
#endif
    
#ifdef ENABLE_MEMORY_WATCHPOINTS
    const void      *watchpoints[3];
#endif
} membus_t;


#ifdef ENABLE_MEMORY_CACHE
#   define memory_rcache_push(M, B)  (M)->rcache[M->rcache_idx++ % 8] = (B)
#   define memory_rcache_peek(M)     (M)->rcache[M->rcache_idx % 8]
#   define memory_rcache_pop(M)      (M)->rcache[--M->rcache_idx % 8]

#   define memory_wcache_push(M, B)  (M)->wcache[M->wcache_idx++ % 8] = (B)
#   define memory_wcache_peek(M)     (M)->wcache[M->wcache_idx % 8]
#   define memory_wcache_pop(M)      (M)->wcache[--M->wcache_idx % 8]
#endif

membus_t* membus_alloc(void);
void membus_free(membus_t *the_membus);

bool membus_register_module(membus_t *the_membus, int tier, membus_module_t *the_module);

uint8_t membus_read_addr(membus_t *the_membus, uint16_t addr);
void membus_write_addr(membus_t *the_membus, uint16_t addr, uint8_t value);

#endif /* __MEMBUS_H__ */
