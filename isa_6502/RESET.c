
ISA_6502_INSTR(RESET)
{
    opcode_context->registers->PC = membus_read_addr(opcode_context->memory, MEMORY_ADDR_RES_VECTOR) | (membus_read_addr(opcode_context->memory, MEMORY_ADDR_RES_VECTOR+1) << 8);
    return isa_6502_instr_stage_end;
}

ISA_6502_STATIC_INSTR(RESET)
{
    opcode_context->registers->PC = membus_read_addr(opcode_context->memory, MEMORY_ADDR_RES_VECTOR) | (membus_read_addr(opcode_context->memory, MEMORY_ADDR_RES_VECTOR+1) << 8);
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(RESET)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "RESET");
#else
    return 0;
#endif
}