
#include "membus_module_std64k.h"

typedef struct membus_module_std64k {
    membus_module_t     header;
    union {
        memory_page_t   PAGES[0x100];
        uint8_t         BYTES[0x100 * 0x100];
    } RAM;
} membus_module_std64k_t;

//

bool
__membus_module_std64k_read_addr(
    const void  *module,
    uint16_t    addr,
    uint8_t     *value
)
{
    membus_module_std64k_t  *MODULE = (membus_module_std64k_t*)module;
    
    *value = MODULE->RAM.BYTES[addr];
    return true;
}

//

bool
__membus_module_std64k_write_addr(
    const void  *module,
    uint16_t    addr,
    uint8_t     value
)
{
    membus_module_std64k_t  *MODULE = (membus_module_std64k_t*)module;
    
    MODULE->RAM.BYTES[addr] = value;
    return true;
}

//

static const membus_module_t membus_module_std64k_header = {
                .module_id = "STD64K",
                .addr_range = { .addr_lo = 0x0000, .addr_len = 0xFFFF },
                .free_callback = NULL,
                .read_callback = __membus_module_std64k_read_addr,
                .write_callback = __membus_module_std64k_write_addr
            };

//

membus_module_t*
membus_module_std64k_alloc(void)
{
    membus_module_std64k_t  *new_module = (membus_module_std64k_t*)malloc(sizeof(membus_module_std64k_t));
    
    if ( new_module ) new_module->header = membus_module_std64k_header;
    return (membus_module_t*)new_module;
}
