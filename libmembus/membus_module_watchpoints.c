
#include "membus_module_watchpoints.h"
#include "membus_private.h"

//

typedef struct membus_module_watchpoints {
    membus_module_t                     header;
    membus_module_mode_t                mode;
    struct membus_watchpoint_generic    *generics;
    struct membus_watchpoint_addr_range *address_ranges;
    struct membus_watchpoint_addr       *addresses;
} membus_module_watchpoints_t;

//

typedef struct membus_watchpoint {
    membus_watchpoint_subtype_t     subtype;
    membus_module_watchpoints_t     *source;
    membus_watchpoint_event_t       on_events;
    membus_watchpoint_callback_t    callback_fn;
    const void*                     context;
} membus_watchpoint_t;

#define MEMBUS_WATCHPOINT_VALID_EVENTS  (membus_watchpoint_event_read | membus_watchpoint_event_write | membus_watchpoint_event_merge_at_register)

//

typedef struct membus_watchpoint_generic {
    membus_watchpoint_t                 base;
    struct membus_watchpoint_generic    *link;
} membus_watchpoint_generic_t;

//

membus_watchpoint_generic_t*
__membus_watchpoint_generic_alloc(void)
{
    membus_watchpoint_generic_t *wp = (membus_watchpoint_generic_t*)malloc(sizeof(membus_watchpoint_generic_t));
    
    if ( wp ) {
        wp->base.subtype = membus_watchpoint_subtype_generic;
        wp->base.source = NULL;
        wp->base.on_events = 0x00;
        wp->base.callback_fn = NULL;
        wp->base.context = NULL;
        wp->link = NULL;
    }
    return wp;
}

//

void
__membus_watchpoint_generic_free(
    membus_watchpoint_generic_t *wp
)
{
    free((void*)wp);
}

//

void
__membus_watchpoint_generic_recursive_free(
    membus_watchpoint_generic_t *wp
)
{
    membus_watchpoint_generic_t *next;
    
    while ( wp ) {
        next = wp->link;
        __membus_watchpoint_generic_free(wp);
        wp = next;
    }
}

//

membus_watchpoint_generic_t*
__membus_watchpoint_generic_register(
    membus_module_watchpoints_t     *source,
    membus_watchpoint_generic_t     **root,
    membus_watchpoint_event_t       on_events,
    membus_watchpoint_callback_t    callback_fn,
    const void                      *context
)
{
    membus_watchpoint_generic_t     *new_wp = __membus_watchpoint_generic_alloc();
    
    if ( new_wp ) {
        new_wp->base.source = source;
        new_wp->base.on_events = on_events;
        new_wp->base.callback_fn = callback_fn;
        new_wp->base.context = context;
        new_wp->link = *root;
        *root = new_wp;
    }
    return new_wp;
}

//

void
__membus_watchpoint_generic_unregister(
    membus_watchpoint_generic_t    *the_watchpoint
)
{
    membus_watchpoint_generic_t     *root = (membus_watchpoint_generic_t*)the_watchpoint->base.source->generics;
    membus_watchpoint_generic_t     *node = root, *prev;
    
    while ( node ) {
        if ( node == the_watchpoint ) {
            /* Root node? */
            if ( node == root ) {
                the_watchpoint->base.source->generics = node->link;
            } else {
                prev->link = node->link;
            }
            __membus_watchpoint_generic_free(node);
            break;
        }
        prev = node;
        node = node->link;
    }
}

//
////
//

typedef struct membus_watchpoint_addr_range {
    membus_watchpoint_t                 base;
    memory_addr_range_t                 addr_range;
    struct membus_watchpoint_addr_range *link;
} membus_watchpoint_addr_range_t;

//

membus_watchpoint_addr_range_t*
__membus_watchpoint_addr_range_alloc(void)
{
    membus_watchpoint_addr_range_t *wp = (membus_watchpoint_addr_range_t*)malloc(sizeof(membus_watchpoint_addr_range_t));
    
    if ( wp ) {
        wp->base.subtype = membus_watchpoint_subtype_address_range;
        wp->base.source = NULL;
        wp->base.on_events = 0x00;
        wp->base.callback_fn = NULL;
        wp->base.context = NULL;
        wp->addr_range = memory_addr_range_undef;
        wp->link = NULL;
    }
    return wp;
}

//

void
__membus_watchpoint_addr_range_free(
    membus_watchpoint_addr_range_t *wp
)
{
    free((void*)wp);
}

//

