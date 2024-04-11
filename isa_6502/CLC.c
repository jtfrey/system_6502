
ISA_6502_INSTR(CLC)
{
    registers_SR_set_bit(opcode_context->registers, register_SR_Bit_C, 0);
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(CLC)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "CLC");
#else
    return 0;
#endif
}
