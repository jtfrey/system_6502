
isa_6502_instr_stage_t
__isa_6502_JSR(
    isa_6502_instr_context_t    *opcode_context,
    isa_6502_instr_stage_t      at_stage
)
{
    static uint16_t ADDR;
    static uint8_t *ADDR_ptr;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            /* Read low byte: */
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr = ((uint8_t*)&ADDR);
#else
            ADDR_ptr = ((uint8_t*)&ADDR) + 1;
#endif
            *ADDR_ptr = memory_read(opcode_context->memory, opcode_context->registers->PC++);
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr++;
#else
            ADDR_ptr--;
#endif
            break;
        case 2:
            /* Read high byte: */
            *ADDR_ptr = memory_read(opcode_context->memory, opcode_context->registers->PC++);
            break;
        case 3:
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr = ((uint8_t*)&opcode_context->registers->PC) + 1;
#else
            ADDR_ptr = ((uint8_t*)&opcode_context->registers->PC);
#endif
            /* Push PC[hi] */
            __isa_6502_push(opcode_context->registers, opcode_context->memory, *ADDR_ptr);
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr--;
#else
            ADDR_ptr++;
#endif
            break;
        case 4:
            /* Push PC[lo] */
            __isa_6502_push(opcode_context->registers, opcode_context->memory, *ADDR_ptr);
            break;
        case 5:
            /* Set PC to target address: */
            opcode_context->registers->PC = ADDR;
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}
