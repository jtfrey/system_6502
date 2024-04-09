
#include "membus_module_byte.h"

typedef struct membus_module_byte {
    membus_module_t     header;
    uint16_t            flags_and_byte;
} membus_module_byte_t;

//

bool
__membus_module_byte_read_addr(
    const void  *module,
    uint16_t    addr,
    uint8_t     *value
)
{
    membus_module_byte_t   *MODULE = (membus_module_byte_t*)module;
    
    if ( ((MODULE->flags_and_byte >> 8) & membus_module_mode_mask) != membus_module_mode_wo ) {
        *value = MODULE->flags_and_byte & 0x00FF;
        return true;
    }
    return false;
}

//

bool
__membus_module_byte_write_addr(
    const void  *module,
    uint16_t    addr,
    uint8_t     value
)
{
    membus_module_byte_t   *MODULE = (membus_module_byte_t*)module;
    
    if ( ((MODULE->flags_and_byte >> 8) & membus_module_mode_mask) != membus_module_mode_ro ) {
        MODULE->flags_and_byte = (MODULE->flags_and_byte & 0xFF00) | value;
        return true;
    }
    return false;
}

//

static const membus_module_t membus_module_byte_header = {
                .module_id = "BYTE",
                .addr_range = { .addr_lo = 0x0000, .addr_len = 0x0000 },
                .free_callback = NULL,
                .read_callback = __membus_module_byte_read_addr,
                .write_callback = __membus_module_byte_write_addr
            };

//

membus_module_t*
membus_module_byte_alloc(
    membus_module_mode_t    mode,
    uint16_t                addr
)
{
    membus_module_byte_t    *new_module = (membus_module_byte_t*)malloc(sizeof(membus_module_byte_t));
    
    if ( new_module ) {
        new_module->header = membus_module_byte_header;
        new_module->header.addr_range = memory_addr_range_with_lo_and_len(addr, 1);
        new_module->flags_and_byte = (mode << 8) | 0xCA;
    }
    return (membus_module_t*)new_module;
}
