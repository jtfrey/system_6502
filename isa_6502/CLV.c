
ISA_6502_INSTR(CLV)
{
    registers_SR_set_bit(opcode_context->registers, register_SR_Bit_V, 0);
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(CLV)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "CLV");
#else
    return 0;
#endif
}
