
#include "executor.h"

typedef struct {
    executor_t          pointers;
    registers_t         registers;
    memory_t            memory;
    isa_6502_table_t    isa;
} executor_bundle_t;

enum {
    executor_flags_is_bundle = 1 << 0
};

executor_t*
executor_alloc_with_default_components(void)
{
    executor_bundle_t   *new_executor = (executor_bundle_t*)malloc(sizeof(executor_bundle_t));
    
    if ( new_executor ) {
        new_executor->pointers.flags = executor_flags_is_bundle;
        new_executor->pointers.registers = &new_executor->registers;
        new_executor->pointers.memory = &new_executor->memory;
        new_executor->pointers.isa = &new_executor->isa;
        
        registers_init(&new_executor->registers);
        memory_init(&new_executor->memory);
        isa_6502_table_init(&new_executor->isa, isa_6502_dialect_base);
    }
    return (executor_t*)new_executor;
}

//

executor_t*
executor_alloc_with_components(
    registers_t         *the_registers,
    memory_t            *the_memory,
    isa_6502_table_t    *the_isa
)
{
    executor_t  *new_executor = (executor_t*)malloc(sizeof(executor_t));
    
    if ( new_executor ) {
        new_executor->flags = 0;
        new_executor->registers = the_registers;
        new_executor->memory = the_memory;
        new_executor->isa = the_isa;
    }
    return (executor_t*)new_executor;
}

//

void
executor_free(
    executor_t      *the_executor
)
{
    if ( (the_executor->flags & executor_flags_is_bundle) == executor_flags_is_bundle ) {
        registers_free(the_executor->registers);
        memory_free(the_executor->memory);
        isa_6502_table_free(the_executor->isa);
    }
    free((void*)the_executor);
}

//

void
executor_reset(
    executor_t      *the_executor
)
{
    /* Reset memory */
    memory_reset(the_executor->memory, 0x00);
    
    /* Reset registers */
    registers_reset(the_executor->registers);
}

//

uint64_t
executor_launch_at_address(
    executor_t                  *the_executor,
    executor_stage_callback_t   callback_fn,
    isa_6502_instr_stage_t      callback_stage_mask,
    uint16_t                    start_addr
)
{
    return executor_launch_at_address_range(the_executor, callback_fn, callback_stage_mask, start_addr, 0xFFFF);
}

//

void
executor_stage_callback_default(
    executor_t                  *the_executor,
    isa_6502_instr_stage_t      the_stage,
    isa_6502_opcode_t           the_opcode,
    isa_6502_addressing_t       the_addressing_mode,
    isa_6502_opcode_dispatch_t  *the_dispatch, 
    uint64_t                    the_cycle_count
)
{
    #define REGISTERS           the_executor->registers
    #define MEMORY              the_executor->memory
    #define ISA                 the_executor->isa
    
    switch ( the_stage ) {
        
        case isa_6502_instr_stage_post_load_PC:
            printf("[%04X] REGISTERS:      ", the_stage); registers_fprintf(REGISTERS, stdout);
            printf("\n");
            break;
    
        case isa_6502_instr_stage_post_fetch_opcode:
            printf("\n[%04X] FETCHED:        $%02hhX => a=%d b=%d c=%d\n", the_stage, the_opcode.BYTE,
                    the_opcode.FIELDS.A, the_opcode.FIELDS.B, the_opcode.FIELDS.C);
            printf("[%04X] REGISTERS:      ", the_stage); registers_fprintf(REGISTERS, stdout);
            break;
        
        case isa_6502_instr_stage_post_decode_opcode:
            printf("[%04X] DECODED:        \"%s\" (ADDRESSING MODE %d = \"%s\")\n", the_stage,
                    the_dispatch->description, the_dispatch->addressing_mode,
                    isa_6502_addressing_modes_descriptions[the_dispatch->addressing_mode]);
            break;
        
        case isa_6502_instr_stage_next_cycle:
            printf("[%04X] REGISTERS:      ", the_stage); registers_fprintf(REGISTERS, stdout);
            break;
            
        case isa_6502_instr_stage_end:
            printf("[%04X] ELAPSED CYCLES: %llu\n", the_stage, the_cycle_count);
            printf("[%04X] REGISTERS:      ", the_stage); registers_fprintf(REGISTERS, stdout);
            break;
            
        case isa_6502_instr_stage_execution_complete:
            printf("\n[%04X] TOTAL CYCLES:   %llu\n", the_stage, the_cycle_count);
            break;
            
        case isa_6502_instr_stage_illegal_instruction:
            fprintf(stderr, "\n[%04X] HALT:  Illegal instruction at $%04hX: $%02hhX\n",
                    the_stage, REGISTERS->PC, the_opcode.BYTE);
            break;
    }
    
    #undef REGISTERS
    #undef MEMORY
    #undef ISA
}

