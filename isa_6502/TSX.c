
ISA_6502_INSTR(TSX)
{
    opcode_context->registers->X = opcode_context->registers->SP;
    registers_did_set_X(opcode_context->registers, registers_Carry_ignore);
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(TSX)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "TSX");
#else
    return 0;
#endif
}
