
ISA_6502_INSTR(NOP)
{
    /* Burned a cycle: */
    return isa_6502_instr_stage_end;
}

ISA_6502_STATIC_INSTR(NOP)
{
    opcode_context->cycle_count++;
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(NOP)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "NOP");
#else
    return 0;
#endif
}