//

uint64_t
executor_launch_at_address_range(
    executor_t                  *the_executor,
    executor_stage_callback_t   callback_fn,
    isa_6502_instr_stage_t      callback_stage_mask,
    uint16_t                    start_addr,
    uint16_t                    end_addr
)
{
    #define REGISTERS           the_executor->registers
    #define MEMORY              the_executor->memory
    #define ISA                 the_executor->isa
    
    uint64_t                    total_cycles = 0;
    uint32_t                    PC_end = end_addr;
    
    /* Load the starting address into the PC */
    if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_pre_load_PC) )
        callback_fn(the_executor, isa_6502_instr_stage_pre_load_PC,
                        isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, 0);
    REGISTERS->PC = start_addr;
    if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_post_load_PC) )
        callback_fn(the_executor, isa_6502_instr_stage_post_load_PC,
                        isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, 0);
    
    while ( REGISTERS->PC < PC_end ) {
        isa_6502_instr_context_t    instr_context = {
                                            .cycle_count = 0,
                                            .memory = MEMORY,
                                            .registers = REGISTERS
                                        };
        isa_6502_opcode_t           *opcode_ptr = &instr_context.opcode;
        isa_6502_opcode_dispatch_t  *dispatch;
        isa_6502_instr_stage_t      next_stage;
        
        /* Read the opcode */
        if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_pre_fetch_opcode) )
            callback_fn(the_executor, isa_6502_instr_stage_pre_fetch_opcode,
                            isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, 0);
        instr_context.opcode.BYTE = memory_read(MEMORY, REGISTERS->PC);
        instr_context.cycle_count++, total_cycles++;
        if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_post_fetch_opcode) )
            callback_fn(the_executor, isa_6502_instr_stage_post_fetch_opcode,
                            instr_context.opcode, isa_6502_addressing_undefined, NULL, instr_context.cycle_count);
        
        /* Decode the opcode: */
        if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_pre_decode_opcode) )
            callback_fn(the_executor, isa_6502_instr_stage_pre_decode_opcode,
                            instr_context.opcode, isa_6502_addressing_undefined, NULL, instr_context.cycle_count);
        dispatch = isa_6502_table_lookup_dispatch(ISA, opcode_ptr);
        if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_post_decode_opcode) )
            callback_fn(the_executor, isa_6502_instr_stage_post_decode_opcode,
                            instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count);
        
        /* Valid instruction? */
        if ( ! dispatch || (dispatch->addressing_mode == isa_6502_addressing_undefined) ) {
            if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_illegal_instruction) )
                callback_fn(the_executor, isa_6502_instr_stage_illegal_instruction,
                                instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count);
            break;
        }
        
        /* Fill-in the addressing mode in the context: */
        instr_context.addressing_mode = dispatch->addressing_mode;
        
        /* Loop through any additional cycles the instruction requires: */
        do {
            if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_pre_next_cycle) )
                callback_fn(the_executor, isa_6502_instr_stage_pre_next_cycle,
                                instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count);
            next_stage = dispatch->callback_fn(&instr_context, isa_6502_instr_stage_next_cycle);
            instr_context.cycle_count++, total_cycles++;
            if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_next_cycle) )
                callback_fn(the_executor, isa_6502_instr_stage_next_cycle,
                                instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count);
        } while ( next_stage != isa_6502_instr_stage_end);
        
        if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_end) )
            callback_fn(the_executor, isa_6502_instr_stage_end,
                            instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count);
        
        REGISTERS->PC++;
    }        
    if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_execution_complete) )
        callback_fn(the_executor, isa_6502_instr_stage_execution_complete,
                        isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, total_cycles);
    return total_cycles;
    
    #undef REGISTERS
    #undef MEMORY
    #undef ISA
}

