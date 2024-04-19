
ISA_6502_INSTR(STY)
{
    static uint16_t ADDR16;
    static uint8_t  ADDR8;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage:
                case isa_6502_addressing_zeropage_x_indexed:
                    ADDR8 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
                    break;
                case isa_6502_addressing_absolute:
                    ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
                    break;
            }
            break;
        
        case 2:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage:
                    ADDR16 = ADDR8;
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_zeropage_x_indexed:
                    ADDR8 += opcode_context->registers->X;
                    break;
                case isa_6502_addressing_absolute:
                    ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
                    break;
            }
            break;
            
        
        case 3:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage_x_indexed:
                    ADDR16 = ADDR8;
                    break;
                case isa_6502_addressing_absolute:
                    break;
            }
            at_stage = isa_6502_instr_stage_end;
            break;
            
    }
    if ( at_stage == isa_6502_instr_stage_end) {        
        membus_write_addr(opcode_context->memory, ADDR16, opcode_context->registers->Y);
    }
    return at_stage;
}

ISA_6502_STATIC_INSTR(STY)
{
    uint16_t    ADDR16;
    uint8_t     ADDR8;
    uint8_t     cycle_count;
    
    switch ( opcode_context->addressing_mode ) {
        case isa_6502_addressing_zeropage:
            ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            cycle_count = 2;
            break;
        case isa_6502_addressing_zeropage_x_indexed:
            ADDR8 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) + opcode_context->registers->X;
            ADDR16 = ADDR8;
            cycle_count = 3;
            break;
        case isa_6502_addressing_absolute:
            ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
            cycle_count = 3;
            break;
    }
    opcode_context->cycle_count += cycle_count;
    membus_write_addr(opcode_context->memory, ADDR16, opcode_context->registers->Y);
    return at_stage;
}

ISA_6502_DISASM(STY)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t                     operand1, operand2;
    const char                  *out_fmt = NULL;
    
    switch ( opcode_context->addressing_mode ) {
    
        case isa_6502_addressing_zeropage:
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "STY $%1$02hhX {<= Y=$%4$02hhX}";
            break;
        case isa_6502_addressing_zeropage_x_indexed:
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "STY $%1$02hhX,X[$%3$02hhX] {<= Y=$%4$02hhX}";
            break;
        case isa_6502_addressing_absolute:
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "STY $%1$02hhX%2$02hhX {<= Y=$%4$02hhX}";
            break;
    }
    return snprintf(buffer, buffer_len, out_fmt, operand1, operand2, opcode_context->registers->X, opcode_context->registers->Y);
#else
    return 0;
#endif
}
