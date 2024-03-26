
isa_6502_instr_stage_t
__isa_6502_PHA(
    isa_6502_instr_context_t    *opcode_context,
    isa_6502_instr_stage_t      at_stage
)
{
    static uint8_t A;
    
    at_stage = isa_6502_instr_stage_next_cycle;
            
    switch ( opcode_context->cycle_count ) {
        case 1:
            A = opcode_context->registers->A;
            break;
        case 2:
            __isa_6502_push(opcode_context->registers, opcode_context->memory, A);
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}