//

uint64_t
executor_boot(
    executor_t                  *the_executor,
    executor_stage_callback_t   callback_fn,
    isa_6502_instr_stage_t      callback_stage_mask
)
{
    #define REGISTERS       the_executor->registers
    #define MEMORY          the_executor->memory
    #define ISA             the_executor->isa
    
    uint64_t                total_cycles = 0;
    int                     do_reset_jump = 1;
    uint32_t                PC_end = 0xFFFF;
    
    /* Load the starting address into the PC */
    if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_pre_load_PC) )
        callback_fn(the_executor, isa_6502_instr_stage_pre_load_PC,
                        isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, 0);
    REGISTERS->PC = MEMORY_ADDR_RES_VECTOR - 1;
    if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_post_load_PC) )
        callback_fn(the_executor, isa_6502_instr_stage_post_load_PC,
                        isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, 0);
    
    while ( REGISTERS->PC < PC_end ) {
        isa_6502_instr_context_t    instr_context = {
                                            .cycle_count = 0,
                                            .memory = MEMORY,
                                            .registers = REGISTERS
                                        };
        isa_6502_opcode_t           *opcode_ptr = &instr_context.opcode;
        isa_6502_opcode_dispatch_t  *dispatch;
        isa_6502_instr_stage_t      next_stage;
        
        if ( do_reset_jump ) {
            do_reset_jump = 0;
            /* JMP ($FFFC) */
            opcode_ptr->BYTE = 0x6C;
        } else {
            /* Read the opcode */
            if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_pre_fetch_opcode) )
                callback_fn(the_executor, isa_6502_instr_stage_pre_fetch_opcode,
                                isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, 0);
            instr_context.opcode.BYTE = memory_read(MEMORY, REGISTERS->PC);
            instr_context.cycle_count++, total_cycles++;
            if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_post_fetch_opcode) )
                callback_fn(the_executor, isa_6502_instr_stage_post_fetch_opcode,
                                instr_context.opcode, isa_6502_addressing_undefined, NULL, instr_context.cycle_count);
        }
        
        /* Decode the opcode: */
        if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_pre_decode_opcode) )
            callback_fn(the_executor, isa_6502_instr_stage_pre_decode_opcode,
                            instr_context.opcode, isa_6502_addressing_undefined, NULL, instr_context.cycle_count);
        dispatch = isa_6502_table_lookup_dispatch(ISA, opcode_ptr);
        if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_post_decode_opcode) )
            callback_fn(the_executor, isa_6502_instr_stage_post_decode_opcode,
                            instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count);
        
        /* Valid instruction? */
        if ( ! dispatch || (dispatch->addressing_mode == isa_6502_addressing_undefined) ) {
            if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_illegal_instruction) )
                callback_fn(the_executor, isa_6502_instr_stage_illegal_instruction,
                                instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count);
            break;
        }
        
        /* Fill-in the addressing mode in the context: */
        instr_context.addressing_mode = dispatch->addressing_mode;
        
        /* Loop through any additional cycles the instruction requires: */
        do {
            if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_pre_next_cycle) )
                callback_fn(the_executor, isa_6502_instr_stage_pre_next_cycle,
                                instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count);
            next_stage = dispatch->callback_fn(&instr_context, isa_6502_instr_stage_next_cycle);
            instr_context.cycle_count++, total_cycles++;
            if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_next_cycle) )
                callback_fn(the_executor, isa_6502_instr_stage_next_cycle,
                                instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count);
        } while ( next_stage != isa_6502_instr_stage_end);
        
        if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_end) )
            callback_fn(the_executor, isa_6502_instr_stage_end,
                            instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count);
        
        REGISTERS->PC++;
    }        
    if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_execution_complete) )
        callback_fn(the_executor, isa_6502_instr_stage_execution_complete,
                        isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, total_cycles);
    return total_cycles;
    
    #undef REGISTERS
    #undef MEMORY
    #undef ISA
}
