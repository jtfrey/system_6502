
#include "executor.h"
#include "membus_module_std64k.h"

typedef struct {
    executor_t          pointers;
    registers_t         registers;
    membus_t            *memory;
    isa_6502_table_t    isa;
} executor_bundle_t;

enum {
    executor_flags_is_bundle = 1 << 0,
    //
    executor_flags_nmi_is_set = 1 << 1,
    executor_flags_irq_is_set = 1 << 2,
};

executor_t*
executor_alloc_with_default_components(void)
{
    executor_bundle_t   *new_executor = (executor_bundle_t*)malloc(sizeof(executor_bundle_t));
    
    if ( new_executor ) {
        membus_module_ref   base64k = membus_module_std64k_alloc();
        
        /* Base 64k under all else: */
        new_executor->memory = membus_alloc();
        membus_register_module(new_executor->memory, 0, base64k);
        
        new_executor->pointers.flags = executor_flags_is_bundle;
        new_executor->pointers.registers = &new_executor->registers;
        new_executor->pointers.memory = new_executor->memory;
        new_executor->pointers.isa = &new_executor->isa;
        
        registers_init(&new_executor->registers);
        
        isa_6502_table_init(&new_executor->isa, isa_6502_dialect_base);
        
#ifdef ENABLE_EXECUTOR_LOCKS
        pthread_mutex_init(&new_executor->pointers.state_lock, NULL);
#endif
    }
    return (executor_t*)new_executor;
}

//

executor_t*
executor_alloc_with_components(
    registers_t         *the_registers,
    membus_t            *the_memory,
    isa_6502_table_t    *the_isa
)
{
    executor_t  *new_executor = (executor_t*)malloc(sizeof(executor_t));
    
    if ( new_executor ) {
        new_executor->flags = 0;
        new_executor->registers = the_registers;
        new_executor->memory = the_memory;
        new_executor->isa = the_isa;
#ifdef ENABLE_EXECUTOR_LOCKS
        pthread_mutex_init(&new_executor->state_lock, NULL);
#endif
    }
    return (executor_t*)new_executor;
}

//

void
executor_free(
    executor_t      *the_executor
)
{
#ifdef ENABLE_EXECUTOR_LOCKS
    pthread_mutex_lock(&the_executor->state_lock);
#endif
    if ( (the_executor->flags & executor_flags_is_bundle) != executor_flags_is_bundle ) {
        registers_free(the_executor->registers);
        membus_free(the_executor->memory);
        isa_6502_table_free(the_executor->isa);
    }
#ifdef ENABLE_EXECUTOR_LOCKS
    pthread_mutex_unlock(&the_executor->state_lock);
    pthread_mutex_destroy(&the_executor->state_lock);
#endif
    free((void*)the_executor);
}

//

