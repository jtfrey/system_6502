
ISA_6502_INSTR(TAY)
{
    opcode_context->registers->Y = opcode_context->registers->A;
    registers_did_set_Y(opcode_context->registers, registers_Carry_ignore);
    return isa_6502_instr_stage_end;
}

ISA_6502_STATIC_INSTR(TAY)
{
    opcode_context->cycle_count++;
    opcode_context->registers->Y = opcode_context->registers->A;
    registers_did_set_Y(opcode_context->registers, registers_Carry_ignore);
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(TAY)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "TAY");
#else
    return 0;
#endif
}
