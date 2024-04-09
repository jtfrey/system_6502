
#include "membus.h"

#include <ctype.h>

//

const memory_addr_range_t memory_addr_range_undef = {
                                .addr_lo = 0x0000,
                                .addr_len = 0x0000
                            };

//

bool
memory_addr_range_do_intersect(
    memory_addr_range_t *r1,
    memory_addr_range_t *r2
)
{
    if ( r2->addr_len && r1->addr_len ) {
        if ( r2->addr_lo <= r1->addr_lo ) {
            /* r2 ordered before r1 */
            uint16_t    r2end = r2->addr_lo + r2->addr_len - 1;
            
            return (r2end >= r1->addr_lo);
        } else {
            /* r1 ordered before r2 */
            uint16_t    r1end = r1->addr_lo + r1->addr_len - 1;
            
            return (r1end >= r2->addr_lo);
        }
    }
    return false;
}

//

memory_addr_range_t*
memory_addr_range_intersection(
    memory_addr_range_t *r1,
    memory_addr_range_t *r2,
    memory_addr_range_t *rOut
)
{
    if ( r2->addr_len && r1->addr_len ) {
        if ( r2->addr_lo <= r1->addr_lo ) {
            /* r2 ordered before r1 */
            uint16_t    r1end = r1->addr_lo + r1->addr_len - 1;
            uint16_t    r2end = r2->addr_lo + r2->addr_len - 1;
            
            if (r2end >= r1->addr_lo) {
                rOut->addr_lo = r1->addr_lo;
                rOut->addr_len = ((r2end <= r1end) ? r2end : r1end) - r1->addr_lo + 1;
                return rOut;
            }
        } else {
            /* r1 ordered before r2 */
            uint16_t    r1end = r1->addr_lo + r1->addr_len - 1;
            uint16_t    r2end = r2->addr_lo + r2->addr_len - 1;
            
            if (r1end >= r2->addr_lo) {
                rOut->addr_lo = r2->addr_lo;
                rOut->addr_len = ((r1end <= r2end) ? r1end : r2end) - r2->addr_lo + 1;
                return rOut;
            }
        }
    }
    *rOut = memory_addr_range_undef;
    return rOut;
}

//

memory_addr_range_t*
memory_addr_range_union(
    memory_addr_range_t *r1,
    memory_addr_range_t *r2,
    memory_addr_range_t *rOut
)
{
    if ( r2->addr_len && r1->addr_len ) {
        if ( r2->addr_lo <= r1->addr_lo ) {
            /* r2 ordered before r1 */
            uint16_t    r1end = r1->addr_lo + r1->addr_len - 1;
            uint16_t    r2end = r2->addr_lo + r2->addr_len - 1;
            
            if (r2end >= r1->addr_lo) {
                rOut->addr_lo = r2->addr_lo;
                rOut->addr_len = ((r2end <= r1end) ? r1end : r2end) - r2->addr_lo + 1;
                return rOut;
            }
        } else {
            /* r1 ordered before r2 */
            uint16_t    r1end = r1->addr_lo + r1->addr_len - 1;
            uint16_t    r2end = r2->addr_lo + r2->addr_len - 1;
            
            if (r1end >= r2->addr_lo) {
                rOut->addr_lo = r1->addr_lo;
                rOut->addr_len = ((r1end <= r2end) ? r2end : r1end) - r1->addr_lo + 1;
                return rOut;
            }
        }
    }
    *rOut = memory_addr_range_undef;
    return rOut;
}

//

typedef struct membus_module_node {
    int                         tier;
    membus_module_t             *module;
    struct membus_module_node   *link;
} membus_module_node_t;

//

membus_t*
membus_alloc(void)
{
    membus_t    *new_bus = (membus_t*)malloc(sizeof(membus_t));
    
    if ( new_bus ) {
        new_bus->modules = NULL;
    }
    return new_bus;
}

//

void
membus_free(
    membus_t    *the_membus
)
{
    membus_module_node_t    *node = (membus_module_node_t*)the_membus->modules, *next;
    
    while ( node ) {
        next = node->link;
        if ( node->module && node->module->free_callback ) {
            node->module->free_callback(node->module);
        } else {
            free((void*)node->module);
        }
        free((void*)node);
        node = next;
    }
    free((void*)the_membus);
}

