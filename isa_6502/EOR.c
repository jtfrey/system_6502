
ISA_6502_INSTR(EOR)
{
    static uint16_t ADDR16, ADDR16_pre_index;
    static uint8_t  ADDR8;
    static uint16_t OPERAND;

    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_immediate:
                    OPERAND = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_zeropage:
                case isa_6502_addressing_zeropage_x_indexed:
                case isa_6502_addressing_x_indexed_indirect:
                case isa_6502_addressing_indirect_y_indexed:
                    ADDR8 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
                    break;
                case isa_6502_addressing_absolute:
                case isa_6502_addressing_absolute_x_indexed:
                case isa_6502_addressing_absolute_y_indexed:
                    ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
                    break;
            }
            break;
        
        case 2:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage:
                    OPERAND = membus_read_addr(opcode_context->memory, ADDR8);
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_zeropage_x_indexed:
                    ADDR8 += opcode_context->registers->X;
                    break;
                case isa_6502_addressing_absolute:
                case isa_6502_addressing_absolute_x_indexed:
                case isa_6502_addressing_absolute_y_indexed:
                    ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
                    break;
                case isa_6502_addressing_x_indexed_indirect:
                    ADDR8 += opcode_context->registers->X;
                    break;
                case isa_6502_addressing_indirect_y_indexed:
                    ADDR16 = membus_read_addr(opcode_context->memory, ADDR8++);
                    break;
            }
            break;
        
        case 3:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage_x_indexed:
                    OPERAND = membus_read_addr(opcode_context->memory, ADDR8);
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_absolute:
                    OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_absolute_x_indexed:
                    ADDR16_pre_index = ADDR16;
                    ADDR16 += opcode_context->registers->X;
                    if ( (ADDR16_pre_index & 0xFF00) == (ADDR16 & 0xFF00) ) {
                        OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
                        at_stage = isa_6502_instr_stage_end;
                    }
                    break;
                case isa_6502_addressing_absolute_y_indexed:
                    ADDR16_pre_index = ADDR16;
                    ADDR16 += opcode_context->registers->Y;
                    if ( (ADDR16_pre_index & 0xFF00) == (ADDR16 & 0xFF00) ) {
                        OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
                        at_stage = isa_6502_instr_stage_end;
                    }
                    break;
                case isa_6502_addressing_x_indexed_indirect:
                    ADDR16 = membus_read_addr(opcode_context->memory, ADDR8++);
                    break;
                case isa_6502_addressing_indirect_y_indexed:
                    ADDR16 |= membus_read_addr(opcode_context->memory, ADDR8) << 8;
                    break;
            }
            break;
        
        case 4:
            switch ( opcode_context->addressing_mode ) {\
                case isa_6502_addressing_absolute_x_indexed:
                case isa_6502_addressing_absolute_y_indexed:
                    OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_x_indexed_indirect:
                    ADDR16 |= membus_read_addr(opcode_context->memory, ADDR8) << 8;
                    break;
                case isa_6502_addressing_indirect_y_indexed:
                    ADDR16_pre_index = ADDR16;
                    ADDR16 += opcode_context->registers->Y;
                    if ( (ADDR16_pre_index & 0xFF00) == (ADDR16 & 0xFF00) ) {
                        OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
                        at_stage = isa_6502_instr_stage_end;
                    }
                    break;
            }
            break;
        
        case 5:
            OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    if ( at_stage == isa_6502_instr_stage_end) {
        opcode_context->registers->A ^= (uint8_t)OPERAND;
        registers_did_set_A(opcode_context->registers, registers_Carry_ignore);
    }
    return at_stage;
}

