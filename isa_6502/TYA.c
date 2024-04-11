
ISA_6502_INSTR(TYA)
{
    opcode_context->registers->A = opcode_context->registers->Y;
    registers_did_set_A(opcode_context->registers, registers_Carry_ignore);
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(TYA)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "TYA");
#else
    return 0;
#endif
}
