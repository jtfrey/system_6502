
isa_6502_instr_stage_t
__isa_6502_TAX(
    isa_6502_instr_context_t    *opcode_context,
    isa_6502_instr_stage_t      at_stage
)
{
    opcode_context->registers->X = opcode_context->registers->A;
    registers_did_set_X(opcode_context->registers, registers_Carry_ignore);
    return isa_6502_instr_stage_end;
}

int
__isa_6502_disasm_TAX(
    isa_6502_instr_context_t    *opcode_context,
    char                        *buffer,
    int                         buffer_len
)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "TAX");
#else
    return 0;
#endif
}
