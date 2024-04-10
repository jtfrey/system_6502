
#include "membus.h"

#include <ctype.h>

//


#include "membus_private.h"

unsigned int
membus_module_get_ref_count(
    membus_module_ref   module
)
{
    return module->ref_count;
}

membus_module_ref
membus_module_retain(
    membus_module_ref   module
)
{
    module->ref_count++;
    return module;
}

void
membus_module_release(
    membus_module_ref   module
)
{
    if ( ! --module->ref_count ) {
        if ( module->free_callback ) module->free_callback(module);
        free((void*)module);
    }
}

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

typedef struct membus_module_module_node {
    membus_module_t                     *module;
    struct membus_module_module_node    *link;
} membus_module_module_node_t;

membus_module_module_node_t*
__membus_module_module_node_alloc(
    membus_module_t     *module
)
{
    membus_module_module_node_t *new_node = (membus_module_module_node_t*)malloc(sizeof(membus_module_module_node_t));
    
    if ( new_node ) {
        new_node->module = membus_module_retain(module);
        new_node->link = NULL;
    }
    return new_node;
}

void
__membus_module_module_node_free(
    membus_module_module_node_t *node
)
{
    membus_module_release(node->module);
    free((void*)node);
}

//

typedef struct membus_module_tier_node {
    int                             tier;
    membus_module_module_node_t     *modules;
    struct membus_module_tier_node  *link;
} membus_module_tier_node_t;

membus_module_tier_node_t*
__membus_module_tier_node_alloc(
    int             tier
)
{
    membus_module_tier_node_t   *new_node = (membus_module_tier_node_t*)malloc(sizeof(membus_module_tier_node_t));
    
    if ( new_node ) {
        new_node->tier = tier;
        new_node->modules = NULL;
        new_node->link = NULL;
    }
    return new_node;
}

void
__membus_module_tier_node_free(
    membus_module_tier_node_t   *node
)
{
    membus_module_module_node_t *modules = node->modules, *next;
    
    while ( modules ) {
        next = modules->link;
        __membus_module_module_node_free(modules);
        modules = next;
    }
    free((void*)node);
}

//

membus_t*
membus_alloc(void)
{
    membus_t    *new_bus = (membus_t*)malloc(sizeof(membus_t));
    
    if ( new_bus ) {
        new_bus->pre_op = new_bus->modules = new_bus->post_op = NULL;
        new_bus->nmi_vector = 0x0FF0;
        new_bus->res_vector = 0xF00F;
        new_bus->irq_vector = 0xCAFE;
    }
    return new_bus;
}

//

void
membus_free(
    membus_t    *the_membus
)
{
    membus_module_tier_node_t   *tiers = (membus_module_tier_node_t*)the_membus->modules, *next;
    
    if ( the_membus->pre_op ) __membus_module_tier_node_free((membus_module_tier_node_t*)the_membus->pre_op);
    while ( tiers ) {
        next = tiers->link;
        __membus_module_tier_node_free(tiers);
        tiers = next;
    }
    if ( the_membus->post_op ) __membus_module_tier_node_free((membus_module_tier_node_t*)the_membus->post_op);
    free((void*)the_membus);
}

//

