
ISA_6502_INSTR(CLD)
{
    registers_SR_set_bit(opcode_context->registers, register_SR_Bit_D, 0);
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(CLD)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "CLD");
#else
    return 0;
#endif
}