void
executor_hard_reset(
    executor_t      *the_executor
)
{
#ifdef ENABLE_EXECUTOR_LOCKS
    pthread_mutex_lock(&the_executor->state_lock);
#endif
    
    /* Reset registers */
    registers_reset(the_executor->registers);
    
#ifdef ENABLE_EXECUTOR_LOCKS
    pthread_mutex_unlock(&the_executor->state_lock);
#endif
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
    uint64_t                    the_cycle_count,
    const char                  *disasm,
    int                         disasm_len
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
    
#ifdef ENABLE_DISASSEMBLY
    char                        disasm_buffer[64];
#endif
    uint64_t                    total_cycles = 0;
    uint32_t                    PC_end = end_addr;
    
#ifdef ENABLE_EXECUTOR_LOCKS
    pthread_mutex_lock(&the_executor->state_lock);
#endif
    
    /* Load the starting address into the PC */
    if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_pre_load_PC) )
        callback_fn(the_executor, isa_6502_instr_stage_pre_load_PC,
                        isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, 0, NULL, 0);
    REGISTERS->PC = start_addr;
    if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_post_load_PC) )
        callback_fn(the_executor, isa_6502_instr_stage_post_load_PC,
                        isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, 0, NULL, 0);
    
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
                            isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, 0, NULL, 0);
        instr_context.opcode.BYTE = membus_read_addr(MEMORY, REGISTERS->PC++);
        instr_context.cycle_count++, total_cycles++;
        if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_post_fetch_opcode) )
            callback_fn(the_executor, isa_6502_instr_stage_post_fetch_opcode,
                            instr_context.opcode, isa_6502_addressing_undefined, NULL, instr_context.cycle_count, NULL, 0);
        
        /* Decode the opcode: */
        if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_pre_decode_opcode) )
            callback_fn(the_executor, isa_6502_instr_stage_pre_decode_opcode,
                            instr_context.opcode, isa_6502_addressing_undefined, NULL, instr_context.cycle_count, NULL, 0);
        dispatch = isa_6502_table_lookup_dispatch(ISA, opcode_ptr);
        if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_post_decode_opcode) )
            callback_fn(the_executor, isa_6502_instr_stage_post_decode_opcode,
                            instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count, NULL, 0);
        
        /* Valid instruction? */
        if ( ! dispatch || (dispatch->addressing_mode == isa_6502_addressing_undefined) ) {
            if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_illegal_instruction) )
                callback_fn(the_executor, isa_6502_instr_stage_illegal_instruction,
                                instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count, NULL, 0);
            break;
        }
        
        /* Fill-in the addressing mode in the context: */
        instr_context.addressing_mode = dispatch->addressing_mode;
        
        /* Loop through any additional cycles the instruction requires: */
        do {
            if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_pre_next_cycle) )
                callback_fn(the_executor, isa_6502_instr_stage_pre_next_cycle,
                                instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count, NULL, 0);
            next_stage = dispatch->exec_fn(&instr_context, isa_6502_instr_stage_next_cycle);
            instr_context.cycle_count++, total_cycles++;
            if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_next_cycle) )
                callback_fn(the_executor, isa_6502_instr_stage_next_cycle,
                                instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count, NULL, 0);
        } while ( next_stage != isa_6502_instr_stage_end);

#ifdef ENABLE_DISASSEMBLY
        if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_disasm) && dispatch->disasm_fn) {
            int     disasm_len = dispatch->disasm_fn(&instr_context, disasm_buffer, sizeof(disasm_buffer));
            
            callback_fn(the_executor, isa_6502_instr_stage_disasm,
                        instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count,
                        disasm_buffer, disasm_len);
        }
#endif
        
        if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_end) )
            callback_fn(the_executor, isa_6502_instr_stage_end,
                            instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count, NULL, 0);
        
        /* Drop the lock then reacquire to check for NMI/IRQ */
#ifdef ENABLE_EXECUTOR_LOCKS
        pthread_mutex_unlock(&the_executor->state_lock);
        pthread_mutex_lock(&the_executor->state_lock);
#endif
        if ( (the_executor->flags & executor_flags_irq_is_set) ) {
            if ( ! registers_SR_get_bit(the_executor->registers, register_SR_Bit_I) ) {
                /* Service the IRQ */
                if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_enter_irq) )
                    callback_fn(the_executor, isa_6502_instr_stage_enter_irq,
                                    isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, total_cycles, NULL, 0);
                if ( ISA->irq ) {
                    instr_context.cycle_count = 1;
                    instr_context.opcode.BYTE = 0;
                    do {
                        next_stage = ISA->irq(&instr_context, isa_6502_instr_stage_next_cycle);
                        instr_context.cycle_count++, total_cycles++;
                    } while ( next_stage != isa_6502_instr_stage_end);
                }
                if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_exec_irq) )
                    callback_fn(the_executor, isa_6502_instr_stage_exec_irq,
                                    isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, total_cycles, NULL, 0);
            }
        }
        if ( (the_executor->flags & executor_flags_nmi_is_set) ) {
            /* Service the NMI */
            if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_enter_nmi) )
                callback_fn(the_executor, isa_6502_instr_stage_enter_nmi,
                                isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, total_cycles, NULL, 0);
            if ( ISA->nmi ) {
                instr_context.cycle_count = 1;
                instr_context.opcode.BYTE = 0;
                do {
                    next_stage = ISA->nmi(&instr_context, isa_6502_instr_stage_next_cycle);
                    instr_context.cycle_count++, total_cycles++;
                } while ( next_stage != isa_6502_instr_stage_end);
            }
            if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_exec_nmi) )
                callback_fn(the_executor, isa_6502_instr_stage_exec_nmi,
                                isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, total_cycles, NULL, 0);
        }
        the_executor->flags &= ~(executor_flags_irq_is_set | executor_flags_nmi_is_set);
    }        
    if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_execution_complete) )
        callback_fn(the_executor, isa_6502_instr_stage_execution_complete,
                        isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, total_cycles, NULL, 0);
    return total_cycles;
    
    #undef REGISTERS
    #undef MEMORY
    #undef ISA
}

