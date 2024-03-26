
#include "executor.h"

/*
 * LDA #$25
 * STA $0200
 * LDA #$30
 * STA $0201
 * LDA #$24
 * STA $0202
 * 
 * LDA #$c0  ;Load the hex value $c0 into the A register
 * TAX       ;Transfer the value in the A register to X
 * INX       ;Increment the value in the X register
 * ADC #$c4  ;Add the hex value $c4 to the A register
 * ADC #$00  ;Add the carry bit
 * BPL next
 * DEX
 *next:
 * SED
 * LDA #$17
 * ADC #$14
 * SEC
 * SBC #$18
 * CLD
 *
 * LDX #$08
 *decrement:
 * DEX
 * STX $0203
 * CPX #$03
 * BNE decrement
 * STX $0204
 *
 * LDA #$04
 * STA $02
 * LDA #$02
 * STA $03
 * LDX #$02
 * LDA ($00,X)
 * LDA #$FF
 * STA ($00,X)
 *
 * PHA
 * PHP
 * PLA
*/
static uint8_t hello_world[] = { 0xa9, 0x25, 0x8d, 0x00, 0x02, 0xa9, 0x30, 
                                 0x8d, 0x01, 0x02, 0xa9, 0x24, 0x8d, 0x02,
                                 0x02, 0xa9, 0xc0, 0xaa, 0xe8, 0x69, 0xc4,
                                 0x69, 0x00, 0x10, 0x01, 0xca, 0xf8, 0xa9,
                                 0x17, 0x69, 0x14, 0x38, 0xe9, 0x18, 0xd8,
                                 0xa2, 0x08, 0xca, 0x8e, 0x03, 0x02, 0xe0,
                                 0x03, 0xd0, 0xf8, 0x8e, 0x04, 0x02, 0xa9,
                                 0x04, 0x85, 0x02, 0xa9, 0x02, 0x85, 0x03,
                                 0xa2, 0x02, 0xa1, 0x00, 0xa9, 0xff, 0x81,
                                 0x00, 0x48, 0x08, 0x68 };
static size_t hello_world_len = sizeof(hello_world);

//

void
our_executor_stage_callback(
    executor_t                  *the_executor,
    isa_6502_instr_stage_t      the_stage,
    isa_6502_opcode_t           the_opcode,
    isa_6502_addressing_t       the_addressing_mode,
    isa_6502_opcode_dispatch_t  *the_dispatch, 
    uint64_t                    the_cycle_count
)
{
    #define REGISTERS       the_executor->registers
    #define MEMORY          the_executor->memory
    #define ISA             the_executor->isa
    
    executor_stage_callback_default(the_executor, the_stage, the_opcode, the_addressing_mode, the_dispatch, the_cycle_count);
    
    switch ( the_stage ) {
            
        case isa_6502_instr_stage_end:
            printf("[%04X] MEMORY:         ", the_stage); memory_fprintf(MEMORY, stdout, 0x0000, 0x000F);
            printf("[%04X] MEMORY:         ", the_stage); memory_fprintf(MEMORY, stdout, 0x0200, 0x020F);
            break;
            
    }
    
    #undef REGISTERS
    #undef MEMORY
    #undef ISA
}

//

int
main()
{
    executor_t      *our_executor = executor_alloc_with_default_components();
    
    if ( our_executor ) {
        uint64_t    total_cycles;
        
        /* Install program code: */
        memcpy(&our_executor->memory->RAM.PAGES[6][0], hello_world, hello_world_len);
        
        /* Execute! */
        total_cycles = executor_launch_at_address_range(
                                our_executor,
                                our_executor_stage_callback,
                                executor_stage_callback_default_stage_mask,
                                0x0600, 0x0600 + hello_world_len
                            );
        
    }
    return 0;
}
