
ISA_6502_INSTR(RTS)
{
    static uint16_t ADDR16;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            /* Pop low-byte of PC */
            ADDR16 = __isa_6502_pop(opcode_context->registers, opcode_context->memory);
            break;
        case 2:
            /* Waste a cycle: */
            break;
        case 3:
            /* Pop high-byte of PC */
            ADDR16 |= __isa_6502_pop(opcode_context->registers, opcode_context->memory) << 8;
            break;
        case 4:
            /* Waste a cycle: */
            break;
        case 5:
            /* Set register */
            opcode_context->registers->PC = ADDR16;
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}

ISA_6502_STATIC_INSTR(RTS)
{
    uint16_t    ADDR16;
    
    /* Pop low-byte of PC */
    ADDR16 = __isa_6502_pop(opcode_context->registers, opcode_context->memory);
    /* Pop high-byte of PC */
    ADDR16 |= __isa_6502_pop(opcode_context->registers, opcode_context->memory) << 8;
    /* Set register */
    opcode_context->registers->PC = ADDR16;
    opcode_context->cycle_count += 5;
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(RTS)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "RTS [$%04hX]", opcode_context->registers->PC);
#else
    return 0;
#endif
}
