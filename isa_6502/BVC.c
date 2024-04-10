
isa_6502_instr_stage_t
__isa_6502_BVC(
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
            DELTA = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++); 
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
        if ( ! registers_SR_get_bit(opcode_context->registers, register_SR_Bit_V) ) opcode_context->registers->PC = ADDR;
    }
    return at_stage;
}

int
__isa_6502_disasm_BVC(
    isa_6502_instr_context_t    *opcode_context,
    char                        *buffer,
    int                         buffer_len
)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t                     operand = membus_rcache_pop(opcode_context->memory);
    
    return snprintf(buffer, buffer_len, "BVC %hhd", *((int8_t*)&operand));
#else
    return 0;
#endif
}
