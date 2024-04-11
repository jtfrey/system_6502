
#include "membus.h"

#include "membus_module_paged.h"
#include "membus_module_word.h"
#include "membus_module_std64k.h"
#include "membus_module_watchpoints.h"
#include "membus_module_mirror.h"

void
observe_address(
    membus_module_ref           the_module,
    membus_watchpoint_event_t   the_event,
    uint16_t                    addr,
    uint8_t                     value,
    const void                  *context
)
{
    printf("MEMBUS WATCHPOINT (ADDRESS) OBSERVER:  %s $%04hX = %02hhX\n",
        (the_event == membus_watchpoint_event_read) ? "READ" : "WRITE",
        addr,
        value
    );
}

int
main()
{
    membus_t    *the_bus = membus_alloc();
    uint16_t    addr;
    
    if ( the_bus ) {
        membus_module_ref       zero_and_stack, control_word, control_word_mirror, std64k, watchpoints;
        
        /* Zero and stack page: */
        membus_register_module(the_bus, 0,
                (zero_and_stack = membus_module_paged_alloc(membus_module_mode_rw, 0x00, 0x02)));
        
        /* Control word at 0x04F0: */
        membus_register_module(the_bus, 0,
                (control_word = membus_module_word_alloc(membus_module_mode_rw, 0x04F0)));
        
        /* Mirror the control word at 0x84F0: */
        membus_register_module(the_bus, 0,
                (control_word_mirror = membus_module_mirror_alloc(control_word, 0x84F0)));
        
        /* Base 64k under all else: */
        membus_register_module(the_bus, 200,
                (std64k = membus_module_std64k_alloc()));
        
        /* Watchpoints as a post-op: */
        membus_register_module(the_bus, MEMBUS_TIER_POST_OP,
                (watchpoints = membus_module_watchpoints_alloc(membus_module_mode_rw)));
        
        /* Register an address-based watchpoint: */
        membus_module_watchpoints_register_addr(
                watchpoints,
                membus_watchpoint_event_all,
                observe_address,
                NULL,
                0x006E);
        
        printf("Write 0x40 to 0x0054-0x0080...\n");
        addr = 0x0054;
        while ( addr <= 0x0080 ) membus_write_addr(the_bus, addr++, 0x40);
        
        printf("Write 0x2B to 0x0484...\n");
        membus_write_addr(the_bus, 0x0484, 0x2B);
        printf("%04hX : %02hhX\n", 0x0484, membus_read_addr(the_bus, 0x0484));
        
        printf("Write 0xAC to 0x04F1...\n");
        membus_write_addr(the_bus, 0x04F1, 0xAC);
        printf("%04hX : %02hhX\n", 0x04F1, membus_read_addr(the_bus, 0x04F1));
        
        printf("Read from 0x84F1 = %02hhX\n", membus_read_addr(the_bus, 0x84F1));
        printf("Write 0xCA to 0x84F1...\n");
        membus_write_addr(the_bus, 0x84F1, 0xCA);
        printf("%04hX : %02hhX\n", 0x04F1, membus_read_addr(the_bus, 0x04F1));
        
        membus_fprintf(the_bus, stdout, 0, memory_addr_range_with_lo_and_hi(0x0000, 0x01FF));
        printf("\n");
        membus_fprintf(the_bus, stdout, 0, memory_addr_range_with_lo_and_hi(0x0480, 0x04FF));
        printf("\n");
        membus_fprintf(the_bus, stdout, 0, memory_addr_range_with_lo_and_hi(0xFFFA, 0xFFFF));
        
        membus_free(the_bus);
    }
    return 0;
}
