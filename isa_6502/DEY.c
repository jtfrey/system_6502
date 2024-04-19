
ISA_6502_INSTR(DEY)
{
    opcode_context->registers->Y--;
    registers_did_set_Y(opcode_context->registers, registers_Carry_ignore);
    return isa_6502_instr_stage_end;
}

ISA_6502_STATIC_INSTR(DEY)
{
    opcode_context->cycle_count++;
    opcode_context->registers->Y--;
    registers_did_set_Y(opcode_context->registers, registers_Carry_ignore);
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(DEY)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "DEY {Y - 1 = $%02hhX}", opcode_context->registers->Y);
#else
    return 0;
#endif
}