bool
membus_register_module(
    membus_t            *the_membus,
    int                 tier,
    membus_module_ref   the_module
)
{
    membus_module_module_node_t *new_node = __membus_module_module_node_alloc(the_module);
    bool                        did_register = false;
    
    if ( new_node ) {
        if ( tier == MEMBUS_TIER_PRE_OP ) {
            if ( ! the_membus->pre_op ) {
                /* Add a new tier: */
                the_membus->pre_op = __membus_module_tier_node_alloc(MEMBUS_TIER_PRE_OP);
            }
            if ( the_membus->pre_op ) {
                membus_module_module_node_t *prev_module = NULL, *modules = ((membus_module_tier_node_t*)the_membus->pre_op)->modules;
        
                while ( modules && (modules->module->addr_range.addr_lo < the_module->addr_range.addr_lo) ) {
                    prev_module = modules;
                    modules = modules->link;
                }
                if ( prev_module ) {
                    /* Insert into the chain: */
                    new_node->link = prev_module->link;
                    prev_module->link = new_node;
                } else {
                    /* New root in the chain: */
                    new_node->link = ((membus_module_tier_node_t*)the_membus->pre_op)->modules;
                    ((membus_module_tier_node_t*)the_membus->pre_op)->modules = new_node;
                }
                did_register = true;
            }
        } else if ( tier == MEMBUS_TIER_POST_OP ) {
            if ( ! the_membus->post_op ) {
                /* Add a new tier: */
                the_membus->post_op = __membus_module_tier_node_alloc(MEMBUS_TIER_POST_OP);
            }
            if ( the_membus->post_op ) {
                membus_module_module_node_t *prev_module = NULL, *modules = ((membus_module_tier_node_t*)the_membus->post_op)->modules;
        
                while ( modules && (modules->module->addr_range.addr_lo < the_module->addr_range.addr_lo) ) {
                    prev_module = modules;
                    modules = modules->link;
                }
                if ( prev_module ) {
                    /* Insert into the chain: */
                    new_node->link = prev_module->link;
                    prev_module->link = new_node;
                } else {
                    /* New root in the chain: */
                    new_node->link = ((membus_module_tier_node_t*)the_membus->post_op)->modules;
                    ((membus_module_tier_node_t*)the_membus->post_op)->modules = new_node;
                }
                did_register = true;
            }
        } else {
            membus_module_tier_node_t   *prev_tier = NULL,
                                        *tiers = (membus_module_tier_node_t*)the_membus->modules;
            bool                        add_tier = true;
            
            /* Find the tier which is >= the tier to which we're adding: */
            while ( tiers && (tiers->tier < tier) ) {
                prev_tier = tiers;
                tiers = tiers->link;
            }
    
            /* Did we find a tier to which to add the_module? */
            if ( tiers && (tiers->tier == tier) ) {
                membus_module_module_node_t *prev_module = NULL, *modules = tiers->modules;
        
                while ( modules && (modules->module->addr_range.addr_lo < the_module->addr_range.addr_lo) ) {
                    prev_module = modules;
                    modules = modules->link;
                }
                if ( prev_module ) {
                    /* Insert into the chain: */
                    new_node->link = prev_module->link;
                    prev_module->link = new_node;
                } else {
                    /* New root in the chain: */
                    new_node->link = tiers->modules;
                    tiers->modules = new_node;
                }
                add_tier = false;
                did_register = true;
            }

            if ( add_tier ) {
                /* Add a new tier: */
                membus_module_tier_node_t   *new_tier = __membus_module_tier_node_alloc(tier);
    
                if ( new_tier ) {
                    new_tier->modules = new_node;
                    if ( prev_tier ) {
                        /* Insert new tier between prev tier and its next tier: */
                        new_tier->link = prev_tier->link;
                        prev_tier->link = new_tier;
                    } else {
                        /* New root tier: */
                        the_membus->modules = (const void*)new_tier;
                    }
                    did_register = true;
                }
            }
            if ( ! did_register ) __membus_module_module_node_free(new_node);
        }
    }
    return did_register;
}

//

uint8_t
membus_read_addr(
    membus_t    *the_membus,
    uint16_t    addr
)
{
    uint8_t     value = 0x00;
    bool        did_read_value = false, is_looping = true;
    
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
            membus_module_tier_node_t   *tiers = (membus_module_tier_node_t*)the_membus->modules;
            
            if ( the_membus->pre_op ) {
                membus_module_module_node_t *modules = ((membus_module_tier_node_t*)the_membus->pre_op)->modules;
                
                while ( modules && (modules->module->addr_range.addr_lo <= addr) ) {
                    if ( memory_addr_range_does_include(&modules->module->addr_range, addr) && modules->module->read_callback )
                        modules->module->read_callback(modules->module, addr, NULL);
                    modules = modules->link;
                }
            }
            while ( is_looping && ! did_read_value && tiers ) {
                membus_module_module_node_t *modules = tiers->modules;
                
                while ( is_looping && ! did_read_value && modules && (modules->module->addr_range.addr_lo <= addr) ) {
                    if ( memory_addr_range_does_include(&modules->module->addr_range, addr) && modules->module->read_callback ) {
                        membus_module_op_result_t   rc = modules->module->read_callback(modules->module, addr, &value);
                        is_looping = (rc != membus_module_op_result_not_accepted);
                        did_read_value = (rc == membus_module_op_result_accepted);
                    }
                    modules = modules->link;
                }
                tiers = tiers->link;
            }
            if ( the_membus->post_op ) {
                membus_module_module_node_t *modules = ((membus_module_tier_node_t*)the_membus->post_op)->modules;
                
                while ( modules && (modules->module->addr_range.addr_lo <= addr) ) {
                    if ( memory_addr_range_does_include(&modules->module->addr_range, addr) && modules->module->read_callback ) {
                        uint8_t     value_copy = value;
                        modules->module->read_callback(modules->module, addr, &value_copy);
                    }
                    modules = modules->link;
                }
            }
            break;
        }   
    }
#ifdef ENABLE_MEMBUS_CACHE
    if ( did_read_value ) membus_rcache_push(the_membus, value);
