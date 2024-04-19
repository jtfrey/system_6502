
ISA_6502_INSTR(DEX)
{
    opcode_context->registers->X--;
    registers_did_set_X(opcode_context->registers, registers_Carry_ignore);
    return isa_6502_instr_stage_end;
}

ISA_6502_STATIC_INSTR(DEX)
{
    opcode_context->cycle_count++;
    opcode_context->registers->X--;
    registers_did_set_X(opcode_context->registers, registers_Carry_ignore);
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(DEX)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "DEX {X - 1 = $%02hhX}", opcode_context->registers->X);
#else
    return 0;
#endif
}
