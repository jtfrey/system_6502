
ISA_6502_INSTR(SEC)
{
    registers_SR_set_bit(opcode_context->registers, register_SR_Bit_C, 1);
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(SEC)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "SEC");
#else
    return 0;
#endif
}
