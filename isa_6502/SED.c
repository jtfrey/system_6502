
ISA_6502_INSTR(SED)
{
    registers_SR_set_bit(opcode_context->registers, register_SR_Bit_D, 1);
    return isa_6502_instr_stage_end;
}

ISA_6502_STATIC_INSTR(SED)
{
    registers_SR_set_bit(opcode_context->registers, register_SR_Bit_D, 1);
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(SED)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "SED");
#else
    return 0;
#endif
}
