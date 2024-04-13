
ISA_65C02_INSTR(PHX)
{
    static uint8_t X;
    
    at_stage = isa_6502_instr_stage_next_cycle;
            
    switch ( opcode_context->cycle_count ) {
        case 1:
            X = opcode_context->registers->X;
            break;
        case 2:
            __isa_6502_push(opcode_context->registers, opcode_context->memory, X);
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}

ISA_65C02_DISASM(PHX)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "PHX");
#else
    return 0;
#endif
}
