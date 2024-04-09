
#include "membus_module_word.h"

typedef struct membus_module_word {
    membus_module_t     header;
    uint32_t            flags_and_word;
} membus_module_word_t;

//

bool
__membus_module_word_read_addr(
    const void  *module,
    uint16_t    addr,
    uint8_t     *value
)
{
    membus_module_word_t   *MODULE = (membus_module_word_t*)module;
    
    if ( ((MODULE->flags_and_word >> 16) & membus_module_mode_mask) != membus_module_mode_wo ) {
        addr = (addr - MODULE->header.addr_range.addr_lo) << 3;
        *value = (MODULE->flags_and_word & (0x00FF << addr)) >> addr;
        return true;
    }
    return false;
}

//

bool
__membus_module_word_write_addr(
    const void  *module,
    uint16_t    addr,
    uint8_t     value
)
{
    membus_module_word_t   *MODULE = (membus_module_word_t*)module;
    
    if ( ((MODULE->flags_and_word >> 16) & membus_module_mode_mask) != membus_module_mode_ro ) {
        addr = (addr - MODULE->header.addr_range.addr_lo) << 3;
        
        MODULE->flags_and_word = (MODULE->flags_and_word & 0xFFFF0000) | (value << addr);
        return true;
    }
    return false;
}

//

static const membus_module_t membus_module_word_header = {
                .module_id = "WORD",
                .addr_range = { .addr_lo = 0x0000, .addr_len = 0x0000 },
                .free_callback = NULL,
                .read_callback = __membus_module_word_read_addr,
                .write_callback = __membus_module_word_write_addr
            };

//

membus_module_t*
membus_module_word_alloc(
    membus_module_mode_t    mode,
    uint16_t                addr
)
{
    membus_module_word_t    *new_module = (membus_module_word_t*)malloc(sizeof(membus_module_word_t));
    
    if ( new_module ) {
        new_module->header = membus_module_word_header;
        new_module->header.addr_range = memory_addr_range_with_lo_and_len(addr, 2);
        new_module->flags_and_word = (mode << 16) | 0xFACE;
    }
    return (membus_module_t*)new_module;
}
