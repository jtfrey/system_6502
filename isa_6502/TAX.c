
ISA_6502_INSTR(TAX)
{
    opcode_context->registers->X = opcode_context->registers->A;
    registers_did_set_X(opcode_context->registers, registers_Carry_ignore);
    return isa_6502_instr_stage_end;
}

ISA_6502_STATIC_INSTR(TAX)
{
    opcode_context->cycle_count++;
    opcode_context->registers->X = opcode_context->registers->A;
    registers_did_set_X(opcode_context->registers, registers_Carry_ignore);
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(TAX)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "TAX");
#else
    return 0;
#endif
}
