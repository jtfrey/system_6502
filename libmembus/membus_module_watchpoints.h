
#ifndef __MEMBUS_MODULE_WATCHPOINTS_H__
#define __MEMBUS_MODULE_WATCHPOINTS_H__

#include "membus.h"

/*
 * @enum membus watchpoint subtypes
 *
 * The various kinds of memory watchpoints that can be created.
 *
 * - membus_watchpoint_subtype_generic: observes all addresses
 * - membus_watchpoint_subtype_address: observes a single address
 * - membus_watchpoint_subtype_address_range: observes a range of
 *   addresses
 */
enum {
    membus_watchpoint_subtype_generic = 0,
    membus_watchpoint_subtype_address,
    membus_watchpoint_subtype_address_range
};

/*
 * @typedef membus_watchpoint_subtype_t
 *
 * The type of a membus watchpoint subtype identifier.
 */
typedef unsigned int membus_watchpoint_subtype_t;

/*
 * @enum membus watchpoint events
 *
 * Events and actions associated with membus watchpoints:
 *
 * - membus_watchpoint_event_read:  notify on read of memory location
 * - membus_watchpoint_event_write:  notify on write to memory location
 * - membus_watchpoint_event_all : notify on read or write of memory location
 
 * - membus_watchpoint_event_merge_at_register:  when the same callback is
 *     registered additonal times on the same address, add to the event mask
 *     rather than setting it outright
 */
enum {
    membus_watchpoint_event_read    = 1 << 0,
    membus_watchpoint_event_write   = 1 << 1,
    //
    membus_watchpoint_event_all     = membus_watchpoint_event_read | membus_watchpoint_event_write,
    //
    membus_watchpoint_event_merge_at_register   = 1 << 3
};

/*
 * @typedef membus_watchpoint_event_t
 *
 * Type of a membus watchpoint event mask.
 */
typedef unsigned int membus_watchpoint_event_t;

/*
 * @typedef membus_watchpoint_ref
 *
 * Opaque reference to a registered membus watchpoint.
 */
typedef struct membus_watchpoint * membus_watchpoint_ref;

/*
 * @typedef membus_watchpoint_callback_t
 *
 * Type of a function that is called when a read/write to a watched memory
 * location is performed.  Arguments are the reference to the membus module
 * for which the event was triggered, the address that triggered it, the event
 * type, and a opaque context pointer that was provided when the watchpoint was
 * registered (used to convey additional state etc. to the callback).
 */
typedef void (*membus_watchpoint_callback_t)(membus_module_ref the_module, membus_watchpoint_event_t the_event, uint16_t addr, uint8_t value, const void *context);

/*
 * @function membus_module_watchpoints_alloc
 *
 * Allocate a new watchpoint membus module registered to all 64K
 * of the memory space.
 */
membus_module_ref membus_module_watchpoints_alloc(membus_module_mode_t mode);

/*
 * @function membus_module_watchpoints_alloc_with_address_range
 *
 * Allocate a new watchpoint membus module registered to a limited
 * range of addresses.
 */
membus_module_ref membus_module_watchpoints_alloc_with_address_range(
            membus_module_mode_t mode, memory_addr_range_t addr_range);

/*
 * @function membus_watchpoint_register_generic
 *
 * Register callback function the_callback as a watchpoint for on_events events
 * occuring w.r.t. any address in the_module membus module.  The context argument
 * is an opaque pointer that is passed to the_callback when it is invoked.
 *
 * If the watchpoint is successfully registered the membus_watchpoint_ref that
 * references it is returned.  On failure, NULL is returned.
 */
membus_watchpoint_ref membus_module_watchpoints_register_generic(
        membus_module_ref the_module, membus_watchpoint_event_t on_events,
        membus_watchpoint_callback_t the_callback, const void *context);

/*
 * @function membus_watchpoint_register_addr
 *
 * Register callback function the_callback as a watchpoint for on_events events
 * occuring w.r.t. addr in the_module membus module.  The context argument is an
 * opaque pointer that is passed to the_callback when it is invoked.
 *
 * If the watchpoint is successfully registered the membus_watchpoint_ref that
 * references it is returned.  On failure, NULL is returned.
 */
membus_watchpoint_ref membus_module_watchpoints_register_addr(
        membus_module_ref the_module, membus_watchpoint_event_t on_events,
        membus_watchpoint_callback_t the_callback, const void *context,
        uint16_t addr);

/*
 * @function membus_watchpoint_register_addr_range
 *
 * Register callback function the_callback as a watchpoint for on_events events
 * occuring w.r.t. addresses in the range addr_range in the_module membus module.
 * The context argument is an opaque pointer that is passed to the_callback when
 * it is invoked.
 *
 * If the watchpoint is successfully registered the membus_watchpoint_ref that
 * references it is returned.  On failure, NULL is returned.
 */
membus_watchpoint_ref membus_module_watchpoints_register_addr_range(
        membus_module_ref the_module, membus_watchpoint_event_t on_events,
        membus_watchpoint_callback_t the_callback, const void *context,
        memory_addr_range_t addr_range);

/*
 * @function membus_watchpoint_get_subtype
 *
 * Get the subtype associated with a watchpoint reference.
 */
membus_watchpoint_subtype_t membus_watchpoint_get_subtype(membus_watchpoint_ref the_watchpoint);

/*
 * @function membus_watchpoint_get_events
 *
 * Get the event mask associated with a watchpoint reference.
 */
membus_watchpoint_event_t membus_watchpoint_get_events(membus_watchpoint_ref the_watchpoint);

/*
 * @function membus_watchpoint_set_events
 *
 * Merge/set the event mask for the_watchpoint using the on_events mask.  If
 * membus_watchpoint_event_merge_at_register is set, the new event bits are OR'ed
 * into the existing mask; otherwise, the mask is set outright.
 */
void membus_watchpoint_set_events(membus_watchpoint_ref the_watchpoint, membus_watchpoint_event_t on_events);

/*
 * @function membus_watchpoint_unregister
 *
 * Remove the_watchpoint from the membus module in which it is registered.
 */
void membus_module_watchpoints_unregister(membus_watchpoint_ref the_watchpoint);

#endif /* __MEMBUS_MODULE_WATCHPOINTS_H__ */
