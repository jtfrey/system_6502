
ISA_6502_INSTR(RTI)
{
    static uint16_t ADDR16;
    static uint8_t  SR;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            /* Pop the SR */
            SR = __isa_6502_pop(opcode_context->registers, opcode_context->memory);
            break;
        case 2:
            /* Clear the B/IGN flags */
            SR &= ~(register_SR_Bit_B | register_SR_Bit_IGN);
            break;
        case 3:
            opcode_context->registers->SR = SR;
            break;
        case 4:
            /* Pop low-byte of PC */
            ADDR16 = __isa_6502_pop(opcode_context->registers, opcode_context->memory);
            break;
        case 5:
            /* Pop high-byte of PC */
            opcode_context->registers->PC = ADDR16 | (__isa_6502_pop(opcode_context->registers, opcode_context->memory) << 8);
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}

ISA_6502_STATIC_INSTR(RTI)
{
    uint16_t ADDR16;
    uint8_t  SR;
    
    /* Pop the SR -- clear the B/IGN flags in the process: */
    opcode_context->registers->SR = __isa_6502_pop(opcode_context->registers, opcode_context->memory) & ~(register_SR_Bit_B | register_SR_Bit_IGN);
    /* Pop low-byte of PC */
    ADDR16 = __isa_6502_pop(opcode_context->registers, opcode_context->memory);
    /* Pop high-byte of PC */
    opcode_context->registers->PC = ADDR16 | (__isa_6502_pop(opcode_context->registers, opcode_context->memory) << 8);
    opcode_context->cycle_count += 5;
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(RTI)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "RTI [$%04hX]", opcode_context->registers->PC);
#else
    return 0;
#endif
}
