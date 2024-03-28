
isa_6502_instr_stage_t
__isa_6502_SEC(
    isa_6502_instr_context_t    *opcode_context,
    isa_6502_instr_stage_t      at_stage
)
{
    registers_SR_set_bit(opcode_context->registers, register_SR_Bit_C, 1);
    return isa_6502_instr_stage_end;
}