void
__membus_watchpoint_addr_range_recursive_free(
    membus_watchpoint_addr_range_t *wp
)
{
    membus_watchpoint_addr_range_t *next;
    
    while ( wp ) {
        next = wp->link;
        __membus_watchpoint_addr_range_free(wp);
        wp = next;
    }
}

//

membus_watchpoint_addr_range_t*
__membus_watchpoint_addr_range_find_addr(
    membus_watchpoint_addr_range_t  *wp,
    uint16_t                        addr
)
{
    while ( wp ) {
        if ( memory_addr_range_does_include(&wp->addr_range, addr) ) break;
        wp = wp->link;
    }
    return wp;
}

//

membus_watchpoint_addr_range_t*
__membus_watchpoint_addr_range_register(
    membus_module_watchpoints_t     *source,
    membus_watchpoint_addr_range_t  **root,
    membus_watchpoint_event_t       on_events,
    membus_watchpoint_callback_t    callback_fn,
    const void                      *context,
    memory_addr_range_t             *addr_range
)
{
    membus_watchpoint_addr_range_t  *new_wp = NULL;
    
    if ( addr_range->addr_len ) {
        new_wp = __membus_watchpoint_addr_range_alloc();
        if ( new_wp ) {
            new_wp->base.source = source;
            new_wp->base.on_events = on_events;
            new_wp->base.callback_fn = callback_fn;
            new_wp->base.context = context;
            new_wp->addr_range = *addr_range;
            new_wp->link = *root;
            *root = new_wp;
        }
    }
    return new_wp;
}

//

void
__membus_watchpoint_addr_range_unregister(
    membus_watchpoint_addr_range_t  *the_watchpoint
)
{
    membus_watchpoint_addr_range_t  *root = (membus_watchpoint_addr_range_t*)the_watchpoint->base.source->address_ranges;
    membus_watchpoint_addr_range_t  *node = root, *prev;
    
    while ( node ) {
        if ( node == the_watchpoint ) {
            /* Root node? */
            if ( node == root ) {
                the_watchpoint->base.source->address_ranges = node->link;
            } else {
                prev->link = node->link;
            }
            __membus_watchpoint_addr_range_free(node);
            break;
        }
        prev = node;
        node = node->link;
    }
}

//
////
//

typedef struct membus_watchpoint_addr {
    membus_watchpoint_t             base;
    uint16_t                        addr;
    struct membus_watchpoint_addr   *left, *right;
    struct membus_watchpoint_addr   *parent, *child;
} membus_watchpoint_addr_t;

//

membus_watchpoint_addr_t*
__membus_watchpoint_addr_alloc(void)
{
    membus_watchpoint_addr_t *wp = (membus_watchpoint_addr_t*)malloc(sizeof(membus_watchpoint_addr_t));
    
    if ( wp ) {
        wp->base.subtype = membus_watchpoint_subtype_address;
        wp->base.source = NULL;
        wp->base.on_events = 0x00;
        wp->base.callback_fn = NULL;
        wp->base.context = NULL;
        wp->addr = 0x0000;
        wp->left = wp->right = wp->parent = wp->child = NULL;
    }
    return wp;
}

//

void
__membus_watchpoint_addr_free(
    membus_watchpoint_addr_t *wp
)
{
    free((void*)wp);
}

//

void
__membus_watchpoint_addr_recursive_free(
    membus_watchpoint_addr_t *wp
)
{
    membus_watchpoint_addr_t *node = wp->child, *next;
    
    while ( node ) {
        next = node->child;
        __membus_watchpoint_addr_free(node);
        node = next;
    }
    
    if ( wp->left ) __membus_watchpoint_addr_recursive_free(wp->left);
    if ( wp->right ) __membus_watchpoint_addr_recursive_free(wp->right);
    
    __membus_watchpoint_addr_free(wp);
}

//

membus_watchpoint_addr_t*
__membus_watchpoint_addr_find_addr(
    membus_watchpoint_addr_t    *wp,
    uint16_t                    addr
)
{
    while ( wp ) {
        if ( wp->addr == addr ) break;
        if ( wp->addr > addr ) wp = wp->left;
        else wp = wp->right;
    }
    return wp;
}

//

