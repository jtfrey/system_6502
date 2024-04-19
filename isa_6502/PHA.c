
ISA_6502_INSTR(PHA)
{
    static uint8_t A;
    
    at_stage = isa_6502_instr_stage_next_cycle;
            
    switch ( opcode_context->cycle_count ) {
        case 1:
            A = opcode_context->registers->A;
            break;
        case 2:
            __isa_6502_push(opcode_context->registers, opcode_context->memory, A);
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}

ISA_6502_STATIC_INSTR(PHA)
{
    __isa_6502_push(opcode_context->registers, opcode_context->memory, opcode_context->registers->A);
    opcode_context->cycle_count += 2;
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(PHA)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "PHA");
#else
    return 0;
#endif
}