//

uint64_t
executor_soft_reset(
    executor_t                  *the_executor,
    executor_stage_callback_t   callback_fn,
    isa_6502_instr_stage_t      callback_stage_mask
)
{
    #define REGISTERS           the_executor->registers
    #define MEMORY              the_executor->memory
    #define ISA                 the_executor->isa
    
#ifdef ENABLE_DISASSEMBLY
    char                        disasm_buffer[64];
#endif
    uint64_t                    total_cycles = 0;
    uint32_t                    PC_end = 0xFFFF;
    
#ifdef ENABLE_EXECUTOR_LOCKS
    pthread_mutex_lock(&the_executor->state_lock);
#endif
    
    /* Load the value of the RESET vector into the PC: */
    if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_pre_load_PC) )
        callback_fn(the_executor, isa_6502_instr_stage_pre_load_PC,
                        isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, 0, NULL, 0);
    REGISTERS->PC = MEMORY->res_vector;
    if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_post_load_PC) )
        callback_fn(the_executor, isa_6502_instr_stage_post_load_PC,
                        isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, 0, NULL, 0);
    
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
                            isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, 0, NULL, 0);
        instr_context.opcode.BYTE = membus_read_addr(MEMORY, REGISTERS->PC++);
        instr_context.cycle_count++, total_cycles++;
        if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_post_fetch_opcode) )
            callback_fn(the_executor, isa_6502_instr_stage_post_fetch_opcode,
                            instr_context.opcode, isa_6502_addressing_undefined, NULL, instr_context.cycle_count, NULL, 0);
                            
        /* Decode the opcode: */
        if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_pre_decode_opcode) )
            callback_fn(the_executor, isa_6502_instr_stage_pre_decode_opcode,
                            instr_context.opcode, isa_6502_addressing_undefined, NULL, instr_context.cycle_count, NULL, 0);
        dispatch = isa_6502_table_lookup_dispatch(ISA, opcode_ptr);
        if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_post_decode_opcode) )
            callback_fn(the_executor, isa_6502_instr_stage_post_decode_opcode,
                            instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count, NULL, 0);
        
        /* Valid instruction? */
        if ( ! dispatch || (dispatch->addressing_mode == isa_6502_addressing_undefined) ) {
            if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_illegal_instruction) )
                callback_fn(the_executor, isa_6502_instr_stage_illegal_instruction,
                                instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count, NULL, 0);
            break;
        }
        
        /* Fill-in the addressing mode in the context: */
        instr_context.addressing_mode = dispatch->addressing_mode;
        
        /* Loop through any additional cycles the instruction requires: */
        do {
            if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_pre_next_cycle) )
                callback_fn(the_executor, isa_6502_instr_stage_pre_next_cycle,
                                instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count, NULL, 0);
            next_stage = dispatch->exec_fn(&instr_context, isa_6502_instr_stage_next_cycle);
            instr_context.cycle_count++, total_cycles++;
            if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_next_cycle) )
                callback_fn(the_executor, isa_6502_instr_stage_next_cycle,
                                instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count, NULL, 0);
        } while ( next_stage != isa_6502_instr_stage_end);