membus_watchpoint_addr_t*
__membus_watchpoint_addr_register(
    membus_module_watchpoints_t     *source,
    membus_watchpoint_addr_t        **root,
    membus_watchpoint_event_t       on_events,
    membus_watchpoint_callback_t    callback_fn,
    const void                      *context,
    uint16_t                        addr
)
{
    membus_watchpoint_addr_t        *wp = NULL, *new_wp = NULL;
    
    if ( ! root || ! *root ) {
        new_wp = __membus_watchpoint_addr_alloc();
        if ( new_wp ) {
            new_wp->base.source = source;
            new_wp->base.on_events = on_events;
            new_wp->base.callback_fn = callback_fn;
            new_wp->base.context = context;
            new_wp->addr = addr;
        }
        if ( root ) *root = new_wp;
    } else {
        wp = *root;
        while ( wp ) {
            if ( wp->addr == addr ) {
                /* Does the callback already exist? */
                membus_watchpoint_addr_t *last_wp = NULL;
                
                while ( wp ) {
                    if ( wp->base.callback_fn == callback_fn ) {
                        if ( on_events & membus_watchpoint_event_merge_at_register )
                            wp->base.on_events |= (on_events & membus_watchpoint_event_all);
                        else
                            wp->base.on_events = (on_events & membus_watchpoint_event_all);
                        new_wp = wp;
                        break;
                    }
                    last_wp = wp;
                    wp = wp->child;
                }
                if ( ! wp ) {
                    new_wp = __membus_watchpoint_addr_alloc();
                    if ( new_wp ) {
                        new_wp->base.source = source;
                        new_wp->base.on_events = (on_events & membus_watchpoint_event_all);
                        new_wp->base.callback_fn = callback_fn;
                        new_wp->base.context = context;
                        new_wp->addr = addr;
                        new_wp->parent = last_wp;
                        last_wp->child = new_wp;
                    }
                }
                break;
            }
            else if ( wp->addr > addr ) {
                /* If there's a node to the left, go that way; otherwise,
                   link a new node to the left:
                 */
                if ( wp->left ) {
                    wp = wp->left;
                } else {
                    new_wp = __membus_watchpoint_addr_alloc();
                    if ( new_wp ) {
                        new_wp->base.source = source;
                        new_wp->base.on_events = (on_events & membus_watchpoint_event_all);
                        new_wp->base.callback_fn = callback_fn;
                        new_wp->base.context = context;
                        new_wp->addr = addr;
                        new_wp->parent = wp;
                        wp->left = new_wp;
                    }
                    break;
                }
            } else {
                /* If there's a node to the right, go that way; otherwise,
                   link a new node to the right:
                 */
                if ( wp->right ) {
                    wp = wp->right;
                } else {
                    new_wp = __membus_watchpoint_addr_alloc();
                    if ( new_wp ) {
                        new_wp->base.source = source;
                        new_wp->base.on_events = (on_events & membus_watchpoint_event_all);
                        new_wp->base.callback_fn = callback_fn;
                        new_wp->base.context = context;
                        new_wp->addr = addr;
                        new_wp->parent = wp;
                        wp->right = new_wp;
                    }
                    break;
                }
            }
        }
    }
    return new_wp;
}

//

