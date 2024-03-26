
isa_6502_instr_stage_t
__isa_6502_PHP(
    isa_6502_instr_context_t    *opcode_context,
    isa_6502_instr_stage_t      at_stage
)
{
    static uint8_t SR;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            SR = opcode_context->registers->SR.BYTE | register_SR_Bit_B | register_SR_Bit_IGN;
            break;
        case 2:
            __isa_6502_push(opcode_context->registers, opcode_context->memory, SR);
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}
