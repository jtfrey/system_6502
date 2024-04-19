
ISA_6502_INSTR(CLI)
{
    registers_SR_set_bit(opcode_context->registers, register_SR_Bit_I, 0);
    return isa_6502_instr_stage_end;
}

ISA_6502_STATIC_INSTR(CLI)
{
    opcode_context->cycle_count++;
    registers_SR_set_bit(opcode_context->registers, register_SR_Bit_I, 0);
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(CLI)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "CLI");
#else
    return 0;
#endif
}