void
__membus_watchpoint_addr_unregister(
    membus_watchpoint_addr_t    *the_watchpoint
)
{
    /* Are we the root of the tree? */
    if ( the_watchpoint->parent == NULL ) {
        /* If we have a common-address ancestry, then we can promote
           our first child to being root: */
        if ( the_watchpoint->child ) {
            if ( (the_watchpoint->child->left = the_watchpoint->left) )
                the_watchpoint->left->parent = the_watchpoint->child;
            if ( (the_watchpoint->child->right = the_watchpoint->right) )
                the_watchpoint->right->parent = the_watchpoint->child;
            the_watchpoint->child->parent = NULL;
            the_watchpoint->base.source->addresses = the_watchpoint->child;
        } else if ( the_watchpoint->left ) {
            if ( the_watchpoint->right ) {
                /* We need to rebalance the binary tree that was rooted at
                   watchpoint.  We must find the next-highest child node by
                   going to the watchpoint's rightmost leaf: */
                membus_watchpoint_addr_t *leaf = the_watchpoint->right;
                while ( leaf->right ) leaf = leaf->right;
                /* leaf now points to our replacement node: */
                if ( (leaf->left = the_watchpoint->left) )
                    the_watchpoint->left->parent = leaf;
                if ( (leaf->right = the_watchpoint->right) )
                    the_watchpoint->right->parent = leaf;
                leaf->parent = NULL;
                the_watchpoint->base.source->addresses = leaf;
            } else {
                /* Promote left node to our position: */
                the_watchpoint->left->parent = NULL;
                the_watchpoint->base.source->addresses = the_watchpoint->left;
            }
        } else if ( the_watchpoint->right ) {
            /* Promote right node to our position: */
            the_watchpoint->right->parent = NULL;
            the_watchpoint->base.source->addresses = the_watchpoint->right;
        } else {
            /* Leaf node: */
            the_watchpoint->base.source->addresses = NULL;
        }
    } else {
        /* Examine the parent to determine on which of its branches we descended: */
        if ( the_watchpoint->parent->child == the_watchpoint ) {
            /* Common-address ancestry.  Remove the node from the tree, pointing
               the parent to the watchpoint's child: */
            the_watchpoint->parent->child = the_watchpoint->child;
        }
        else if ( the_watchpoint->parent->left == the_watchpoint ) {
            /* We are the left-branch node off of the parent. If we have a common-address
               ancestry, then we can promote our first child to our position: */
            if ( the_watchpoint->child ) {
                if ( (the_watchpoint->child->left = the_watchpoint->left) )
                    the_watchpoint->left->parent = the_watchpoint->child;
                if ( (the_watchpoint->child->right = the_watchpoint->right) )
                    the_watchpoint->right->parent = the_watchpoint->child;
                the_watchpoint->child->parent = the_watchpoint->parent;
                the_watchpoint->parent->left = the_watchpoint->child;
            } else if ( the_watchpoint->left ) {
                if ( the_watchpoint->right ) {
                    /* We need to rebalance the binary tree that was rooted at
                       watchpoint.  We must find the next-highest child node by
                       going to the watchpoint's rightmost leaf: */
                    membus_watchpoint_addr_t *leaf = the_watchpoint->right;
                    while ( leaf->right ) leaf = leaf->right;
                    /* leaf now points to our replacement node: */
                    if ( (leaf->left = the_watchpoint->left) )
                        the_watchpoint->left->parent = leaf;
                    if ( (leaf->right = the_watchpoint->right) )
                        the_watchpoint->right->parent = leaf;
                    leaf->parent = the_watchpoint->parent;
                    the_watchpoint->parent->left = leaf;
                } else {
                    /* Promote left node to our position: */
                    the_watchpoint->left->parent = the_watchpoint->parent;
                    the_watchpoint->parent->left = the_watchpoint->left;
                }
            } else if ( the_watchpoint->right ) {
                /* Promote right node to our position: */
                the_watchpoint->right->parent = the_watchpoint->parent;
                the_watchpoint->parent->left = the_watchpoint->right;
            } else {
                /* Leaf node: */
                the_watchpoint->parent->left = NULL;
            }
        }
        else {
            /* We are the right-branch node off of the parent. If we have a common-address
               ancestry, then we can promote our first child to our position: */
            if ( the_watchpoint->child ) {
                if ( (the_watchpoint->child->left = the_watchpoint->left) )
                    the_watchpoint->left->parent = the_watchpoint->child;
                if ( (the_watchpoint->child->right = the_watchpoint->right) )
                    the_watchpoint->right->parent = the_watchpoint->child;
                the_watchpoint->child->parent = the_watchpoint->parent;
                the_watchpoint->parent->right = the_watchpoint->child;
            } else if ( the_watchpoint->right ) {
                if ( the_watchpoint->left ) {
                    /* We need to rebalance the binary tree that was rooted at
                       watchpoint.  We must find the next-lowest child node by
                       going to the watchpoint's leftmost leaf: */
                    membus_watchpoint_addr_t *leaf = the_watchpoint->left;
                    while ( leaf->left ) leaf = leaf->left;
                    /* leaf now points to our replacement node: */
                    if ( (leaf->left = the_watchpoint->left) )
                        the_watchpoint->left->parent = leaf;
                    if ( (leaf->right = the_watchpoint->right) )
                        the_watchpoint->right->parent = leaf;
                    leaf->parent = the_watchpoint->parent;
                    the_watchpoint->parent->right = leaf;
                } else {
                    /* Promote right node to our position: */
                    the_watchpoint->right->parent = the_watchpoint->parent;
                    the_watchpoint->parent->right = the_watchpoint->right;
                }
            } else if ( the_watchpoint->left ) {
                /* Promote left node to our position: */
                the_watchpoint->left->parent = the_watchpoint->parent;
                the_watchpoint->parent->right = the_watchpoint->left;
            } else {
                /* Leaf node: */
                the_watchpoint->parent->right = NULL;
            }
        }
    }
    __membus_watchpoint_addr_free(the_watchpoint);
}

//
////
//
















//





