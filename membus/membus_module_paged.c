
#include "membus_module_paged.h"

typedef struct membus_module_paged {
    membus_module_t         header;
    membus_module_mode_t    mode;
    memory_page_t           pages[1];
} membus_module_paged_t;

//

bool
__membus_module_paged_read_addr(
    const void  *module,
    uint16_t    addr,
    uint8_t     *value
)
{
    membus_module_paged_t   *MODULE = (membus_module_paged_t*)module;
    
    if ( (MODULE->mode & membus_module_mode_mask) != membus_module_mode_wo ) {
        addr -= MODULE->header.addr_range.addr_lo;
        *value = MODULE->pages[addr >> 8][addr & 0x00FF];
        return true;
    }
    return false;
}

//

bool
__membus_module_paged_write_addr(
    const void  *module,
    uint16_t    addr,
    uint8_t     value
)
{
    membus_module_paged_t   *MODULE = (membus_module_paged_t*)module;
    
    if ( (MODULE->mode & membus_module_mode_mask) != membus_module_mode_ro ) {
        addr -= MODULE->header.addr_range.addr_lo;
        MODULE->pages[addr >> 8][addr & 0x00FF] = value;
        return true;
    }
    return false;
}

//

static const membus_module_t membus_module_paged_header = {
                .module_id = "PAGED",
                .addr_range = { .addr_lo = 0x0000, .addr_len = 0x0000 },
                .free_callback = NULL,
                .read_callback = __membus_module_paged_read_addr,
                .write_callback = __membus_module_paged_write_addr
            };

//

membus_module_t*
membus_module_paged_alloc(
    membus_module_mode_t    mode,
    uint8_t                 base_page,
    uint8_t                 n_pages
)
{
    membus_module_paged_t   *new_module = NULL;
    size_t                  full_size = sizeof(membus_module_paged_t);
    uint16_t                end_page = base_page + n_pages;
    
    if ( end_page > 0x0100 ) n_pages = 0x0100 - base_page;
    full_size += sizeof(memory_page_t) * (n_pages - 1);
    
    new_module = (membus_module_paged_t*)malloc(full_size);
    if ( new_module ) {
        new_module->header = membus_module_paged_header;
        new_module->mode = mode;
        new_module->header.addr_range = memory_addr_range_with_lo_and_len(
                                                (uint16_t)base_page << 8,
                                                (uint16_t)n_pages << 8
                                            );
    }
    return (membus_module_t*)new_module;
}
