
ISA_6502_INSTR(INY)
{
    opcode_context->registers->Y++;
    registers_did_set_Y(opcode_context->registers, registers_Carry_ignore);
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(INY)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "INY {Y + 1 = $%02hhX}", opcode_context->registers->Y);
#else
    return 0;
#endif
}
