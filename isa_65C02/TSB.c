
ISA_65C02_INSTR(TSB)
{
    static uint16_t ADDR16;
    static uint8_t  ADDR8;
    static uint8_t  OPERAND;
    bool            is_penultimate = false;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage:
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
                    OPERAND = membus_read_addr(opcode_context->memory, ADDR8);
                    break;
                case isa_6502_addressing_absolute:
                    ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
                    break;
            }
            break;
        
        case 3:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage:
                    ADDR16 = ADDR8;
                    is_penultimate = true;
                    break;
                case isa_6502_addressing_absolute:
                    OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
                    break;
            }
            break;
        case 4:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage:
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_absolute:
                    is_penultimate = true;
                    break;
            }
        case 5:
            at_stage = isa_6502_instr_stage_end;
            break;            
    }
    if ( is_penultimate ) {
        uint8_t     A_and_M = (opcode_context->registers->A & OPERAND);
        
        opcode_context->registers->SR &= ~register_SR_Bit_Z;
        opcode_context->registers->SR |= (A_and_M ? 0 : register_SR_Bit_Z);
    }
    if ( at_stage == isa_6502_instr_stage_end ) {
        membus_write_addr(opcode_context->memory, ADDR16, OPERAND | opcode_context->registers->A);
    }
    return at_stage;
}

ISA_65C02_STATIC_INSTR(TSB)
{
    uint16_t    ADDR16;
    uint8_t     ADDR8;
    uint8_t     OPERAND, A_and_M;
    uint8_t     cycle_count;
    
    switch ( opcode_context->addressing_mode ) {
        case isa_6502_addressing_zeropage:
            ADDR8 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            OPERAND = membus_read_addr(opcode_context->memory, ADDR8);
            ADDR16 = ADDR8;
            cycle_count = 4;
            break;
        case isa_6502_addressing_absolute:
            ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
            OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
            cycle_count = 5;
            break;
    }
    opcode_context->cycle_count += cycle_count;
    A_and_M = (opcode_context->registers->A & OPERAND);
    opcode_context->registers->SR &= ~register_SR_Bit_Z;
    opcode_context->registers->SR |= (A_and_M ? 0 : register_SR_Bit_Z);
    membus_write_addr(opcode_context->memory, ADDR16, OPERAND | opcode_context->registers->A);
    return at_stage;
}

ISA_65C02_DISASM(TSB)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t                     value, operand1, operand2, result;
    const char                  *out_fmt = NULL;
    
    switch ( opcode_context->addressing_mode ) {
    
        case isa_6502_addressing_zeropage:
            result = membus_wcache_pop(opcode_context->memory);     /* Result */
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            return snprintf(buffer, buffer_len, "TSB $%02hhX {$%02hhX OR $%02hhX => $%02hhX ------%hhu-}",
                        operand1, opcode_context->registers->A, value, result,
                        registers_SR_get_bit(opcode_context->registers, register_SR_Bit_Z));
            break;
        case isa_6502_addressing_absolute:
            result = membus_wcache_pop(opcode_context->memory);     /* Result */
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            return snprintf(buffer, buffer_len, "TSB $%02hhX%02hhX {$%02hhX OR $%02hhX => $%02hhX ------%hhu-}",
                        operand1, operand2, opcode_context->registers->A, value, result,
                        registers_SR_get_bit(opcode_context->registers, register_SR_Bit_Z));
            break;
    }
    return 0;
#else
    return 0;
#endif
}
