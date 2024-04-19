
ISA_65C02_INSTR(ROR)
{
    static uint16_t ADDR16, ADDR16_pre_index;
    static uint8_t  ADDR8;
    static uint16_t OPERAND;
    static bool     is_penultimate, is_OPERAND_set;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            is_penultimate = false;
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_accumulator: {
                    OPERAND = ((uint16_t)opcode_context->registers->A << 1) | registers_SR_get_bit(opcode_context->registers, register_SR_Bit_C);
                    opcode_context->registers->A = OPERAND & 0x00FF;
                    registers_did_set_A(
                            opcode_context->registers,
                            ((OPERAND & 0xFF00) != 0) ? registers_Carry_set : registers_Carry_clear
                        );
                    return isa_6502_instr_stage_end;
                }
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
                    is_penultimate = true;
                    break;
                case isa_6502_addressing_zeropage_x_indexed:
                    OPERAND = membus_read_addr(opcode_context->memory, ADDR8);
                    break;
                case isa_6502_addressing_absolute:
                    OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
                    break;
                case isa_6502_addressing_absolute_x_indexed:
                    ADDR16_pre_index = ADDR16;
                    ADDR16 += opcode_context->registers->X;
                    break;
            }
            break;
        
        case 4:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage:
                    ADDR16 = ADDR8;
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_zeropage_x_indexed:
                case isa_6502_addressing_absolute:
                    is_penultimate = true;
                    break;
                case isa_6502_addressing_absolute_x_indexed:
                    if ( (ADDR16_pre_index & 0xFF00) == (ADDR16 & 0xFF00) ) {
                        OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
                        is_penultimate = true;
                    }
                    break;
            }
            break;
        
        case 5:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage_x_indexed:
                    ADDR16 = ADDR8;
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_absolute:
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_absolute_x_indexed:
                    if ( is_penultimate ) {
                        at_stage = isa_6502_instr_stage_end;
                    } else {
                        OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
                        is_penultimate = true;
                    }
                    break;
            }
            break;
            
        case 6:
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    if ( is_penultimate ) {
        bool        is_carry = (OPERAND & 0x01);
        
        OPERAND = ((OPERAND >> 1) | (registers_SR_get_bit(opcode_context->registers, register_SR_Bit_C) << 7)) & 0x00FF;
        registers_status_with_value(
                opcode_context->registers,
                OPERAND,
                is_carry ? registers_Carry_set : registers_Carry_clear
            );
    }
    else if ( at_stage == isa_6502_instr_stage_end) {
        membus_write_addr(opcode_context->memory, ADDR16, OPERAND);
    }
    return at_stage;
}

ISA_65C02_STATIC_INSTR(ROR)
{
    uint16_t    ADDR16, ADDR16_pre_index;
    uint8_t     ADDR8;
    uint16_t    OPERAND;
    uint8_t     cycle_count;
    bool        is_carry;
    
    switch ( opcode_context->addressing_mode ) {
        case isa_6502_addressing_accumulator: {
            bool    is_carry = (opcode_context->registers->A & 0x01);
        
            opcode_context->registers->A = (opcode_context->registers->A >> 1) | (registers_SR_get_bit(opcode_context->registers, register_SR_Bit_C) << 7);
            registers_did_set_A(
                    opcode_context->registers,
                    is_carry ? registers_Carry_set : registers_Carry_clear
                );
            opcode_context->cycle_count += 1;
            return isa_6502_instr_stage_end;
        }
        
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
            ADDR16_pre_index = ADDR16;
            ADDR16 += opcode_context->registers->X;
            OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
            cycle_count = 5 + ((ADDR16_pre_index & 0xFF00) != (ADDR16 & 0xFF00));
            break;
    }
    opcode_context->cycle_count += cycle_count;
    is_carry = (OPERAND & 0x01);
    OPERAND = ((OPERAND >> 1) | (registers_SR_get_bit(opcode_context->registers, register_SR_Bit_C) << 7)) & 0x00FF;
    registers_status_with_value(
            opcode_context->registers,
            OPERAND,
            is_carry ? registers_Carry_set : registers_Carry_clear
        );
    membus_write_addr(opcode_context->memory, ADDR16, OPERAND);
    return isa_6502_instr_stage_end;
}

ISA_65C02_DISASM(ROR)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t                     value1, value2, operand1, operand2;
    const char                  *out_fmt = NULL;
    
    switch ( opcode_context->addressing_mode ) {
    
        case isa_6502_addressing_accumulator:
            out_fmt = "ROR A {(A >> 1) | (CARRY << 7) = $%3$02hhX}";
            break;
        case isa_6502_addressing_zeropage:
            value1 = membus_rcache_pop(opcode_context->memory);     /* Value in */
            value2 = membus_wcache_pop(opcode_context->memory);     /* Value out */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "ROR $%1$02hhX {($%5$02hhX >> 1) | (CARRY << 7) = $%6$02hhX}";
            break;
        case isa_6502_addressing_zeropage_x_indexed:
            value1 = membus_rcache_pop(opcode_context->memory);     /* Value in */
            value2 = membus_wcache_pop(opcode_context->memory);     /* Value out */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "ROR $%1$02hhX,X[$%4$02hhX] {($%5$02hhX >> 1) | (CARRY << 7) = $%6$02hhX}";
            break;
        case isa_6502_addressing_absolute:
            value1 = membus_rcache_pop(opcode_context->memory);     /* Value in */
            value2 = membus_wcache_pop(opcode_context->memory);     /* Value out */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "ROR $%1$02hhX%2$02hhX {($%5$02hhX >> 1) | (CARRY << 7) = $%6$02hhX}";
            break;
        case isa_6502_addressing_absolute_x_indexed:
            value1 = membus_rcache_pop(opcode_context->memory);     /* Value in */
            value2 = membus_wcache_pop(opcode_context->memory);     /* Value out */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "ROR $%1$02hhX%2$02hhX,X[$%4$02hhX] {($%5$02hhX >> 1) | (CARRY << 7) = $%6$02hhX}";
            break;
    }
    return snprintf(buffer, buffer_len, out_fmt, operand1, operand2, opcode_context->registers->A, opcode_context->registers->X,  value1, value2);
#else
    return 0;
#endif
}
