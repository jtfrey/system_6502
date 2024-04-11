
ISA_6502_INSTR(INX)
{
    opcode_context->registers->X++;
    registers_did_set_X(opcode_context->registers, registers_Carry_ignore);
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(INX)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "INX {X + 1 = $%02hhX}", opcode_context->registers->X);
#else
    return 0;
#endif
}

