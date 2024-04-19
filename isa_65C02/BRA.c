
ISA_65C02_INSTR(BRA)
{
    static uint16_t     ADDR16;
    static uint8_t      DELTA;
       
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            DELTA = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            break;
        
        case 2:
            ADDR16 = opcode_context->registers->PC + *((int8_t*)&DELTA);
            if ( (ADDR16 & 0xFF00) == (opcode_context->registers->PC & 0xFF00) ) at_stage = isa_6502_instr_stage_end;
            break;
        
        case 3:
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    if ( at_stage == isa_6502_instr_stage_end ) opcode_context->registers->PC = ADDR16;
    return at_stage;
}

ISA_65C02_STATIC_INSTR(BRA)
{
    uint8_t         DELTA = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
    uint16_t        ADDR16 = opcode_context->registers->PC + *((int8_t*)&DELTA);
    opcode_context->cycle_count += 2 + ((ADDR16 & 0xFF00) != (opcode_context->registers->PC & 0xFF00));
    return isa_6502_instr_stage_end;
}

ISA_65C02_DISASM(BRA)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t                     operand = membus_rcache_pop(opcode_context->memory);
    
    return snprintf(buffer, buffer_len, "BRA *%+hhd", *((int8_t*)&operand));
#else
    return 0;
#endif
}