#endif
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
    bool        did_write_value = false, is_looping = true;
    
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
            membus_module_tier_node_t   *tiers = (membus_module_tier_node_t*)the_membus->modules;
            
            if ( the_membus->pre_op ) {
                membus_module_module_node_t *modules = ((membus_module_tier_node_t*)the_membus->pre_op)->modules;
                
                while ( modules && (modules->module->addr_range.addr_lo <= addr) ) {
                    if ( memory_addr_range_does_include(&modules->module->addr_range, addr) && modules->module->write_callback )
                        modules->module->write_callback(modules->module, addr, 0);
                    modules = modules->link;
                }
            }
            while ( is_looping && ! did_write_value && tiers ) {
                membus_module_module_node_t *modules = tiers->modules;
                
                while ( is_looping && ! did_write_value && modules && (modules->module->addr_range.addr_lo <= addr) ) {
                    if ( memory_addr_range_does_include(&modules->module->addr_range, addr) && modules->module->write_callback ) {
                        membus_module_op_result_t   rc = modules->module->write_callback(modules->module, addr, value);
                        is_looping = (rc != membus_module_op_result_not_accepted);
                        did_write_value = (rc == membus_module_op_result_accepted);
                    }
                    modules = modules->link;
                }
                tiers = tiers->link;
            }
            if ( the_membus->post_op ) {
                membus_module_module_node_t *modules = ((membus_module_tier_node_t*)the_membus->post_op)->modules;
                
                while ( modules && (modules->module->addr_range.addr_lo <= addr) ) {
                    if ( memory_addr_range_does_include(&modules->module->addr_range, addr) && modules->module->write_callback )
                        modules->module->write_callback(modules->module, addr, value);
                    modules = modules->link;
                }
            }
            break;
        }   
    }
#ifdef ENABLE_MEMBUS_CACHE
    if ( did_write_value ) membus_wcache_push(the_membus, value);
#endif
}

//

void
membus_write_byte_to_range(
    membus_t            *the_membus,
    memory_addr_range_t r,
    uint8_t             value
)
{
    uint32_t            addr = r.addr_lo, addr_end = r.addr_lo + r.addr_len;
    
    if ( addr_end > 0x00010000 ) addr_end = 0x0000FFFF;
    while ( addr < addr_end ) membus_write_addr(the_membus, addr++, value);
}

//

void
membus_write_word_to_range(
    membus_t            *the_membus,
    memory_addr_range_t r,
    uint16_t            value
)
{
    uint32_t            addr = r.addr_lo, addr_end = r.addr_lo + r.addr_len;
    int                 piece = 0;
    uint8_t             lo = value & 0x00FF, hi = value >> 8;
    
    if ( addr_end > 0x00010000 ) addr_end = 0x0000FFFF;
    while ( addr < addr_end ) {
        membus_write_addr(the_membus, addr++, (piece ? hi : lo) );
        piece = (piece + 1) % 2;
    }
}

//

ssize_t
membus_load_from_fd(
    membus_t            *the_membus,
    memory_addr_range_t r,
    int                 fd
)
{
    ssize_t             total_bytes = 0;
    uint32_t            addr = r.addr_lo, addr_end = r.addr_lo + r.addr_len;
    
    if ( addr_end > 0x00010000 ) addr_end = 0x00010000;
    while ( addr < addr_end ) {
        uint8_t         byte_buffer[4096], *p = byte_buffer;
        size_t          read_len = addr_end - addr + 1;
        ssize_t         bytes_read;
        
        if ( read_len > sizeof(byte_buffer) ) read_len = sizeof(byte_buffer);
        bytes_read = read(fd, byte_buffer, read_len);
        total_bytes += bytes_read;
        while ( bytes_read-- > 0 ) membus_write_addr(the_membus, addr++, *p++);
    }
    return total_bytes;
}

//

ssize_t
membus_load_from_stream(
    membus_t            *the_membus,
    memory_addr_range_t r,
    FILE                *stream
)
{
    ssize_t             total_bytes = 0;
    uint32_t            addr = r.addr_lo, addr_end = r.addr_lo + r.addr_len;
    
    if ( addr_end > 0x00010000 ) addr_end = 0x00010000;
    while ( addr < addr_end ) {
        uint8_t         byte_buffer[4096], *p = byte_buffer;
        size_t          read_len = addr_end - addr + 1;
        ssize_t         bytes_read;
        
        if ( read_len > sizeof(byte_buffer) ) read_len = sizeof(byte_buffer);
        bytes_read = fread(byte_buffer, 1, read_len, stream);
        total_bytes += bytes_read;
        while ( bytes_read-- > 0 ) membus_write_addr(the_membus, addr++, *p++);
    }
    return total_bytes;
}

//

