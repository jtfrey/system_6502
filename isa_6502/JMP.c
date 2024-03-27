
isa_6502_instr_stage_t
__isa_6502_JMP(
    isa_6502_instr_context_t    *opcode_context,
    isa_6502_instr_stage_t      at_stage
)
{
    static uint16_t ADDR;
    static uint8_t *ADDR_ptr;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr = ((uint8_t*)&ADDR);
#else
            ADDR_ptr = ((uint8_t*)&ADDR) + 1;
#endif
            /* Read low byte: */
            *ADDR_ptr = memory_read(opcode_context->memory, ++opcode_context->registers->PC);
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr++;
#else
            ADDR_ptr--;
#endif
            break;
        case 2:
            /* Read high byte: */
            *ADDR_ptr = memory_read(opcode_context->memory, ++opcode_context->registers->PC);
            if ( opcode_context->addressing_mode == isa_6502_addressing_absolute ) {
                opcode_context->registers->PC = ADDR - 1;
                at_stage = isa_6502_instr_stage_end;
            }
            break;
        case 3:
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr = ((uint8_t*)&opcode_context->registers->PC);
#else
            ADDR_ptr = ((uint8_t*)&opcode_context->registers->PC) + 1;
#endif
            /* Read low byte: */
            *ADDR_ptr = memory_read(opcode_context->memory, ADDR++);
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr++;
#else
            ADDR_ptr--;
#endif
            at_stage = isa_6502_instr_stage_next_cycle;
            break;
        case 4:
            /* Read high byte: */
            *ADDR_ptr = memory_read(opcode_context->memory, ADDR);
            opcode_context->registers->PC--;
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}
