
#ifndef __MEMBUS_H__
#define __MEMBUS_H__

#include "system_6502_config.h"
#include "memory_addr_range.h"

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
 * @typedef membus_module_ref
 *
 * Opaque reference to a membus module.
 */
typedef struct membus_module * membus_module_ref;

/*
 * @typedef membus_module_free_fn_t
 *
 * The type of a membus module callback function that does any cleanup
 * prior to deallocation of the module object itself.  E.g. any memory
 * allocated outside the module object itself should be freed.
 */
typedef void (*membus_module_free_fn_t)(membus_module_ref module);

/*
 * @enum membus module operation result
 *
 * Tri-state indicating what a matching membus module did with a
 * transaction:
 *
 * - membus_module_op_result_not_accepted:  the module did not handle
 *        the operation
 * - membus_module_op_result_accepted: the module handled the operation
 *        and propagation should end
 * - membus_module_op_result_observed: the module handled the operation
 *        and propagation should continue on the bus
 */
enum {
    membus_module_op_result_not_accepted = 0,
    membus_module_op_result_accepted,
    membus_module_op_result_observed
};

/*
 * @typedef membus_module_op_result_t
 *
 * Type of the result of a membus module operation.
 */
typedef unsigned int membus_module_op_result_t;

/*
 * @typedef membus_module_read_addr_t
 *
 * The type of a membus module callback function that fetches the byte at
 * memory location addr.
 *
 * If the addr is acceptable, the byte pointed-to by value is set and either
 * membus_module_op_result_accepted or membus_module_op_result_observed is
 * returned.  Otherwise, membus_module_op_result_not_accepted is returned.
 */
typedef membus_module_op_result_t (*membus_module_read_addr_t)(membus_module_ref module, uint16_t addr, uint8_t *value);

/*
 * @typedef membus_module_write_addr_t
 *
 * The type of a membus module callback function that stores a byte at
 * memory location addr.
 *
 * If the addr is acceptable, either membus_module_op_result_accepted or
 * membus_module_op_result_observed is returned.  Otherwise,
 * membus_module_op_result_not_accepted is returned.
 */
typedef membus_module_op_result_t (*membus_module_write_addr_t)(membus_module_ref module, uint16_t addr, uint8_t value);

/*
 * @enum membus module mode
 *
 * Access mode for some membus modules:
 *
 * - membus_module_mode_rw:  addresses associated with module are read-write
 * - membus_module_mode_ro:  addresses associated with module are read-only
 * - membus_module_mode_wo:  addresses associated with module are write-only
 */
enum {
    membus_module_mode_rw = 0,
    membus_module_mode_ro = 1,
    membus_module_mode_wo = 2,
    membus_module_mode_mask = 3
};

/*
 * @typedef membus_module_mode_t
 *
 * Type of the access mode of a membus module.
 */
typedef unsigned int membus_module_mode_t;

/*
 * @function membus_module_get_ref_count
 *
 * Get the number of references to module are currently held.
 */
unsigned int membus_module_get_ref_count(membus_module_ref module);

/*
 * @function membus_module_retain
 *
 * Increment the reference count of module.  The module reference
 * is returned.
 */
membus_module_ref membus_module_retain(membus_module_ref module);

/*
 * @function membus_module_release
 *
 * Decrement the reference count of module.  Once no references
 * remain, the module is destroyed.
 */
void membus_module_release(membus_module_ref module);





/*
 * @typedef membus_t
 *
 * Data structure representing a membus object.  Fields are:
 *
 *     - pre_op: opaque reference to the chain of membus modules that
 *          will be called invariably ahead of all other modules on the
 *          bus
 *     - modules: opaque reference to the chain of tiers of membus
 *          modules registered on the bus
 *     - post_op: opaque reference to the chain of membus modules that
 *          will be called invariably after all other modules on the
 *          bus
 *     - rw_lock: mutex used to coordinate multithreaded access to the
 *          bus
 *     - nmi_vector: special case, the word at 0xFFFA if unhandled by
 *          any module
 *     - res_vector: special case, the word at 0xFFFC if unhandled by
 *          any module
 *     - irq_vector: special case, the word at 0xFFFE if unhandled by
 *          any module
 *
 * If membus caching is enabled at compile time the data structure
 * will also include the read- and write-caches and their indices:
 *
 *      - rcache_idx: next index in the read cache to which data will
 *          be pushed
 *      - wcache_idx: next index in the write cache to which data will
 *          be pushed
 *      - rcache: cyclic buffer of the last 8 bytes successfully read
 *          from the bus
 *      - wcache: cyclic buffer of the last 8 bytes successfully written
 *          to the bus
 */