void
__membus_module_watchpoints_free(
    membus_module_ref   module
)
{
    membus_module_watchpoints_t *MODULE = (membus_module_watchpoints_t*)module;
    
    if ( MODULE->generics )
        __membus_watchpoint_generic_recursive_free(MODULE->generics);
    if ( MODULE->addresses )
        __membus_watchpoint_addr_recursive_free(MODULE->addresses);
    if ( MODULE->address_ranges )
        __membus_watchpoint_addr_range_recursive_free(MODULE->address_ranges);
}

//

membus_module_op_result_t
__membus_module_watchpoints_read_addr(
    membus_module_ref	module,
    uint16_t            addr,
    uint8_t             *value
)
{
    membus_module_watchpoints_t *MODULE = (membus_module_watchpoints_t*)module;
    
    if ( (MODULE->mode & membus_module_mode_mask) != membus_module_mode_wo ) {
        if ( MODULE->addresses ) {
            membus_watchpoint_addr_t *wp = __membus_watchpoint_addr_find_addr(
                                                MODULE->addresses,
                                                addr
                                            );
            while ( wp ) {
                if ( (wp->base.on_events & membus_watchpoint_event_read) == membus_watchpoint_event_read )
                    wp->base.callback_fn(module, membus_watchpoint_event_read, addr, *value, wp->base.context);
                wp = wp->child;
            }
        }
        if ( MODULE->generics ) {
            membus_watchpoint_generic_t *wp = MODULE->generics;
            while ( wp ) {
                if ( (wp->base.on_events & membus_watchpoint_event_read) == membus_watchpoint_event_read )
                    wp->base.callback_fn(module, membus_watchpoint_event_read, addr, *value, wp->base.context);
                wp = wp->link;
            }
        }
        if ( MODULE->address_ranges ) {
            membus_watchpoint_addr_range_t   *wp = __membus_watchpoint_addr_range_find_addr(MODULE->address_ranges, addr);
            while ( wp ) {
                if ( (wp->base.on_events & membus_watchpoint_event_read) == membus_watchpoint_event_read )
                    wp->base.callback_fn(module, membus_watchpoint_event_read, addr, *value, wp->base.context);
                wp = __membus_watchpoint_addr_range_find_addr(wp->link, addr);
            }
        }
        return membus_module_op_result_accepted;
    }
    return membus_module_op_result_not_accepted;
}

//

membus_module_op_result_t
__membus_module_watchpoints_write_addr(
    membus_module_ref	module,
    uint16_t            addr,
    uint8_t             value
)
{
    membus_module_watchpoints_t *MODULE = (membus_module_watchpoints_t*)module;
    
    if ( (MODULE->mode & membus_module_mode_mask) != membus_module_mode_ro ) {
        if ( MODULE->addresses ) {
            membus_watchpoint_addr_t *wp = __membus_watchpoint_addr_find_addr(
                                                MODULE->addresses,
                                                addr
                                            );
            while ( wp ) {
                if ( (wp->base.on_events & membus_watchpoint_event_write) == membus_watchpoint_event_write )
                    wp->base.callback_fn(module, membus_watchpoint_event_write, addr, value, wp->base.context);
                wp = wp->child;
            }
        }
        if ( MODULE->generics ) {
            membus_watchpoint_generic_t *wp = MODULE->generics;
            while ( wp ) {
                if ( (wp->base.on_events & membus_watchpoint_event_write) == membus_watchpoint_event_write )
                    wp->base.callback_fn(module, membus_watchpoint_event_write, addr, value, wp->base.context);
                wp = wp->link;
            }
        }
        if ( MODULE->address_ranges ) {
            membus_watchpoint_addr_range_t   *wp = __membus_watchpoint_addr_range_find_addr(MODULE->address_ranges, addr);
            while ( wp ) {
                if ( (wp->base.on_events & membus_watchpoint_event_write) == membus_watchpoint_event_write )
                    wp->base.callback_fn(module, membus_watchpoint_event_write, addr, value, wp->base.context);
                wp = __membus_watchpoint_addr_range_find_addr(wp->link, addr);
            }
        }
        return membus_module_op_result_accepted;
    }
    return membus_module_op_result_not_accepted;
}

//

static const membus_module_t membus_module_watchpoints_header = {
                .module_id = "WATCHPOINTS",
                .addr_range = { .addr_lo = 0x0000, .addr_len = 0xFFFF },
                .free_callback = __membus_module_watchpoints_free,
                .read_callback = __membus_module_watchpoints_read_addr,
                .write_callback = __membus_module_watchpoints_write_addr
            };

