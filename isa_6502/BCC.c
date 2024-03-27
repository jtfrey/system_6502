
isa_6502_instr_stage_t
__isa_6502_BCC(
    isa_6502_instr_context_t    *opcode_context,
    isa_6502_instr_stage_t      at_stage
)
{
    static uint16_t     ADDR = 0x0000;
    static uint8_t      DELTA;
       
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            ADDR = 0x0000;
            DELTA = memory_read(opcode_context->memory, opcode_context->registers->PC++); 
            break;
        
        case 2:
            if ( DELTA & 0x80 ) {
                DELTA = (~DELTA + 1);
                ADDR = opcode_context->registers->PC - DELTA;
            } else {
                ADDR = opcode_context->registers->PC + DELTA;
            }
            if ( (ADDR & 0xFF00) == (opcode_context->registers->PC & 0xFF00) ) {
                at_stage = isa_6502_instr_stage_end;
            }
            break;
        
        case 3:
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    if ( at_stage == isa_6502_instr_stage_end ) {
        if ( ! opcode_context->registers->SR.FIELDS.C ) opcode_context->registers->PC = ADDR;
    }
    return at_stage;
}