typedef struct membus {
    const void      *pre_op, *modules, *post_op;
    uint16_t        nmi_vector, res_vector, irq_vector;

#ifdef ENABLE_MEMBUS_LOCKS
    pthread_mutex_t rw_lock;
#endif

#ifdef ENABLE_MEMBUS_CACHE
    uint8_t         rcache_idx, wcache_idx;
    uint8_t         rcache[8], wcache[8];
#endif
} membus_t;


#ifdef ENABLE_MEMBUS_CACHE
/*
 * @defined membus_rcache_push
 *
 * Push the byte B to the read cache of membus object M.
 */
#   define membus_rcache_push(M, B)  (M)->rcache[(M)->rcache_idx++ % 8] = (B)
/*
 * @defined membus_rcache_peek
 *
 * Returns the last-pushed byte of the read cache of membus object M
 * without moving the cache pointer.
 */
#   define membus_rcache_peek(M)     (M)->rcache[((M)->rcache_idx - 1) % 8]
/*
 * @defined membus_rcache_pop
 *
 * Removes and returns the last-pushed byte of the read cache of membus
 * object M.
 */
#   define membus_rcache_pop(M)      (M)->rcache[--(M)->rcache_idx % 8]
/*
 * @defined membus_wcache_push
 *
 * Push the byte B to the write cache of membus object M.
 */
#   define membus_wcache_push(M, B)  (M)->wcache[(M)->wcache_idx++ % 8] = (B)
/*
 * @defined membus_wcache_peek
 *
 * Returns the last-pushed byte of the write cache of membus object M
 * without moving the cache pointer.
 */
#   define membus_wcache_peek(M)     (M)->wcache[((M)->wcache_idx - 1) % 8]
/*
 * @defined membus_wcache_pop
 *
 * Removes and returns the last-pushed byte of the write cache of membus
 * object M.
 */
#   define membus_wcache_pop(M)      (M)->wcache[--(M)->wcache_idx % 8]
#endif


/*
 * @function membus_alloc
 *
 * Allocate and initialize a new membus object.  The bus initially contains
 * no modules and effectively only handles the three special jump vector
 * words (NMI, RES, IRQ).
 *
 * A variety of unique membus modules can be allocated and registered with
 * a membus object in order to provide a virtualized composite memory space.
 */
membus_t* membus_alloc(void);

/*
 * @function membus_free
 *
 * Dispose of the_membus.  All membus module references registered with
 * the_membus are released (via membus_module_release()).
 */
void membus_free(membus_t *the_membus);

/*
 * @defined MEMBUS_TIER_PRE_OP
 *
 * Special memory bus tier for modules that are called before the operation
 * takes place.  Primarily for modules that observe activity on the bus.
 */
#define MEMBUS_TIER_PRE_OP INT_MIN

/*
 * @defined MEMBUS_TIER_POST_OP
 *
 * Special memory bus tier for modules that are called after the operation
 * takes place.  Primarily for modules that observe activity on the bus.
 *
 * It's also permissible to register a std64k module in this tier in order
 * to maintain a full RAM image with all other modules acting as overlays.
 */
#define MEMBUS_TIER_POST_OP INT_MAX

/*
 * @function membus_register_module
 *
 * Register a membus module object with the_membus in the specifed tier.
 * A reference to the_module is retained by the_membus (via the
 * membus_module_retain() function).
 */
bool membus_register_module(membus_t *the_membus, int tier, membus_module_ref the_module);

/*
 * @function membus_read_addr
 *
 * Returns the byte present at memory address addr in the_membus.
 */
