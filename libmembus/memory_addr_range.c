
#include "memory_addr_range.h"

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
