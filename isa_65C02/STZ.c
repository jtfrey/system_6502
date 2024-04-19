
ISA_65C02_INSTR(STZ)
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
                case isa_6502_addressing_absolute_x_indexed:
                    ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
                    break;
            }
            break;
        
        case 2:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage:
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_zeropage_x_indexed:
                    ADDR8 += opcode_context->registers->X;
                    ADDR16 = ADDR8;
                    break;
                case isa_6502_addressing_absolute:
                case isa_6502_addressing_absolute_x_indexed:
                    ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
                    break;
            }
            break;
        
        case 3:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage_x_indexed:
                    ADDR16 = ADDR8;
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_absolute:
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_absolute_x_indexed:
                    ADDR16 += opcode_context->registers->X;
                    break;
            }
            break;
        
        case 4:
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    if ( at_stage == isa_6502_instr_stage_end) {
        membus_write_addr(opcode_context->memory, ADDR16, 0x00);
    }
    return at_stage;
}


ISA_65C02_STATIC_INSTR(STZ)
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
            ADDR8 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            ADDR8 += opcode_context->registers->X;
            ADDR16 = ADDR8;
            cycle_count = 3;
            break;
        case isa_6502_addressing_absolute:
            ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
            cycle_count = 3;
            break;
        case isa_6502_addressing_absolute_x_indexed:
            ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
            ADDR16 += opcode_context->registers->X;
            cycle_count = 4;
            break;
    }
    opcode_context->cycle_count += cycle_count;
    membus_write_addr(opcode_context->memory, ADDR16, 0x00);
    return isa_6502_instr_stage_end;
}

ISA_65C02_DISASM(STZ)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t                     value, operand1, operand2, operand3;
    const char                  *out_fmt = NULL;
    
    switch ( opcode_context->addressing_mode ) {
    
        case isa_6502_addressing_zeropage:
            value = membus_wcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "STZ $%1$02hhX {<= $%6$02hhX}";
            break;
        case isa_6502_addressing_zeropage_x_indexed:
            value = membus_wcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "STZ $%1$02hhX,X[$%4$02hhX] {<= $%6$02hhX}";
            break;
        case isa_6502_addressing_absolute:
            value = membus_wcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "STZ $%1$02hhX%2$02hhX {<= $%6$02hhX}";
            break;
        case isa_6502_addressing_absolute_x_indexed:
            value = membus_wcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "STZ $%1$02hhX%2$02hhX,X[$%4$02hhX] {<= $%6$02hhX}";
            break;
    }
    return snprintf(buffer, buffer_len, out_fmt, operand1, operand2, operand3, opcode_context->registers->X, opcode_context->registers->Y, value);
#else
    return 0;
#endif
}
