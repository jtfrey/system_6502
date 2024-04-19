
ISA_6502_INSTR(PHP)
{
    static uint8_t SR;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            SR = opcode_context->registers->SR | register_SR_Bit_B | register_SR_Bit_IGN;
            break;
        case 2:
            __isa_6502_push(opcode_context->registers, opcode_context->memory, SR);
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}

ISA_6502_STATIC_INSTR(PHP)
{
    __isa_6502_push(opcode_context->registers, opcode_context->memory, opcode_context->registers->SR | register_SR_Bit_B | register_SR_Bit_IGN);
    opcode_context->cycle_count += 2;
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(PHP)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "PHP");
#else
    return 0;
#endif
}
