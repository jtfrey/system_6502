
ISA_6502_INSTR(RTS)
{
    static uint16_t ADDR;
    static uint8_t *ADDR_ptr;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            /* Prep for read: */
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr = ((uint8_t*)&ADDR);
#else
            ADDR_ptr = ((uint8_t*)&ADDR) + 1;
#endif
            /* Pop low-byte of PC */
            *ADDR_ptr = __isa_6502_pop(opcode_context->registers, opcode_context->memory);
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr++;
#else
            ADDR_ptr--;
#endif
            break;
        case 2:
            /* Memory cycle: */
            break;
        case 3:
            /* Pop high-byte of PC */
            *ADDR_ptr = __isa_6502_pop(opcode_context->registers, opcode_context->memory);
            break;
        case 4:
            /* Memory cycle: */
            break;
        case 5:
            /* Set register */
            opcode_context->registers->PC = ADDR;
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}

ISA_6502_DISASM(RTS)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "RTS [$%04hX]", opcode_context->registers->PC);
#else
    return 0;
#endif
}