//

membus_module_ref
membus_module_watchpoints_alloc(
    membus_module_mode_t    mode
)
{
    membus_module_watchpoints_t *new_module = (membus_module_watchpoints_t*)malloc(sizeof(membus_module_watchpoints_t));
    
    if ( new_module ) {
        new_module->header = membus_module_watchpoints_header;
        new_module->mode = mode;
    }
    return (membus_module_ref)new_module;
}

//

membus_module_ref
membus_module_watchpoints_alloc_with_address_range(
    membus_module_mode_t    mode,
    memory_addr_range_t     addr_range
)
{
    membus_module_watchpoints_t *new_module = (membus_module_watchpoints_t*)malloc(sizeof(membus_module_watchpoints_t));
    
    if ( new_module ) {
        new_module->header = membus_module_watchpoints_header;
        new_module->header.addr_range = addr_range;
        new_module->mode = mode;
    }
    return (membus_module_ref)new_module;
}

//

membus_watchpoint_ref
membus_module_watchpoints_register_addr(
    membus_module_ref               the_module,
    membus_watchpoint_event_t       on_events,
    membus_watchpoint_callback_t    the_callback,
    const void                      *context,
    uint16_t                        addr
)
{
    membus_module_watchpoints_t     *MODULE = (membus_module_watchpoints_t*)the_module;
    membus_watchpoint_addr_t        *wp = NULL;
    
    if ( the_callback ) wp = __membus_watchpoint_addr_register(
                                    MODULE,
                                    &MODULE->addresses,
                                    on_events,
                                    the_callback,
                                    context,
                                    addr
                                );
    return (membus_watchpoint_ref)wp;
}

//

membus_watchpoint_ref
membus_module_watchpoints_register_generic(
    membus_module_ref               the_module,
    membus_watchpoint_event_t       on_events,
    membus_watchpoint_callback_t    the_callback,
    const void                      *context
)
{
    membus_module_watchpoints_t     *MODULE = (membus_module_watchpoints_t*)the_module;
    membus_watchpoint_generic_t     *wp = NULL;
    
    if ( the_callback ) wp = __membus_watchpoint_generic_register(
                                    MODULE,
                                    &MODULE->generics,
                                    on_events,
                                    the_callback,
                                    context
                                );
    return (membus_watchpoint_ref)wp;
}

//

membus_watchpoint_ref
membus_module_watchpoints_register_addr_range(
    membus_module_ref               the_module,
    membus_watchpoint_event_t       on_events,
    membus_watchpoint_callback_t    the_callback,
    const void                      *context,
    memory_addr_range_t             addr_range
)
{
    membus_module_watchpoints_t     *MODULE = (membus_module_watchpoints_t*)the_module;
    membus_watchpoint_addr_range_t  *wp = NULL;
    
    if ( the_callback ) wp = __membus_watchpoint_addr_range_register(
                                    MODULE,
                                    &MODULE->address_ranges,
                                    on_events,
                                    the_callback,
                                    context,
                                    &addr_range
                                );
    return (membus_watchpoint_ref)wp;
}

//

membus_watchpoint_subtype_t
membus_watchpoint_get_subtype(
    membus_watchpoint_ref   the_watchpoint
)
{
    return the_watchpoint->subtype;
}

//

membus_watchpoint_event_t
membus_watchpoint_get_events(
    membus_watchpoint_ref   the_watchpoint
)
{
    return the_watchpoint->on_events;
}

//

void
membus_watchpoint_set_events(
    membus_watchpoint_ref       the_watchpoint,
    membus_watchpoint_event_t   on_events
)
{
    if ( on_events & membus_watchpoint_event_merge_at_register )
        the_watchpoint->on_events |= (on_events & membus_watchpoint_event_all);
    else
        the_watchpoint->on_events = (on_events & membus_watchpoint_event_all);
}

//

void
membus_watchpoint_unregister(
    membus_watchpoint_ref   the_watchpoint
)
{
    switch ( the_watchpoint->subtype ) {
        case membus_watchpoint_subtype_address:
            __membus_watchpoint_addr_unregister((membus_watchpoint_addr_t*)the_watchpoint);
            break;
        case membus_watchpoint_subtype_generic:
            __membus_watchpoint_generic_unregister((membus_watchpoint_generic_t*)the_watchpoint);
            break;
        case membus_watchpoint_subtype_address_range:
            __membus_watchpoint_addr_range_unregister((membus_watchpoint_addr_range_t*)the_watchpoint);
            break;
    }
}