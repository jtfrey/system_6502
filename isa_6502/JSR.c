
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
            /* Add 2 to PC */
            ADDR = opcode_context->registers->PC + 2;
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr = ((uint8_t*)&ADDR) + 1;
#else
            ADDR_ptr = ((uint8_t*)&ADDR);
#endif
            break;
        case 2:
            /* Push PC[hi] */
            __isa_6502_push(opcode_context->registers, opcode_context->memory, *ADDR_ptr);
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr--;
#else
            ADDR_ptr++;
#endif
            break;
        case 3:
            /* Push PC[lo] */
            __isa_6502_push(opcode_context->registers, opcode_context->memory, *ADDR_ptr);
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr = ((uint8_t*)&ADDR) + 1;
#else
            ADDR_ptr = ((uint8_t*)&ADDR);
#endif
            break;
        case 4:
            /* Read low byte: */
            *ADDR_ptr = memory_read(opcode_context->memory, ++opcode_context->registers->PC);
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr--;
#else
            ADDR_ptr++;
#endif
            break;
        case 5:
            /* Read high byte: */
            *ADDR_ptr = memory_read(opcode_context->memory, ++opcode_context->registers->PC);
            opcode_context->registers->PC = ADDR - 1;
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}