uint8_t membus_read_addr(membus_t *the_membus, uint16_t addr);

/*
 * @function membus_write_addr
 *
 * Stores the value to memory address addr in the_membus.
 */
void membus_write_addr(membus_t *the_membus, uint16_t addr, uint8_t value);

/*
 * @function membus_write_byte_to_range
 *
 * Writes the byte value to the range r of addresses in the_membus memory
 * array.  Any watchpoints triggered by the write will be notified.
 */
void membus_write_byte_to_range(membus_t *the_membus, memory_addr_range_t r, uint8_t value);

/*
 * @function membus_write_word_to_range
 *
 * Writes the byte value to the range r of addresses in the_membus memory
 * array.  Any watchpoints triggered by the write will be notified.
 */
void membus_write_word_to_range(membus_t *the_membus, memory_addr_range_t r, uint16_t value);

/*
 * @function membus_copy_out_bytes
 *
 * Copy all bytes from the_membus in the given address range r to the buffer
 * pointed to by bytes_ptr.  The buffer is assumed to be at least as large as
 * the length of the address range r.
 */
void membus_copy_out_bytes(membus_t *the_membus, uint8_t *bytes_ptr, memory_addr_range_t r);

/*
 * @function membus_copy_in_bytes
 *
 * Copy bytes from the buffer pointed to by bytes_ptr to the given address range
 * of the_membus.  The buffer is assumed to be at least as large as the length of
 * the address range r.
 */
void membus_copy_in_bytes(membus_t *the_membus, const uint8_t *bytes_ptr, memory_addr_range_t r);

/*
 * @function membus_load_from_fd
 *
 * Fill the_membus memory array in the given range r with bytes read from the given
 * file descriptor.  The actual number of bytes read is returned.
 */
ssize_t membus_load_from_fd(membus_t *the_membus, memory_addr_range_t r, int fd);

/*
 * @function membus_load_from_stream
 *
 * Fill the_membus memory array starting at address baseaddr with bytesize bytes
 * read from the given file stream.  The actual number of bytes read is returned.
 *
 * If the bytesize and baseaddr together extend beyond the limits of the memory
 * array, the bytesize is adjusted to only fill the memory array.
 */
ssize_t membus_load_from_stream(membus_t *the_membus, memory_addr_range_t r, FILE *stream);

/*
 * @function membus_save_to_fd
 *
 * Write bytesize bytes starting at address baseaddr of the_membus memory array
 * to the given file descriptor.  The actual number of bytes written is returned.
 *
 * If the bytesize and baseaddr together extend beyond the limits of the memory
 * array, the bytesize is adjusted to only reach the end of the memory array.
 */
ssize_t membus_save_to_fd(membus_t *the_membus, memory_addr_range_t r, int fd);

/*
 * @function membus_save_to_stream
 *
 * Write bytesize bytes starting at address baseaddr of the_membus memory array
 * to the given file stream.  The actual number of bytes written is returned.
 *
 * If the bytesize and baseaddr together extend beyond the limits of the memory
 * array, the bytesize is adjusted to only reach the end of the memory array.
 */
ssize_t membus_save_to_stream(membus_t *the_membus, memory_addr_range_t r, FILE *stream);

/*
 * @enum Membus dump options
 *
 * Bit vector values that control the output of memory ranges.
 *
 * - membus_dump_opts_8byte_width: output 8 bytes per line rather than 16
 * - membus_dump_opts_compact: eliminate as much whitespace as possible on each line
 */
enum {
    membus_dump_opts_8byte_width = 1 << 0,
    membus_dump_opts_compact = 1 << 1
};

/*
 * @typedef membus_dump_opts_t
 *
 * The type of membus dump options.
 */
typedef unsigned int membus_dump_opts_t;

/*
 * @function membus_fprintf
 *
 * Output to the given file stream a hexdump of the raw bytes and ASCII characters
 * in the address range [addr_start, addr_end] of the_membus memory array.
 */
int membus_fprintf(membus_t *the_membus, FILE *stream, membus_dump_opts_t opts, memory_addr_range_t addr_range);

#endif /* __MEMBUS_H__ */
