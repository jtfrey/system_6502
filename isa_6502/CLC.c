
isa_6502_instr_stage_t
__isa_6502_CLC(
    isa_6502_instr_context_t    *opcode_context,
    isa_6502_instr_stage_t      at_stage
)
{
    opcode_context->registers->SR.FIELDS.C = 0;
    return isa_6502_instr_stage_end;
}
