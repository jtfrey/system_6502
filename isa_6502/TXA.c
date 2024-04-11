
ISA_6502_INSTR(TXA)
{
    opcode_context->registers->A = opcode_context->registers->X;
    registers_did_set_A(opcode_context->registers, registers_Carry_ignore);
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(TXA)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "TXA");
#else
    return 0;
#endif
}
