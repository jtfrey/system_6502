
#ifndef __MEMBUS_PRIVATE_H__
#define __MEMBUS_PRIVATE_H__

#include "membus.h"

typedef struct membus_module {
    unsigned int                    ref_count;
    const char                      *module_id;
    memory_addr_range_t             addr_range;
    membus_module_free_fn_t         free_callback;
    membus_module_read_addr_t       read_callback;
    membus_module_write_addr_t      write_callback;
} membus_module_t;

#endif /* __MEMBUS_PRIVATE_H__ */
