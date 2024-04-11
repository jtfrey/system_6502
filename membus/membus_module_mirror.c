
#include "membus_module_mirror.h"
#include "membus_private.h"

typedef struct membus_module_mirror {
    membus_module_t     header;
    membus_module_ref   original_module;
} membus_module_mirror_t;

//

void
__membus_module_mirror_free(
    membus_module_ref	module
)
{
    membus_module_mirror_t  *MODULE = (membus_module_mirror_t*)module;
    membus_module_release(MODULE->original_module);
}

//

membus_module_op_result_t
__membus_module_mirror_read_addr(
    membus_module_ref	module,
    uint16_t            addr,
    uint8_t             *value
)
{
    membus_module_mirror_t  *MODULE = (membus_module_mirror_t*)module;
    membus_module_t         *OTHER_MODULE = (membus_module_t*)MODULE->original_module;
    
    /* Map the incoming address into the original's space: */
    addr = (addr - MODULE->header.addr_range.addr_lo) + OTHER_MODULE->addr_range.addr_lo;
    
    return OTHER_MODULE->read_callback(MODULE->original_module, addr, value);
}

//

membus_module_op_result_t
__membus_module_mirror_write_addr(
    membus_module_ref	module,
    uint16_t            addr,
    uint8_t             value
)
{
    membus_module_mirror_t  *MODULE = (membus_module_mirror_t*)module;
    membus_module_t         *OTHER_MODULE = (membus_module_t*)MODULE->original_module;
    
    /* Map the incoming address into the original's space: */
    addr = (addr - MODULE->header.addr_range.addr_lo) + OTHER_MODULE->addr_range.addr_lo;
    
    return OTHER_MODULE->write_callback(MODULE->original_module, addr, value);
}

//

static const membus_module_t membus_module_mirror_header = {
                .module_id = "MIRROR",
                .addr_range = { .addr_lo = 0x0000, .addr_len = 0x0000 },
                .free_callback = __membus_module_mirror_free,
                .read_callback = __membus_module_mirror_read_addr,
                .write_callback = __membus_module_mirror_write_addr
            };

//

membus_module_ref
membus_module_mirror_alloc(
    membus_module_ref   original_module,
    uint16_t            mirror_base_addr
)
{
    membus_module_t         *ORIGINAL_MODULE = (membus_module_t*)original_module;
    membus_module_mirror_t  *new_module = NULL;
    uint32_t                mirror_end_addr = mirror_base_addr + ORIGINAL_MODULE->addr_range.addr_len - 1;
    
    if ( mirror_end_addr < 0x00010000 ) {
        new_module = (membus_module_mirror_t*)malloc(sizeof(membus_module_mirror_t));
    
        if ( new_module ) {
            new_module->header = membus_module_mirror_header;
            new_module->header.addr_range = memory_addr_range_with_lo_and_hi(mirror_base_addr, mirror_end_addr);
            new_module->original_module = membus_module_retain(original_module);
        }
    }
    return (membus_module_ref)new_module;
}
