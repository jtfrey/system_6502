
#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "system_6502_config.h"

/*
 * @typedef memory_page
 *
 * A single page (256 bytes) of memory.
 */
typedef uint8_t memory_page[256];

/*
 * @typedef memory_t
 *
 * A data structure containing 64 KiB of memory (256 pages)
 * presented as an array of pages or a flat array of bytes.
 * The structure also includes an opaque pointer to an
 * internally-managed list of memory watchpoints that will
 * be notified on read/write to an address.
 *
 * The rcache is a ring buffer that is filled as memory_read()
 * calls are made.  No address information is retained in the
 * cache, so bytes may not have been contiguous.  But in the
 * case of the 6502, each instruction has a known number of
 * memory reads associated with it:  a JMP $XXXX uses 3 reads
 * while a JMP ($XXXX) uses 5 reads.  In both cases, the
 * rcache will contain those reads in sequence.  A runtime
 * disassembler (with knowledge of what opcode was executed)
 * can check the rcache for the operand(s).
 *
 * The same principle works w.r.t. wcache and the output from
 * an instruction.
 */
typedef struct memory {
    union {
        memory_page     PAGES[256];
        uint8_t         BYTES[256 * 256];
    } RAM;

#ifdef ENABLE_MEMORY_CACHE
    uint8_t             rcache_idx, wcache_idx;
    uint8_t             rcache[8], wcache[8];
#endif
    
#ifdef ENABLE_MEMORY_WATCHPOINTS
    const void          *watchpoints;
#endif
} memory_t;

#ifdef ENABLE_MEMORY_CACHE
#   define memory_rcache_push(M, B)  (M)->rcache[M->rcache_idx++ % 8] = (B)
#   define memory_rcache_peek(M)     (M)->rcache[M->rcache_idx % 8]
#   define memory_rcache_pop(M)      (M)->rcache[--M->rcache_idx % 8]

#   define memory_wcache_push(M, B)  (M)->wcache[M->wcache_idx++ % 8] = (B)
#   define memory_wcache_peek(M)     (M)->wcache[M->wcache_idx % 8]
#   define memory_wcache_pop(M)      (M)->wcache[--M->wcache_idx % 8]
#endif

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
 * @memory_alloc
 *
 * Dynamically-allocate a memory array.  The array should be
 * initialized by calling the memory_init() function.
 */
memory_t* memory_alloc(void);

/*
 * @function memory_init
 *
 * Initialize the_memory memory array.  If the_memory is NULL, a
 * new memory array will be allocated and returned.
 */
memory_t* memory_init(memory_t *the_memory);

/*
 * @function memory_free
 *
 * Deallocate a dynamically-allocated the_memory.
 */
void memory_free(memory_t *the_memory);

/*
 * @function memory_reset()
 *
 * Reset the_memory memory array by filling the array with the
 * provided fill_byte.
 */
void memory_reset(memory_t *the_memory, uint8_t fill_byte);

/*
 * @function memory_read
 *
 * Returns the byte present at addr in the_memory memory array.  Any
 * watchpoints triggered by the read will be notified.
 */
uint8_t memory_read(memory_t *the_memory, uint16_t addr);

/*
 * @function memory_write
 *
 * Writes the byte value to addr in the_memory memory array.  Any
 * watchpoints triggered by the write will be notified.
 */
void memory_write(memory_t *the_memory, uint16_t addr, uint8_t value);

/*
 * @function memory_load_from_fd
 *
 * Fill the_memory memory array starting at address baseaddr with bytesize bytes
 * read from the given file descriptor.  The actual number of bytes read is returned.
 *
 * If the bytesize and baseaddr together extend beyond the limits of the memory
 * array, the bytesize is adjusted to only fill the memory array.
 */
ssize_t memory_load_from_fd(memory_t *the_memory, uint16_t baseaddr, uint16_t bytesize, int fd);

/*
 * @function memory_load_from_stream
 *
 * Fill the_memory memory array starting at address baseaddr with bytesize bytes
 * read from the given file stream.  The actual number of bytes read is returned.
 *
 * If the bytesize and baseaddr together extend beyond the limits of the memory
 * array, the bytesize is adjusted to only fill the memory array.
 */
ssize_t memory_load_from_stream(memory_t *the_memory, uint16_t baseaddr, uint16_t bytesize, FILE *stream);

/*
 * @function memory_save_to_fd
 *
 * Write bytesize bytes starting at address baseaddr of the_memory memory array
 * to the given file descriptor.  The actual number of bytes written is returned.
 *
 * If the bytesize and baseaddr together extend beyond the limits of the memory
 * array, the bytesize is adjusted to only reach the end of the memory array.
 */