ISA_6502_STATIC_INSTR(EOR)
{
    uint16_t        ADDR16, ADDR16_pre_index;
    uint8_t         ADDR8;
    uint16_t        OPERAND, ALU;
    uint8_t         flag_bits;
    int             cycle_count = 0;
    
    switch ( opcode_context->addressing_mode ) {
        
        case isa_6502_addressing_immediate:
            OPERAND = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            cycle_count = 1;
            break;
        
        case isa_6502_addressing_zeropage:
            ADDR8 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            OPERAND = membus_read_addr(opcode_context->memory, ADDR8);
            cycle_count = 2;
            break;
        
        case isa_6502_addressing_zeropage_x_indexed:
            ADDR8 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) + opcode_context->registers->X;
            OPERAND = membus_read_addr(opcode_context->memory, ADDR8);
            cycle_count = 3;
            break;
            
        case isa_6502_addressing_absolute:
            ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
            OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
            cycle_count = 3;
            break;
        
        case isa_6502_addressing_absolute_x_indexed:
            ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
            ADDR16_pre_index = ADDR16;
            ADDR16 += opcode_context->registers->X;
            OPERAND = membus_read_addr(opcode_context->memory, ADDR8);
            cycle_count = 3 + ((ADDR16_pre_index & 0xFF00) != (ADDR16 & 0xFF00));
            break;
        
        case isa_6502_addressing_absolute_y_indexed:
            ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
            ADDR16_pre_index = ADDR16;
            ADDR16 += opcode_context->registers->Y;
            OPERAND = membus_read_addr(opcode_context->memory, ADDR8);
            cycle_count = 3 + ((ADDR16_pre_index & 0xFF00) != (ADDR16 & 0xFF00));
            break;
        
        case isa_6502_addressing_x_indexed_indirect:
            ADDR8 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) + opcode_context->registers->X;
            ADDR16 = membus_read_addr(opcode_context->memory, ADDR8++);
            ADDR16 |= membus_read_addr(opcode_context->memory, ADDR8) << 8;
            OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
            cycle_count = 5;
            break;
        
        case isa_6502_addressing_indirect_y_indexed:
            ADDR8 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            ADDR16 = membus_read_addr(opcode_context->memory, ADDR8++);
            ADDR16 |= membus_read_addr(opcode_context->memory, ADDR8) << 8;
            ADDR16_pre_index = ADDR16;
            ADDR16 += opcode_context->registers->Y;
            OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
            cycle_count = 4 + ((ADDR16_pre_index & 0xFF00) != (ADDR16 & 0xFF00));
            break;
        
    }
    opcode_context->cycle_count += cycle_count;
    opcode_context->registers->A ^= (uint8_t)OPERAND;
    registers_did_set_A(opcode_context->registers, registers_Carry_ignore);
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(EOR)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t                     value, operand1, operand2, operand3;
    const char                  *out_fmt = NULL;
    
    switch ( opcode_context->addressing_mode ) {
    
        case isa_6502_addressing_immediate:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            out_fmt = "EOR #$%7$02hhX {A ^ $%7$02hhX = $%4$02hhX}";
            break;
        case isa_6502_addressing_zeropage:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "EOR $%1$02hhX {A ^ $%7$02hhX = $%4$02hhX}";
            break;
        case isa_6502_addressing_zeropage_x_indexed:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "EOR $%1$02hhX,X {A ^ $%7$02hhX = $%4$02hhX}";
            break;
        case isa_6502_addressing_absolute:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "EOR $%1$02hhX%2$02hhX {A ^ $%7$02hhX = $%4$02hhX}";
            break;
        case isa_6502_addressing_absolute_x_indexed:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "EOR $%1$02hhX%2$02hhX,X[$%5$02hhX] {A ^ $%7$02hhX = $%4$02hhX}";
            break;
        case isa_6502_addressing_absolute_y_indexed:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "EOR $%1$02hhX%2$02hhX,Y[$%6$02hhX] {A ^ $%7$02hhX = $%4$02hhX}";
            break;
        case isa_6502_addressing_x_indexed_indirect:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            operand3 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "EOR ($%3$02hhX=$%1$02hhX%2$02hhX,X[$%5$02hhX]) {A ^ $%7$02hhX = $%4$02hhX}";
            break;
        case isa_6502_addressing_indirect_y_indexed:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            operand3 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "EOR ($%3$02hhX[$%1$02hhX%3$02hhX]),Y[$%6$02hhX] {A ^ $%7$02hhX = $%4$02hhX}";
            break;
    }
    return snprintf(buffer, buffer_len, out_fmt, operand1, operand2, operand3, opcode_context->registers->A, opcode_context->registers->X,  opcode_context->registers->Y, value);
#else
    return 0;
#endif
}
