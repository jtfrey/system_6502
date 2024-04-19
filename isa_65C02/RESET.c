
ISA_65C02_INSTR(RESET)
{
    /* Clear BCD flag */
    registers_SR_set_bit(opcode_context->registers, register_SR_Bit_D, 0);
    opcode_context->registers->PC = membus_read_addr(opcode_context->memory, MEMORY_ADDR_RES_VECTOR) | (membus_read_addr(opcode_context->memory, MEMORY_ADDR_RES_VECTOR+1) << 8);
    return isa_6502_instr_stage_end;
}

ISA_65C02_STATIC_INSTR(RESET)
{
    /* Clear BCD flag */
    registers_SR_set_bit(opcode_context->registers, register_SR_Bit_D, 0);
    opcode_context->registers->PC = membus_read_addr(opcode_context->memory, MEMORY_ADDR_RES_VECTOR) | (membus_read_addr(opcode_context->memory, MEMORY_ADDR_RES_VECTOR+1) << 8);
    return isa_6502_instr_stage_end;
}

ISA_65C02_DISASM(RESET)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "RESET");
#else
    return 0;
#endif
}