ssize_t memory_save_to_fd(memory_t *the_memory, uint16_t baseaddr, uint16_t bytesize, int fd);

/*
 * @function memory_save_to_stream
 *
 * Write bytesize bytes starting at address baseaddr of the_memory memory array
 * to the given file stream.  The actual number of bytes written is returned.
 *
 * If the bytesize and baseaddr together extend beyond the limits of the memory
 * array, the bytesize is adjusted to only reach the end of the memory array.
 */
ssize_t memory_save_to_stream(memory_t *the_memory, uint16_t baseaddr, uint16_t bytesize, FILE *stream);

/*
 * @function memory_fprintf
 *
 * Output to the given file stream a hexdump of the raw bytes and ASCII characters
 * in the address range [addr_start, addr_end] of the_memory memory array.
 */
int memory_fprintf(memory_t *the_memory, FILE *stream, uint16_t addr_start, uint16_t addr_end);



#ifdef ENABLE_MEMORY_WATCHPOINTS

/*
 * @enum memory watchpoint events
 *
 * Events and actions associated with memory watchpoints:
 *
 * - memory_watchpoint_event_read:  notify on read of memory location
 * - memory_watchpoint_event_write:  notify on write to memory location
 * - memory_watchpoint_event_all : notify on read or write of memory location
 
 * - memory_watchpoint_event_merge_at_register:  when the same callback is
 *     registered additonal times on the same address, add to the event mask
 *     rather than setting it outright
 */
enum {
    memory_watchpoint_event_read    = 1 << 0,
    memory_watchpoint_event_write   = 1 << 1,
    //
    memory_watchpoint_event_all     = memory_watchpoint_event_read | memory_watchpoint_event_write,
    //
    memory_watchpoint_event_merge_at_register   = 1 << 7
};

/*
 * @typedef memory_watchpoint_event_t
 *
 * Type of a memory watchpoint event mask.
 */
typedef uint8_t memory_watchpoint_event_t;

/*
 * @typedef memory_watchpoint_ref
 *
 * Opaque reference to a registered memory watchpoint.
 */
typedef struct memory_watchpoint * memory_watchpoint_ref;

/*
 * @typedef memory_watchpoint_callback_t
 *
 * Type of a function that is called when a read/write to a watched memory
 * location is performed.  Arguments are the reference to the memory array
 * for which the event was triggered, the address that triggered it, the event
 * type, and a opaque context pointer that was provided when the watchpoint was
 * registered (used to convey additional state etc. to the callback).
 */
typedef void (*memory_watchpoint_callback_t)(memory_t *the_memory, uint16_t addr, memory_watchpoint_event_t the_event, const void *context);

/*
 * @function memory_watchpoint_register
 *
 * Register callback function the_callback as a watchpoint for on_events events
 * occuring w.r.t. addr in the_memory memory array.  The context argument is an
 * opaque pointer that is passed to the_callback when it is invoked.
 *
 * If the watchpoint is successfully registered the memory_watchpoint_ref that
 * references it is returned.  On failure, NULL is returned.
 */
memory_watchpoint_ref memory_watchpoint_register(memory_t *the_memory, uint16_t addr, memory_watchpoint_event_t on_events, memory_watchpoint_callback_t the_callback, const void *context);

/*
 * @function memory_watchpoint_get_address
 *
 * Get the address associated with a watchpoint reference.
 */
uint16_t memory_watchpoint_get_address(memory_watchpoint_ref the_watchpoint);

/*
 * @function memory_watchpoint_get_events
 *
 * Get the event mask associated with a watchpoint reference.
 */
memory_watchpoint_event_t memory_watchpoint_get_events(memory_watchpoint_ref the_watchpoint);

/*
 * @function memory_watchpoint_set_events
 *
 * Merge/set the event mask for the_watchpoint using the on_events mask.  If
 * memory_watchpoint_event_merge_at_register is set, the new event bits are OR'ed
 * into the existing mask; otherwise, the mask is set outright.
 */
void memory_watchpoint_set_events(memory_watchpoint_ref the_watchpoint, memory_watchpoint_event_t on_events);

/*
 * @function memory_watchpoint_unregister
 *
 * Remove the_watchpoint from the memory array in which it is registered.
 */
void memory_watchpoint_unregister(memory_watchpoint_ref the_watchpoint);

#endif

#endif /* __MEMORY_H__ */
