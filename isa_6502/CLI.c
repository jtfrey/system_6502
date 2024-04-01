
isa_6502_instr_stage_t
__isa_6502_CLI(
    isa_6502_instr_context_t    *opcode_context,
    isa_6502_instr_stage_t      at_stage
)
{
    registers_SR_set_bit(opcode_context->registers, register_SR_Bit_I, 0);
    return isa_6502_instr_stage_end;
}

int
__isa_6502_disasm_CLI(
    isa_6502_instr_context_t    *opcode_context,
    char                        *buffer,
    int                         buffer_len
)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "CLI");
#else
    return 0;
#endif
}
