
ISA_6502_INSTR(PLP)
{
    static uint8_t SR;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            /* Pop SR */
            SR = __isa_6502_pop(opcode_context->registers, opcode_context->memory);
            break;
        case 2:
            SR &= ~(register_SR_Bit_B | register_SR_Bit_IGN);
            break;
        case 3:
            /* Clear B/IGN */
            opcode_context->registers->SR = SR;
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}

ISA_6502_STATIC_INSTR(PLP)
{
    uint8_t     SR = __isa_6502_pop(opcode_context->registers, opcode_context->memory);
    
    opcode_context->registers->SR = SR & ~(register_SR_Bit_B | register_SR_Bit_IGN);
    opcode_context->cycle_count += 3;
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(PLP)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "PLP");
#else
    return 0;
#endif
}
