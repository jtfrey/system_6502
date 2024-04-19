
ISA_6502_INSTR(TXS)
{
    opcode_context->registers->SP = opcode_context->registers->X;
    return isa_6502_instr_stage_end;
}

ISA_6502_STATIC_INSTR(TXS)
{
    opcode_context->cycle_count++;
    opcode_context->registers->SP = opcode_context->registers->X;
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(TXS)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "TXS");
#else
    return 0;
#endif
}
