
#include "membus.h"

#include "membus_module_paged.h"
#include "membus_module_word.h"
#include "membus_module_std64k.h"


int
main()
{
    membus_t    *the_bus = membus_alloc();
    
    if ( the_bus ) {
        /* Zero and stack page: */
        membus_register_module(the_bus, 0, membus_module_paged_alloc(membus_module_mode_rw, 0x00, 0x02));
        
        /* Control word at 0x04F0: */
        membus_register_module(the_bus, 0, membus_module_word_alloc(membus_module_mode_ro, 0x04F0));
        
        /* Base 64k under all else: */
        membus_register_module(the_bus, 200, membus_module_std64k_alloc());
        
        
        
        membus_write_addr(the_bus, 0x0054, 0xFE);
        printf("%04X : %02X\n", 0x0054, membus_read_addr(the_bus, 0x0054));
        
        membus_write_addr(the_bus, 0x0400, 0x2B);
        printf("%04X : %02X\n", 0x0400, membus_read_addr(the_bus, 0x0400));
        
        membus_write_addr(the_bus, 0x04F1, 0xAC);
        printf("%04X : %02X\n", 0x04F1, membus_read_addr(the_bus, 0x04F1));
        
        membus_free(the_bus);
    }
    return 0;
}