//

bool
membus_register_module(
    membus_t        *the_membus,
    int             tier,
    membus_module_t *the_module
)
{
    membus_module_node_t    *node = (membus_module_node_t*)the_membus->modules;
    membus_module_node_t    *prev = NULL;
    membus_module_node_t    *new_node;
    
    while ( node && (node->tier < tier) ) {
        prev = node;
        node = node->link;
    }
    if ( node && (tier == node->tier) ) {
        while ( node && (tier == node->tier) && (the_module->addr_range.addr_lo > node->module->addr_range.addr_lo) ) {
            prev = node;
            node = node->link;
        }
    }
    new_node = (membus_module_node_t*)malloc(sizeof(membus_module_node_t));
    if ( new_node ) {
        new_node->link = node;
        if ( prev ) {
            prev->link = new_node;
        } else {
            the_membus->modules = new_node;
        }
        new_node->tier = tier;
        new_node->module = the_module;
        return true;
    }
    return false;
}

//

uint8_t
membus_read_addr(
    membus_t    *the_membus,
    uint16_t    addr
)
{
    uint8_t     value = 0x00;
    bool        did_read_value = false;
    
    switch ( addr ) {
        case MEMORY_ADDR_NMI_VECTOR:
            did_read_value = true;
            value = (the_membus->nmi_vector & 0x00FF);
            break;
        case MEMORY_ADDR_NMI_VECTOR + 1:
            did_read_value = true;
            value = (the_membus->nmi_vector & 0xFF00) >> 8;
            break;
        case MEMORY_ADDR_RES_VECTOR:
            did_read_value = true;
            value = (the_membus->res_vector & 0x00FF);
            break;
        case MEMORY_ADDR_RES_VECTOR + 1:
            did_read_value = true;
            value = (the_membus->res_vector & 0xFF00) >> 8;
            break;
        case MEMORY_ADDR_IRQ_VECTOR:
            did_read_value = true;
            value = (the_membus->irq_vector & 0x00FF);
            break;
        case MEMORY_ADDR_IRQ_VECTOR + 1:
            did_read_value = true;
            value = (the_membus->irq_vector & 0xFF00) >> 8;
            break;
        default: {
            membus_module_node_t    *node = (membus_module_node_t*)the_membus->modules;
            
            while ( node && ! memory_addr_range_does_include(&node->module->addr_range, addr) )
                node = node->link;
            if ( node && node->module->read_callback )
                did_read_value = node->module->read_callback(node->module, addr, &value);
            break;
        }   
    }
    return value;
}

//

void
membus_write_addr(
    membus_t    *the_membus,
    uint16_t    addr,
    uint8_t     value
)
{
    bool        did_write_value = false;
    
    switch ( addr ) {
        case MEMORY_ADDR_NMI_VECTOR:
            did_write_value = true;
            the_membus->nmi_vector = (the_membus->nmi_vector & 0xFF00) | value;
            break;
        case MEMORY_ADDR_NMI_VECTOR + 1:
            did_write_value = true;
            the_membus->nmi_vector = (the_membus->nmi_vector & 0x00FF) | (value << 8);
            break;
        case MEMORY_ADDR_RES_VECTOR:
            did_write_value = true;
            the_membus->nmi_vector = (the_membus->res_vector & 0xFF00) | value;
            break;
        case MEMORY_ADDR_RES_VECTOR + 1:
            did_write_value = true;
            the_membus->nmi_vector = (the_membus->res_vector & 0x00FF) | (value << 8);
            break;
        case MEMORY_ADDR_IRQ_VECTOR:
            did_write_value = true;
            the_membus->nmi_vector = (the_membus->irq_vector & 0xFF00) | value;
            break;
        case MEMORY_ADDR_IRQ_VECTOR + 1:
            did_write_value = true;
            the_membus->nmi_vector = (the_membus->irq_vector & 0x00FF) | (value << 8);
            break;
        default: {
            membus_module_node_t    *node = (membus_module_node_t*)the_membus->modules;
            
            while ( node && ! memory_addr_range_does_include(&node->module->addr_range, addr) )
                node = node->link;
            if ( node && node->module->read_callback )
                did_write_value = node->module->write_callback(node->module, addr, value);
            break;
        }   
    }
}


