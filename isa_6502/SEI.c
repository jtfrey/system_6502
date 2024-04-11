
ISA_6502_INSTR(SEI)
{
    registers_SR_set_bit(opcode_context->registers, register_SR_Bit_I, 1);
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(SEI)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "SEI");
#else
    return 0;
#endif
}