#ifdef ENABLE_DISASSEMBLY
        if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_disasm) && dispatch->disasm_fn) {
            int     disasm_len = dispatch->disasm_fn(&instr_context, disasm_buffer, sizeof(disasm_buffer));
            
            callback_fn(the_executor, isa_6502_instr_stage_disasm,
                        instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count,
                        disasm_buffer, disasm_len);
        }
#endif

        if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_end) )
            callback_fn(the_executor, isa_6502_instr_stage_end,
                            instr_context.opcode, dispatch->addressing_mode, dispatch, instr_context.cycle_count, NULL, 0);
        
        /* Drop the lock then reacquire to check for NMI/IRQ */        
#ifdef ENABLE_EXECUTOR_LOCKS
        pthread_mutex_unlock(&the_executor->state_lock);
        pthread_mutex_lock(&the_executor->state_lock);
#endif
        if ( (the_executor->flags & executor_flags_irq_is_set) ) {
            if ( ! registers_SR_get_bit(the_executor->registers, register_SR_Bit_I) ) {
                /* Service the IRQ */
                if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_enter_irq) )
                    callback_fn(the_executor, isa_6502_instr_stage_enter_irq,
                                    isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, total_cycles, NULL, 0);
                if ( ISA->irq ) {
                    instr_context.cycle_count = 1;
                    instr_context.opcode.BYTE = 0;
                    do {
                        next_stage = ISA->irq(&instr_context, isa_6502_instr_stage_next_cycle);
                        instr_context.cycle_count++, total_cycles++;
                    } while ( next_stage != isa_6502_instr_stage_end);
                }
                if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_exec_irq) )
                    callback_fn(the_executor, isa_6502_instr_stage_exec_irq,
                                    isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, total_cycles, NULL, 0);
            }
        }
        if ( (the_executor->flags & executor_flags_nmi_is_set) ) {
            /* Service the NMI */
            if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_enter_nmi) )
                callback_fn(the_executor, isa_6502_instr_stage_enter_nmi,
                                isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, total_cycles, NULL, 0);
            if ( ISA->nmi ) {
                instr_context.cycle_count = 1;
                instr_context.opcode.BYTE = 0;
                do {
                    next_stage = ISA->nmi(&instr_context, isa_6502_instr_stage_next_cycle);
                    instr_context.cycle_count++, total_cycles++;
                } while ( next_stage != isa_6502_instr_stage_end);
            }
            if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_exec_nmi) )
                callback_fn(the_executor, isa_6502_instr_stage_exec_nmi,
                                isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, total_cycles, NULL, 0);
        }
        the_executor->flags &= ~(executor_flags_irq_is_set | executor_flags_nmi_is_set);
    }        
    if ( callback_fn && (callback_stage_mask & isa_6502_instr_stage_execution_complete) )
        callback_fn(the_executor, isa_6502_instr_stage_execution_complete,
                        isa_6502_opcode_null(), isa_6502_addressing_undefined, NULL, total_cycles, NULL, 0);
    return total_cycles;
    
    #undef REGISTERS
    #undef MEMORY
    #undef ISA
}

//

void
executor_set_irq(
    executor_t  *the_executor
)
{      
#ifdef ENABLE_EXECUTOR_LOCKS
    pthread_mutex_lock(&the_executor->state_lock);
#endif
    the_executor->flags |= executor_flags_irq_is_set;
#ifdef ENABLE_EXECUTOR_LOCKS
    pthread_mutex_unlock(&the_executor->state_lock);
#endif
}

//

void
executor_set_nmi(
    executor_t  *the_executor
)
{      
#ifdef ENABLE_EXECUTOR_LOCKS
    pthread_mutex_lock(&the_executor->state_lock);
#endif
    the_executor->flags |= executor_flags_nmi_is_set;
#ifdef ENABLE_EXECUTOR_LOCKS
    pthread_mutex_unlock(&the_executor->state_lock);
#endif
}