ssize_t
membus_save_to_fd(
    membus_t            *the_membus,
    memory_addr_range_t r,
    int                 fd
)
{
    ssize_t             total_bytes = 0;
    uint32_t            addr = r.addr_lo, addr_end = r.addr_lo + r.addr_len;
    
    if ( addr_end > 0x00010000 ) addr_end = 0x00010000;
    while ( addr < addr_end ) {
        uint8_t         byte_buffer[4096], *p = byte_buffer;
        size_t          write_len = addr_end - addr + 1, xfer_len;
        ssize_t         bytes_written;
        
        if ( write_len > sizeof(byte_buffer) ) write_len = sizeof(byte_buffer);
        xfer_len = write_len;
        while ( xfer_len-- > 0 ) *p++ = membus_read_addr(the_membus, addr++);
        bytes_written = write(fd, byte_buffer, write_len);
        total_bytes += bytes_written;
    }
    return total_bytes;
}

//

ssize_t
membus_save_to_stream(
    membus_t            *the_membus,
    memory_addr_range_t r,
    FILE                *stream
)
{
    ssize_t             total_bytes = 0;
    uint32_t            addr = r.addr_lo, addr_end = r.addr_lo + r.addr_len;
    
    if ( addr_end > 0x00010000 ) addr_end = 0x00010000;
    while ( addr < addr_end ) {
        uint8_t         byte_buffer[4096], *p = byte_buffer;
        size_t          write_len = addr_end - addr + 1, xfer_len;
        ssize_t         bytes_written;
        
        if ( write_len > sizeof(byte_buffer) ) write_len = sizeof(byte_buffer);
        xfer_len = write_len;
        while ( xfer_len-- > 0 ) *p++ = membus_read_addr(the_membus, addr++);
        bytes_written = fwrite(byte_buffer, 1, write_len, stream);
        total_bytes += bytes_written;
    }
    return total_bytes;
}

//

int
membus_fprintf(
    membus_t            *the_membus,
    FILE                *stream,
    membus_dump_opts_t  opts, 
    memory_addr_range_t addr_range
)
{
    int                 n_tot = 0;
    uint32_t            addr = addr_range.addr_lo, addr_max = addr_range.addr_lo + addr_range.addr_len;
    char                *out_buffer_ptr, out_buffer[1024];
    size_t              out_buffer_len;
    int                 is_compact = ((opts & membus_dump_opts_compact) == membus_dump_opts_compact);
    
    if ( (opts & membus_dump_opts_8byte_width) == membus_dump_opts_8byte_width ) {
        while ( addr < addr_max ) {
            char    chars[9];
            int     l = 0, n;

            out_buffer_ptr = out_buffer;
            out_buffer_len = sizeof(out_buffer);

            n = snprintf(out_buffer_ptr, out_buffer_len, is_compact ? "%04hX:" : "%04hX : ", addr);
            out_buffer_ptr += n, out_buffer_len -= n;

            while ( (l < 8) && (addr < addr_max) ) {
                uint8_t     b = membus_read_addr(the_membus, addr++);
                
                n = snprintf(out_buffer_ptr, out_buffer_len, "%02X ", b);
                out_buffer_ptr += n, out_buffer_len -= n;

                chars[l++] = (isprint(b) ? b : '.');
            }
            chars[l] = '\0';
            while ( l < 8 ) {
                n = snprintf(out_buffer_ptr, out_buffer_len, "   ");
                out_buffer_ptr += n, out_buffer_len -= n;
                l++;
            }
            n = snprintf(out_buffer_ptr, out_buffer_len, is_compact ? "%s" : "   %s", chars);
            n_tot += fprintf(stream, "%s\n", out_buffer);
        }
    } else {
        while ( addr < addr_max ) {
            char    chars[17];
            int     l = 0, n;

            out_buffer_ptr = out_buffer;
            out_buffer_len = sizeof(out_buffer);

            n = snprintf(out_buffer_ptr, out_buffer_len, is_compact ? "%04hX:" : "%04hX : ", addr);
            out_buffer_ptr += n, out_buffer_len -= n;

            while ( (l < 16) && (addr < addr_max) ) {
                uint8_t     b = membus_read_addr(the_membus, addr++);
                
                if ( l == 8 ) {
                    n = snprintf(out_buffer_ptr, out_buffer_len, is_compact ? " " : "   ");
                    out_buffer_ptr += n, out_buffer_len -= n;
                }
                n = snprintf(out_buffer_ptr, out_buffer_len, "%02X ", b);
                out_buffer_ptr += n, out_buffer_len -= n;

                chars[l++] = (isprint(b) ? b : '.');
            }
            chars[l] = '\0';
            while ( l < 16 ) {
                n = snprintf(out_buffer_ptr, out_buffer_len, (l == 8) ? (is_compact ? "    " : "      ") : "   ");
                out_buffer_ptr += n, out_buffer_len -= n;
                l++;
            }
            n = snprintf(out_buffer_ptr, out_buffer_len, is_compact ? "%s" : "   %s", chars);
            n_tot += fprintf(stream, "%s\n", out_buffer);
        }
    }
    return n_tot;
}

