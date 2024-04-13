
ISA_6502_INSTR(RESET)
{
    opcode_context->registers->PC = opcode_context->memory->res_vector;
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