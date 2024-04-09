
#include "memory.h"

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

#ifdef ENABLE_MEMORY_WATCHPOINTS

typedef struct memory_watchpoint {
    memory_watchpoint_subtype_t     subtype;
    memory_t                        *source;
    memory_watchpoint_event_t       on_events;
    memory_watchpoint_callback_t    callback_fn;
    const void*                     context;
} memory_watchpoint_t;

#define MEMORY_WATCHPOINT_VALID_EVENTS  (memory_watchpoint_event_read | memory_watchpoint_event_write | memory_watchpoint_event_merge_at_register)

//

typedef struct memory_watchpoint_generic {
    memory_watchpoint_t                 base;
    struct memory_watchpoint_generic    *link;
} memory_watchpoint_generic_t;

//

memory_watchpoint_generic_t*
__memory_watchpoint_generic_alloc(void)
{
    memory_watchpoint_generic_t *wp = (memory_watchpoint_generic_t*)malloc(sizeof(memory_watchpoint_generic_t));
    
    if ( wp ) {
        wp->base.subtype = memory_watchpoint_subtype_generic;
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
__memory_watchpoint_generic_free(
    memory_watchpoint_generic_t *wp
)
{
    free((void*)wp);
}

//

void
__memory_watchpoint_generic_recursive_free(
    memory_watchpoint_generic_t *wp
)
{
    memory_watchpoint_generic_t *next;
    
    while ( wp ) {
        next = wp->link;
        __memory_watchpoint_generic_free(wp);
        wp = next;
    }
}

//

memory_watchpoint_generic_t*
__memory_watchpoint_generic_register(
    memory_t                        *source,
    memory_watchpoint_generic_t     **root,
    memory_watchpoint_event_t       on_events,
    memory_watchpoint_callback_t    callback_fn,
    const void                      *context
)
{
    memory_watchpoint_generic_t     *new_wp = __memory_watchpoint_generic_alloc();
    
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
__memory_watchpoint_generic_unregister(
    memory_watchpoint_generic_t    *the_watchpoint
)
{
    memory_watchpoint_generic_t     *root = (memory_watchpoint_generic_t*)the_watchpoint->base.source->watchpoints[memory_watchpoint_subtype_generic];
    memory_watchpoint_generic_t     *node = root, *prev;
    
    while ( node ) {
        if ( node == the_watchpoint ) {
            /* Root node? */
            if ( node == root ) {
                the_watchpoint->base.source->watchpoints[memory_watchpoint_subtype_generic] = node->link;
            } else {
                prev->link = node->link;
            }
            __memory_watchpoint_generic_free(node);
            break;
        }
        prev = node;
        node = node->link;
    }
}

//
////
//

typedef struct memory_watchpoint_range {
    memory_watchpoint_t                 base;
    memory_addr_range_t                 addr_range;
    struct memory_watchpoint_range      *link;
} memory_watchpoint_range_t;

//

memory_watchpoint_range_t*
__memory_watchpoint_range_alloc(void)
{
    memory_watchpoint_range_t *wp = (memory_watchpoint_range_t*)malloc(sizeof(memory_watchpoint_range_t));
    
    if ( wp ) {
        wp->base.subtype = memory_watchpoint_subtype_range;
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
__memory_watchpoint_range_free(
    memory_watchpoint_range_t *wp
)
{
    free((void*)wp);
}

//

void
__memory_watchpoint_range_recursive_free(
    memory_watchpoint_range_t *wp
)
{
    memory_watchpoint_range_t *next;
    
    while ( wp ) {
        next = wp->link;
        __memory_watchpoint_range_free(wp);
        wp = next;
    }
}

//

memory_watchpoint_range_t*
__memory_watchpoint_range_find_addr(
    memory_watchpoint_range_t   *wp,
    uint16_t                    addr
)
{
    while ( wp ) {
        if ( memory_addr_range_does_include(&wp->addr_range, addr) ) break;
        wp = wp->link;
    }
    return wp;
}

//

memory_watchpoint_range_t*
__memory_watchpoint_range_register(
    memory_t                        *source,
    memory_watchpoint_range_t       **root,
    memory_watchpoint_event_t       on_events,
    memory_watchpoint_callback_t    callback_fn,
    const void                      *context,
    memory_addr_range_t             *addr_range
)
{
    memory_watchpoint_range_t       *new_wp = NULL;
    
    if ( addr_range->addr_len ) {
        new_wp = __memory_watchpoint_range_alloc();
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
__memory_watchpoint_range_unregister(
    memory_watchpoint_range_t       *the_watchpoint
)
{
    memory_watchpoint_range_t       *root = (memory_watchpoint_range_t*)the_watchpoint->base.source->watchpoints[memory_watchpoint_subtype_range];
    memory_watchpoint_range_t       *node = root, *prev;
    
    while ( node ) {
        if ( node == the_watchpoint ) {
            /* Root node? */
            if ( node == root ) {
                the_watchpoint->base.source->watchpoints[memory_watchpoint_subtype_range] = node->link;
            } else {
                prev->link = node->link;
            }
            __memory_watchpoint_range_free(node);
            break;
        }
        prev = node;
        node = node->link;
    }
}

//
////
//

typedef struct memory_watchpoint_addr {
    memory_watchpoint_t             base;
    uint16_t                        addr;
    struct memory_watchpoint_addr   *left, *right;
    struct memory_watchpoint_addr   *parent, *child;
} memory_watchpoint_addr_t;

//

memory_watchpoint_addr_t*
__memory_watchpoint_addr_alloc(void)
{
    memory_watchpoint_addr_t *wp = (memory_watchpoint_addr_t*)malloc(sizeof(memory_watchpoint_addr_t));
    
    if ( wp ) {
        wp->base.subtype = memory_watchpoint_subtype_address;
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
__memory_watchpoint_addr_free(
    memory_watchpoint_addr_t *wp
)
{
    free((void*)wp);
}

//

void
__memory_watchpoint_addr_recursive_free(
    memory_watchpoint_addr_t *wp
)
{
    memory_watchpoint_addr_t *node = wp->child, *next;
    
    while ( node ) {
        next = node->child;
        __memory_watchpoint_addr_free(node);
        node = next;
    }
    
    if ( wp->left ) __memory_watchpoint_addr_recursive_free(wp->left);
    if ( wp->right ) __memory_watchpoint_addr_recursive_free(wp->right);
    
    __memory_watchpoint_addr_free(wp);
}

//

memory_watchpoint_addr_t*
__memory_watchpoint_addr_find_addr(
    memory_watchpoint_addr_t    *wp,
    uint16_t                    addr
)
{
    while ( wp ) {
        if ( wp->addr == addr ) break;
        if ( wp->addr > addr ) wp = wp->left;
        wp = wp->right;
    }
    return wp;
}

//

memory_watchpoint_addr_t*
__memory_watchpoint_addr_register(
    memory_t                        *source,
    memory_watchpoint_addr_t        **root,
    memory_watchpoint_event_t       on_events,
    memory_watchpoint_callback_t    callback_fn,
    const void                      *context,
    uint16_t                        addr
)
{
    memory_watchpoint_addr_t        *wp = NULL, *new_wp = NULL;
    
    if ( ! root || ! *root ) {
        new_wp = __memory_watchpoint_addr_alloc();
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
                memory_watchpoint_addr_t *last_wp = NULL;
                
                while ( wp ) {
                    if ( wp->base.callback_fn == callback_fn ) {
                        if ( on_events & memory_watchpoint_event_merge_at_register )
                            wp->base.on_events |= (on_events & memory_watchpoint_event_all);
                        else
                            wp->base.on_events = (on_events & memory_watchpoint_event_all);
                        new_wp = wp;
                        break;
                    }
                    last_wp = wp;
                    wp = wp->child;
                }
                if ( ! wp ) {
                    new_wp = __memory_watchpoint_addr_alloc();
                    if ( new_wp ) {
                        new_wp->base.source = source;
                        new_wp->base.on_events = (on_events & memory_watchpoint_event_all);
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
                    new_wp = __memory_watchpoint_addr_alloc();
                    if ( new_wp ) {
                        new_wp->base.source = source;
                        new_wp->base.on_events = (on_events & memory_watchpoint_event_all);
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
                    new_wp = __memory_watchpoint_addr_alloc();
                    if ( new_wp ) {
                        new_wp->base.source = source;
                        new_wp->base.on_events = (on_events & memory_watchpoint_event_all);
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
__memory_watchpoint_addr_unregister(
    memory_watchpoint_addr_t    *the_watchpoint
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
            the_watchpoint->base.source->watchpoints[memory_watchpoint_subtype_address] = the_watchpoint->child;
        } else if ( the_watchpoint->left ) {
            if ( the_watchpoint->right ) {
                /* We need to rebalance the binary tree that was rooted at
                   watchpoint.  We must find the next-highest child node by
                   going to the watchpoint's rightmost leaf: */
                memory_watchpoint_addr_t *leaf = the_watchpoint->right;
                while ( leaf->right ) leaf = leaf->right;
                /* leaf now points to our replacement node: */
                if ( (leaf->left = the_watchpoint->left) )
                    the_watchpoint->left->parent = leaf;
                if ( (leaf->right = the_watchpoint->right) )
                    the_watchpoint->right->parent = leaf;
                leaf->parent = NULL;
                the_watchpoint->base.source->watchpoints[memory_watchpoint_subtype_address] = leaf;
            } else {
                /* Promote left node to our position: */
                the_watchpoint->left->parent = NULL;
                the_watchpoint->base.source->watchpoints[memory_watchpoint_subtype_address] = the_watchpoint->left;
            }
        } else if ( the_watchpoint->right ) {
            /* Promote right node to our position: */
            the_watchpoint->right->parent = NULL;
            the_watchpoint->base.source->watchpoints[memory_watchpoint_subtype_address] = the_watchpoint->right;
        } else {
            /* Leaf node: */
            the_watchpoint->base.source->watchpoints[memory_watchpoint_subtype_address] = NULL;
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
                    memory_watchpoint_addr_t *leaf = the_watchpoint->right;
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
                    memory_watchpoint_addr_t *leaf = the_watchpoint->left;
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
    __memory_watchpoint_addr_free(the_watchpoint);
}

#endif

//

memory_t*
memory_alloc(void)
{
    return (memory_t*)malloc(sizeof(memory_t));
}

//

memory_t*
memory_init(
    memory_t    *the_memory
)
{
    if ( ! the_memory ) the_memory = memory_alloc();
    if ( the_memory ) {
#ifdef ENABLE_MEMORY_WATCHPOINTS
        the_memory->watchpoints[0] = the_memory->watchpoints[1] = the_memory->watchpoints[2] = NULL;
#endif
    }
    return the_memory;
}

//

void
memory_free(
    memory_t    *the_memory
)
{
#ifdef ENABLE_MEMORY_WATCHPOINTS
    if ( the_memory->watchpoints[memory_watchpoint_subtype_address] ) __memory_watchpoint_addr_recursive_free((memory_watchpoint_addr_t*)the_memory->watchpoints[memory_watchpoint_subtype_address]);
#endif
    free((void*)the_memory);
}

//

void
memory_reset(
    memory_t    *the_memory,
    uint8_t     fill_byte
)
{
    memset(&the_memory->RAM.BYTES[0], (int)fill_byte, sizeof(the_memory->RAM.BYTES));
}

//

#ifdef ENABLE_MEMORY_WATCHPOINTS

memory_watchpoint_ref
memory_watchpoint_register_addr(
    memory_t                        *the_memory,
    memory_watchpoint_event_t       on_events,
    memory_watchpoint_callback_t    the_callback,
    const void                      *context,
    uint16_t                        addr
)
{
    memory_watchpoint_addr_t        *wp = NULL;
    
    if ( the_callback ) wp = __memory_watchpoint_addr_register(
                                    the_memory,
                                    (memory_watchpoint_addr_t**)&the_memory->watchpoints[memory_watchpoint_subtype_address],
                                    on_events,
                                    the_callback,
                                    context,
                                    addr
                                );
    return (memory_watchpoint_ref)wp;
}

//

memory_watchpoint_ref
memory_watchpoint_register_generic(
    memory_t                        *the_memory,
    memory_watchpoint_event_t       on_events,
    memory_watchpoint_callback_t    the_callback,
    const void                      *context
)
{
    memory_watchpoint_generic_t     *wp = NULL;
    
    if ( the_callback ) wp = __memory_watchpoint_generic_register(
                                    the_memory,
                                    (memory_watchpoint_generic_t**)&the_memory->watchpoints[memory_watchpoint_subtype_generic],
                                    on_events,
                                    the_callback,
                                    context
                                );
    return (memory_watchpoint_ref)wp;
}

//

memory_watchpoint_ref
memory_watchpoint_register_range(
    memory_t                        *the_memory,
    memory_watchpoint_event_t       on_events,
    memory_watchpoint_callback_t    the_callback,
    const void                      *context,
    memory_addr_range_t             addr_range
)
{
    memory_watchpoint_range_t       *wp = NULL;
    
    if ( the_callback ) wp = __memory_watchpoint_range_register(
                                    the_memory,
                                    (memory_watchpoint_range_t**)&the_memory->watchpoints[memory_watchpoint_subtype_range],
                                    on_events,
                                    the_callback,
                                    context,
                                    &addr_range
                                );
    return (memory_watchpoint_ref)wp;
}

//

memory_watchpoint_subtype_t
memory_watchpoint_get_subtype(
    memory_watchpoint_ref   the_watchpoint
)
{
    return the_watchpoint->subtype;
}

//

memory_watchpoint_event_t
memory_watchpoint_get_events(
    memory_watchpoint_ref   the_watchpoint
)
{
    return the_watchpoint->on_events;
}

//

void
memory_watchpoint_set_events(
    memory_watchpoint_ref       the_watchpoint,
    memory_watchpoint_event_t   on_events
)
{
    if ( on_events & memory_watchpoint_event_merge_at_register )
        the_watchpoint->on_events |= (on_events & memory_watchpoint_event_all);
    else
        the_watchpoint->on_events = (on_events & memory_watchpoint_event_all);
}

//

void
memory_watchpoint_unregister(
    memory_watchpoint_ref   the_watchpoint
)
{
    switch ( the_watchpoint->subtype ) {
        case memory_watchpoint_subtype_address:
            __memory_watchpoint_addr_unregister((memory_watchpoint_addr_t*)the_watchpoint);
            break;
        case memory_watchpoint_subtype_generic:
            __memory_watchpoint_generic_unregister((memory_watchpoint_generic_t*)the_watchpoint);
            break;
        case memory_watchpoint_subtype_range:
            __memory_watchpoint_range_unregister((memory_watchpoint_range_t*)the_watchpoint);
            break;
    }
}

#endif

//

uint8_t
memory_read(
    memory_t    *the_memory,
    uint16_t    addr
)
{
#ifdef ENABLE_MEMORY_WATCHPOINTS
    if ( the_memory->watchpoints[memory_watchpoint_subtype_address] ) {
        memory_watchpoint_addr_t *wp = __memory_watchpoint_addr_find_addr(
                                            (memory_watchpoint_addr_t*)the_memory->watchpoints[memory_watchpoint_subtype_address],
                                            addr
                                        );
        while ( wp ) {
            if ( (wp->base.on_events & memory_watchpoint_event_read) == memory_watchpoint_event_read )
                wp->base.callback_fn(the_memory, addr, memory_watchpoint_event_read, wp->base.context);
            wp = wp->child;
        }
    }
    if ( the_memory->watchpoints[memory_watchpoint_subtype_generic] ) {
        memory_watchpoint_generic_t *wp = (memory_watchpoint_generic_t*)the_memory->watchpoints[memory_watchpoint_subtype_generic];
        while ( wp ) {
            if ( (wp->base.on_events & memory_watchpoint_event_read) == memory_watchpoint_event_read )
                wp->base.callback_fn(the_memory, addr, memory_watchpoint_event_read, wp->base.context);
            wp = wp->link;
        }
    }
    if ( the_memory->watchpoints[memory_watchpoint_subtype_range] ) {
        memory_watchpoint_range_t   *wp = __memory_watchpoint_range_find_addr((memory_watchpoint_range_t*)the_memory->watchpoints[memory_watchpoint_subtype_range], addr);
        while ( wp ) {
            if ( (wp->base.on_events & memory_watchpoint_event_read) == memory_watchpoint_event_read )
                wp->base.callback_fn(the_memory, addr, memory_watchpoint_event_read, wp->base.context);
            wp = __memory_watchpoint_range_find_addr(wp->link, addr);
        }
    }
#endif
#ifdef ENABLE_MEMORY_CACHE
    return memory_rcache_push(the_memory, the_memory->RAM.BYTES[addr]);
#else
    return the_memory->RAM.BYTES[addr];
#endif
}

//

void
memory_write(
    memory_t    *the_memory,
    uint16_t    addr,
    uint8_t     value
)
{
#ifdef ENABLE_MEMORY_WATCHPOINTS
    if ( the_memory->watchpoints[memory_watchpoint_subtype_address] ) {
        memory_watchpoint_addr_t *wp = __memory_watchpoint_addr_find_addr(
                                            (memory_watchpoint_addr_t*)the_memory->watchpoints[memory_watchpoint_subtype_address],
                                            addr
                                        );
        while ( wp ) {
            if ( (wp->base.on_events & memory_watchpoint_event_write) == memory_watchpoint_event_write )
                wp->base.callback_fn(the_memory, addr, memory_watchpoint_event_write, wp->base.context);
            wp = wp->child;
        }
    }
    if ( the_memory->watchpoints[memory_watchpoint_subtype_generic] ) {
        memory_watchpoint_generic_t *wp = (memory_watchpoint_generic_t*)the_memory->watchpoints[memory_watchpoint_subtype_generic];
        while ( wp ) {
            if ( (wp->base.on_events & memory_watchpoint_event_write) == memory_watchpoint_event_write )
                wp->base.callback_fn(the_memory, addr, memory_watchpoint_event_write, wp->base.context);
            wp = wp->link;
        }
    }
    if ( the_memory->watchpoints[memory_watchpoint_subtype_range] ) {
        memory_watchpoint_range_t   *wp = __memory_watchpoint_range_find_addr((memory_watchpoint_range_t*)the_memory->watchpoints[memory_watchpoint_subtype_range], addr);
        while ( wp ) {
            if ( (wp->base.on_events & memory_watchpoint_event_write) == memory_watchpoint_event_write )
                wp->base.callback_fn(the_memory, addr, memory_watchpoint_event_write, wp->base.context);
            wp = __memory_watchpoint_range_find_addr(wp->link, addr);
        }
    }
#endif
#ifdef ENABLE_MEMORY_CACHE
    memory_wcache_push(the_memory, the_memory->RAM.BYTES[addr] = value);
#else
    the_memory->RAM.BYTES[addr] = value;
#endif
}

//

void
memory_write_range(
    memory_t            *the_memory,
    memory_addr_range_t *r,
    uint8_t             value
)
{
    uint16_t            addr = r->addr_lo, addr_len = r->addr_len;
    
    while ( addr_len-- ) {
#ifdef ENABLE_MEMORY_WATCHPOINTS
        if ( the_memory->watchpoints[memory_watchpoint_subtype_address] ) {
            memory_watchpoint_addr_t *wp = __memory_watchpoint_addr_find_addr(
                                                (memory_watchpoint_addr_t*)the_memory->watchpoints[memory_watchpoint_subtype_address],
                                                addr
                                            );
            while ( wp ) {
                if ( (wp->base.on_events & memory_watchpoint_event_write) == memory_watchpoint_event_write )
                    wp->base.callback_fn(the_memory, addr, memory_watchpoint_event_write, wp->base.context);
                wp = wp->child;
            }
        }
        if ( the_memory->watchpoints[memory_watchpoint_subtype_generic] ) {
            memory_watchpoint_generic_t *wp = (memory_watchpoint_generic_t*)the_memory->watchpoints[memory_watchpoint_subtype_generic];
            while ( wp ) {
                if ( (wp->base.on_events & memory_watchpoint_event_write) == memory_watchpoint_event_write )
                    wp->base.callback_fn(the_memory, addr, memory_watchpoint_event_write, wp->base.context);
                wp = wp->link;
            }
        }
        if ( the_memory->watchpoints[memory_watchpoint_subtype_range] ) {
            memory_watchpoint_range_t   *wp = __memory_watchpoint_range_find_addr((memory_watchpoint_range_t*)the_memory->watchpoints[memory_watchpoint_subtype_range], addr);
            while ( wp ) {
                if ( (wp->base.on_events & memory_watchpoint_event_write) == memory_watchpoint_event_write )
                    wp->base.callback_fn(the_memory, addr, memory_watchpoint_event_write, wp->base.context);
                wp = __memory_watchpoint_range_find_addr(wp->link, addr);
            }
        }
#endif
#ifdef ENABLE_MEMORY_CACHE
        memory_wcache_push(the_memory, the_memory->RAM.BYTES[addr] = value);
#else
        the_memory->RAM.BYTES[addr] = value;
#endif
        addr++;
    }
}

//

ssize_t
memory_load_from_fd(
    memory_t    *the_memory,
    uint16_t    baseaddr,
    uint16_t    bytesize,
    int         fd
)
{
    if ( baseaddr + bytesize > 0xFFFF ) bytesize = 0xFFFF - baseaddr + 1;
    return read(fd, &the_memory->RAM.BYTES[baseaddr], bytesize);
}

//

ssize_t
memory_load_from_stream(
    memory_t    *the_memory,
    uint16_t    baseaddr,
    uint16_t    bytesize,
    FILE        *stream
)
{
    if ( baseaddr + bytesize > 0xFFFF ) bytesize = 0xFFFF - baseaddr + 1;
    return fread(&the_memory->RAM.BYTES[baseaddr], 1, bytesize, stream);
}

//

ssize_t
memory_save_to_fd(
    memory_t    *the_memory,
    uint16_t    baseaddr,
    uint16_t    bytesize,
    int         fd
)
{
    if ( baseaddr + bytesize > 0xFFFF ) bytesize = 0xFFFF - baseaddr + 1;
    return write(fd, &the_memory->RAM.BYTES[baseaddr], bytesize);
}

//

ssize_t
memory_save_to_stream(
    memory_t    *the_memory,
    uint16_t    baseaddr,
    uint16_t    bytesize,
    FILE        *stream
)
{
    if ( baseaddr + bytesize > 0xFFFF ) bytesize = 0xFFFF - baseaddr + 1;
    return fwrite(&the_memory->RAM.BYTES[baseaddr], 1, bytesize, stream);
}

//

int
memory_fprintf(
    memory_t            *the_memory,
    FILE                *stream,
    memory_dump_opts_t  opts, 
    uint16_t            addr_start,
    uint16_t            addr_end
)
{
    int         n_tot = 0;
    uint16_t    i = 0, i_max = addr_end - addr_start + 1;
    uint8_t     *p = &the_memory->RAM.BYTES[addr_start];
    char        *out_buffer_ptr, out_buffer[1024];
    size_t      out_buffer_len;
    int         is_compact = ((opts & memory_dump_opts_compact) == memory_dump_opts_compact);
    
    if ( (opts & memory_dump_opts_8byte_width) == memory_dump_opts_8byte_width ) {
        while ( i < i_max ) {
            char    chars[9];
            int     l = 0, n;

            out_buffer_ptr = out_buffer;
            out_buffer_len = sizeof(out_buffer);

            n = snprintf(out_buffer_ptr, out_buffer_len, is_compact ? "%04hX:" : "%04hX : ", addr_start + i);
            out_buffer_ptr += n, out_buffer_len -= n;

            while ( (l < 8) && (i < i_max) ) {
                n = snprintf(out_buffer_ptr, out_buffer_len, "%02X ", *p);
                out_buffer_ptr += n, out_buffer_len -= n;

                chars[l] = (isprint(*p) ? *p : '.');
                l++, p++, i++;
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
        while ( i < i_max ) {
            char    chars[17];
            int     l = 0, n;

            out_buffer_ptr = out_buffer;
            out_buffer_len = sizeof(out_buffer);

            n = snprintf(out_buffer_ptr, out_buffer_len, is_compact ? "%04hX:" : "%04hX : ", addr_start + i);
            out_buffer_ptr += n, out_buffer_len -= n;

            while ( (l < 16) && (i < i_max) ) {
                if ( l == 8 ) {
                    n = snprintf(out_buffer_ptr, out_buffer_len, is_compact ? " " : "   ");
                    out_buffer_ptr += n, out_buffer_len -= n;
                }
                n = snprintf(out_buffer_ptr, out_buffer_len, "%02X ", *p);
                out_buffer_ptr += n, out_buffer_len -= n;

                chars[l] = (isprint(*p) ? *p : '.');
                l++, p++, i++;
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

//

#ifdef ENABLE_MEMORY_TEST


#ifdef ENABLE_MEMORY_WATCHPOINTS
void
observe_address(
    memory_t                    *the_memory,
    uint16_t                    addr,
    memory_watchpoint_event_t   event,
    const void                  *context
)
{
    printf("MEMORY WATCHPOINT (ADDRESS) OBSERVER:  %s $%04hX\n",
        (event == memory_watchpoint_event_read) ? "READ" : "WRITE",
        addr
    );
}

void
observe_generic(
    memory_t                    *the_memory,
    uint16_t                    addr,
    memory_watchpoint_event_t   event,
    const void                  *context
)
{
    printf("MEMORY WATCHPOINT (GENERIC) OBSERVER:  %s $%04hX\n",
        (event == memory_watchpoint_event_read) ? "READ" : "WRITE",
        addr
    );
}

void
observe_range(
    memory_t                    *the_memory,
    uint16_t                    addr,
    memory_watchpoint_event_t   event,
    const void                  *context
)
{
    printf("MEMORY WATCHPOINT (RANGE) OBSERVER:  %s $%04hX\n",
        (event == memory_watchpoint_event_read) ? "READ" : "WRITE",
        addr
    );
}

#endif

int
main()
{
    memory_t    the_memory;
    
    memory_init(&the_memory);

#ifdef ENABLE_MEMORY_WATCHPOINTS
    memory_watchpoint_register_addr(&the_memory, memory_watchpoint_event_all, observe_address, NULL, 0x1500);
    memory_watchpoint_register_generic(&the_memory, memory_watchpoint_event_all, observe_generic, NULL);
    memory_watchpoint_register_range(&the_memory, memory_watchpoint_event_read, observe_range, NULL, memory_addr_range_with_lo_and_hi(0x1000, 0x2000));
#endif

    printf("Read $1500 = $%02hhX\n", memory_read(&the_memory, 0x1500));
    memory_write(&the_memory, 0x1500, 0xFF);
    printf("Read $1500 = $%02hhX\n", memory_read(&the_memory, 0x1500));
    
    /* [0028:00 00 00 00 00 00 00 00 ........] */
    memory_fprintf(&the_memory, stdout, memory_dump_opts_8byte_width | memory_dump_opts_compact, 0x0000, 0x0048);
    
    return 0;
}

#endif /* HAVE_MEMORY_TEST */
