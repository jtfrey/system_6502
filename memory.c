
#include "memory.h"

#include <ctype.h>

typedef struct memory_watchpoint {
    memory_t                        *source;
    uint16_t                        addr;
    memory_watchpoint_event_t       on_events;
    memory_watchpoint_callback_t    callback_fn;
    const void*                     context;
    struct memory_watchpoint        *left, *right;
    struct memory_watchpoint        *parent, *child;
} memory_watchpoint_t;

#define MEMORY_WATCHPOINT_VALID_EVENTS  (memory_watchpoint_event_read | memory_watchpoint_event_write | memory_watchpoint_event_merge_at_register)

//

memory_watchpoint_t*
__memory_watchpoint_alloc(void)
{
    memory_watchpoint_t *wp = (memory_watchpoint_t*)malloc(sizeof(memory_watchpoint_t));
    
    if ( wp ) {
        wp->source = NULL;
        wp->addr = 0x0000;
        wp->on_events = 0x00;
        wp->callback_fn = NULL;
        wp->context = NULL;
        wp->left = wp->right = wp->parent = wp->child = NULL;
    }
    return wp;
}

//

void
__memory_watchpoint_free(
    memory_watchpoint_t *wp
)
{
    free((void*)wp);
}

//

memory_watchpoint_t*
__memory_watchpoint_find_addr(
    memory_watchpoint_t *wp,
    uint16_t            addr
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

memory_watchpoint_t*
__memory_watchpoint_register(
    memory_t                        *source,
    memory_watchpoint_t             **root,
    uint16_t                        addr,
    memory_watchpoint_event_t       on_events,
    memory_watchpoint_callback_t    callback_fn,
    const void                      *context
)
{
    memory_watchpoint_t *wp = NULL, *new_wp = NULL;
    
    if ( ! root || ! *root ) {
        new_wp = __memory_watchpoint_alloc();
        if ( new_wp ) {
            new_wp->source = source;
            new_wp->addr = addr;
            new_wp->on_events = on_events;
            new_wp->callback_fn = callback_fn;
            new_wp->context = context;
        }
        if ( root ) *root = new_wp;
    } else {
        wp = *root;
        while ( wp ) {
            if ( wp->addr == addr ) {
                /* Does the callback already exist? */
                memory_watchpoint_t *last_wp = NULL;
                
                while ( wp ) {
                    if ( wp->callback_fn == callback_fn ) {
                        if ( on_events & memory_watchpoint_event_merge_at_register )
                            wp->on_events |= (on_events & memory_watchpoint_event_all);
                        else
                            wp->on_events = (on_events & memory_watchpoint_event_all);
                        new_wp = wp;
                        break;
                    }
                    last_wp = wp;
                    wp = wp->child;
                }
                if ( ! wp ) {
                    new_wp = __memory_watchpoint_alloc();
                    if ( new_wp ) {
                        new_wp->source = source;
                        new_wp->addr = addr;
                        new_wp->on_events = (on_events & memory_watchpoint_event_all);
                        new_wp->callback_fn = callback_fn;
                        new_wp->context = context;
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
                    new_wp = __memory_watchpoint_alloc();
                    if ( new_wp ) {
                        new_wp->source = source;
                        new_wp->addr = addr;
                        new_wp->on_events = (on_events & memory_watchpoint_event_all);
                        new_wp->callback_fn = callback_fn;
                        new_wp->context = context;
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
                    new_wp = __memory_watchpoint_alloc();
                    if ( new_wp ) {
                        new_wp->source = source;
                        new_wp->addr = addr;
                        new_wp->on_events = (on_events & memory_watchpoint_event_all);
                        new_wp->callback_fn = callback_fn;
                        new_wp->context = context;
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
        the_memory->watchpoints = NULL;
    }
    return the_memory;
}

//

void
memory_free(
    memory_t    *the_memory
)
{
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

memory_watchpoint_ref
memory_watchpoint_register(
    memory_t                        *the_memory,
    uint16_t                        addr,
    memory_watchpoint_event_t       on_events,
    memory_watchpoint_callback_t    the_callback,
    const void                      *context
)
{  
    memory_watchpoint_t *wp = NULL;
    
    if ( the_callback ) wp = (memory_watchpoint_ref)__memory_watchpoint_register(
                                    the_memory,
                                    (memory_watchpoint_t**)&the_memory->watchpoints,
                                    addr,
                                    on_events,
                                    the_callback,
                                    context
                                );
    return wp;
}

//

uint16_t
memory_watchpoint_get_address(
    memory_watchpoint_ref   the_watchpoint
)
{
    return the_watchpoint->addr;
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
            the_watchpoint->source->watchpoints = the_watchpoint->child;
        } else if ( the_watchpoint->left ) {
            if ( the_watchpoint->right ) {
                /* We need to rebalance the binary tree that was rooted at
                   watchpoint.  We must find the next-highest child node by
                   going to the watchpoint's rightmost leaf: */
                memory_watchpoint_t *leaf = the_watchpoint->right;
                while ( leaf->right ) leaf = leaf->right;
                /* leaf now points to our replacement node: */
                if ( (leaf->left = the_watchpoint->left) )
                    the_watchpoint->left->parent = leaf;
                if ( (leaf->right = the_watchpoint->right) )
                    the_watchpoint->right->parent = leaf;
                leaf->parent = NULL;
                the_watchpoint->source->watchpoints = leaf;
            } else {
                /* Promote left node to our position: */
                the_watchpoint->left->parent = NULL;
                the_watchpoint->source->watchpoints = the_watchpoint->left;
            }
        } else if ( the_watchpoint->right ) {
            /* Promote right node to our position: */
            the_watchpoint->right->parent = NULL;
            the_watchpoint->source->watchpoints = the_watchpoint->right;
        } else {
            /* Leaf node: */
            the_watchpoint->source->watchpoints = NULL;
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
                    memory_watchpoint_t *leaf = the_watchpoint->right;
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
                    memory_watchpoint_t *leaf = the_watchpoint->left;
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
    __memory_watchpoint_free(the_watchpoint);
}

//

uint8_t
memory_read(
    memory_t    *the_memory,
    uint16_t    addr
)
{
    if ( the_memory->watchpoints ) {
        memory_watchpoint_t *wp = __memory_watchpoint_find_addr(
                                        (memory_watchpoint_t*)the_memory->watchpoints,
                                        addr
                                    );
        while ( wp ) {
            if ( (wp->on_events & memory_watchpoint_event_read) == memory_watchpoint_event_read )
                wp->callback_fn(the_memory, addr, memory_watchpoint_event_read, wp->context);
            wp = wp->child;
        }
    }
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
    if ( the_memory->watchpoints ) {
        memory_watchpoint_t *wp = __memory_watchpoint_find_addr(
                                        (memory_watchpoint_t*)the_memory->watchpoints,
                                        addr
                                    );
        while ( wp ) {
            if ( (wp->on_events & memory_watchpoint_event_write) == memory_watchpoint_event_write )
                wp->callback_fn(the_memory, addr, memory_watchpoint_event_write, wp->context);
            wp = wp->child;
        }
    }
#ifdef ENABLE_MEMORY_CACHE
    memory_wcache_push(the_memory, the_memory->RAM.BYTES[addr] = value);
#else
    the_memory->RAM.BYTES[addr] = value;
#endif
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
    memory_t    *the_memory,
    FILE        *stream,
    uint16_t    addr_start,
    uint16_t    addr_end
)
{
    int         n_tot = 0;
    uint16_t    i = 0, i_max = addr_end - addr_start + 1;
    uint8_t     *p = &the_memory->RAM.BYTES[addr_start];
    char        *out_buffer_ptr, out_buffer[1024];
    size_t      out_buffer_len;

    while ( i < i_max ) {
        char    chars[17];
        int     l = 0, n;

        out_buffer_ptr = out_buffer;
        out_buffer_len = sizeof(out_buffer);

        n = snprintf(out_buffer_ptr, out_buffer_len, "%04hX : ", addr_start + i);
        out_buffer_ptr += n, out_buffer_len -= n;

        while ( (l < 16) && (i < i_max) ) {
            if ( l == 8 ) {
                n = snprintf(out_buffer_ptr, out_buffer_len, "   ");
                out_buffer_ptr += n, out_buffer_len -= n;
            }
            n = snprintf(out_buffer_ptr, out_buffer_len, "%02X ", *p);
            out_buffer_ptr += n, out_buffer_len -= n;

            chars[l] = (isprint(*p) ? *p : '.');
            l++, p++, i++;
        }
        chars[l] = '\0';
        while ( l < 16 ) {
            n = snprintf(out_buffer_ptr, out_buffer_len, (l == 8) ? "      " : "   ");
            out_buffer_ptr += n, out_buffer_len -= n;
            l++;
        }
        n = snprintf(out_buffer_ptr, out_buffer_len, "   %s", chars);
        n_tot += fprintf(stream, "%s\n", out_buffer);
    }
    return n_tot;
}

//

#ifdef ENABLE_MEMORY_TEST

void
observe(
    memory_t                    *the_memory,
    uint16_t                    addr,
    memory_watchpoint_event_t   event,
    const void                  *context
)
{
    printf("MEMORY WATCHPOINT OBSERVER:  %s $%04hX\n",
        (event == memory_watchpoint_event_read) ? "READ" : "WRITE",
        addr
    );
}

int
main()
{
    memory_t    the_memory;
    
    memory_init(&the_memory);
    memory_watchpoint_register(&the_memory, 0x1500, memory_watchpoint_event_all, observe, NULL);
    
    printf("Read $1500 = $%02hhX\n", memory_read(&the_memory, 0x1500));
    memory_write(&the_memory, 0x1500, 0xFF);
    printf("Read $1500 = $%02hhX\n", memory_read(&the_memory, 0x1500));
    
    return 0;
}

#endif /* HAVE_MEMORY_TEST */
