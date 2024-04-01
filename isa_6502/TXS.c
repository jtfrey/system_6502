
isa_6502_instr_stage_t
__isa_6502_TXS(
    isa_6502_instr_context_t    *opcode_context,
    isa_6502_instr_stage_t      at_stage
)
{
    opcode_context->registers->SP = opcode_context->registers->X;
    return isa_6502_instr_stage_end;
}

int
__isa_6502_disasm_TXS(
    isa_6502_instr_context_t    *opcode_context,
    char                        *buffer,
    int                         buffer_len
)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "TXS");
#else
    return 0;
#endif
}
