
ISA_65C02_INSTR(INC)
{
    static uint16_t ADDR16;
    static uint8_t  ADDR8;
    static uint8_t  OPERAND;
    
    at_stage = isa_6502_instr_stage_next_cycle;
                    
    switch ( opcode_context->cycle_count ) {
        case 1:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_accumulator:
                    opcode_context->registers->A++;
                    registers_did_set_A(opcode_context->registers, registers_Carry_ignore);
                    return isa_6502_instr_stage_end;
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
                    OPERAND = membus_read_addr(opcode_context->memory, ADDR8);
                    ADDR16 = ADDR8;
                    break;
                case isa_6502_addressing_zeropage_x_indexed:
                    ADDR8 += opcode_context->registers->X;
                    break;
                case isa_6502_addressing_absolute:
                case isa_6502_addressing_absolute_x_indexed:
                    ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
                    break;
            }
            break;
        case 3:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage:
                    OPERAND++;
                    break;
                case isa_6502_addressing_zeropage_x_indexed:
                    OPERAND = membus_read_addr(opcode_context->memory, ADDR8);
                    ADDR16 = ADDR8;
                    break;
                case isa_6502_addressing_absolute:
                    OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
                    break;
                case isa_6502_addressing_absolute_x_indexed:
                    ADDR16 += opcode_context->registers->X;
                    break;
            }
            break;
        case 4:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage:
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_zeropage_x_indexed:
                    OPERAND++;
                    break;
                case isa_6502_addressing_absolute:
                    OPERAND++;
                    break;
                case isa_6502_addressing_absolute_x_indexed:
                    OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
                    break;
            }
            break;
        
        case 5:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage_x_indexed:
                case isa_6502_addressing_absolute:
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_absolute_x_indexed:
                    OPERAND++;
                    break;
            }
            break;
        
        case 6:
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    if ( at_stage == isa_6502_instr_stage_end) {
        membus_write_addr(opcode_context->memory, ADDR16, OPERAND);
        registers_status_with_value(opcode_context->registers, OPERAND, registers_Carry_ignore);
    }
    return at_stage;
}

ISA_65C02_STATIC_INSTR(INC)
{
    uint16_t    ADDR16;
    uint8_t     ADDR8;
    uint8_t     OPERAND;
    uint8_t     cycle_count;
    
    switch ( opcode_context->addressing_mode ) {
        case isa_6502_addressing_accumulator:
            opcode_context->registers->A++;
            registers_did_set_A(opcode_context->registers, registers_Carry_ignore);
            opcode_context->cycle_count++;
            return isa_6502_instr_stage_end;
        case isa_6502_addressing_zeropage:
            ADDR8 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            OPERAND = membus_read_addr(opcode_context->memory, ADDR8);
            ADDR16 = ADDR8;
            cycle_count = 4;
            break;
        case isa_6502_addressing_zeropage_x_indexed:
            ADDR8 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) + opcode_context->registers->X;
            OPERAND = membus_read_addr(opcode_context->memory, ADDR8);
            ADDR16 = ADDR8;
            cycle_count = 5;
            break;
        case isa_6502_addressing_absolute:
            ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
            OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
            cycle_count = 5;
            break;
        case isa_6502_addressing_absolute_x_indexed:
            ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
            ADDR16 += opcode_context->registers->X;
            OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
            cycle_count = 6;
            break;
    }
    opcode_context->cycle_count += cycle_count;
    membus_write_addr(opcode_context->memory, ADDR16, ++OPERAND);
    registers_status_with_value(opcode_context->registers, OPERAND, registers_Carry_ignore);
    return isa_6502_instr_stage_end;
}

ISA_65C02_DISASM(INC)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t                     value1, value2, operand1, operand2;
    const char                  *out_fmt = NULL;
    
    switch ( opcode_context->addressing_mode ) {
    
        case isa_6502_addressing_accumulator:
            out_fmt = "INC A {A + 1 = $%3$02hhX}";
            break;
        case isa_6502_addressing_zeropage:
            value1 = membus_rcache_pop(opcode_context->memory);     /* Value in */
            value2 = membus_wcache_pop(opcode_context->memory);     /* Value out */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "INC $%1$02hhX {$%5$02hhX + 1 = $%6$02hhX}";
            break;
        case isa_6502_addressing_zeropage_x_indexed:
            value1 = membus_rcache_pop(opcode_context->memory);     /* Value in */
            value2 = membus_wcache_pop(opcode_context->memory);     /* Value out */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "INC $%1$02hhX,X[$%4$02hhX] {$%5$02hhX + 1 = $%6$02hhX}";
            break;
        case isa_6502_addressing_absolute:
            value1 = membus_rcache_pop(opcode_context->memory);     /* Value in */
            value2 = membus_wcache_pop(opcode_context->memory);     /* Value out */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "INC $%1$02hhX%2$02hhX {$%5$02hhX + 1 = $%6$02hhX}";
            break;
        case isa_6502_addressing_absolute_x_indexed:
            value1 = membus_rcache_pop(opcode_context->memory);     /* Value in */
            value2 = membus_wcache_pop(opcode_context->memory);     /* Value out */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "INC $%1$02hhX%2$02hhX,X[$%4$02hhX] {$%5$02hhX + 1 = $%6$02hhX}";
            break;
    }
    return snprintf(buffer, buffer_len, out_fmt, operand1, operand2, opcode_context->registers->A, opcode_context->registers->X,  value1, value2);
#else
    return 0;
#endif
}
