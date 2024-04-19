
ISA_6502_INSTR(BIT)
{
    static uint16_t ADDR16;
    static uint8_t  ADDR8;
    static uint8_t  OPERAND;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
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
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_absolute:
                    ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
                    break;
            }
            break;
        case 3:
            OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    if ( at_stage == isa_6502_instr_stage_end) {
        uint8_t     A_AND_M = (opcode_context->registers->A & OPERAND);
        
        opcode_context->registers->SR &= ~(register_SR_Bit_V | register_SR_Bit_Z | register_SR_Bit_N);
        opcode_context->registers->SR |= (OPERAND & 0xC0) | (A_AND_M ? 0 : register_SR_Bit_Z);
    }
    return at_stage;
}

ISA_6502_STATIC_INSTR(BIT)
{
    uint16_t    ADDR16;
    uint8_t     ADDR8;
    uint8_t     OPERAND, A_AND_M;
    uint8_t     cycle_count;
    
    switch ( opcode_context->addressing_mode ) {
        case isa_6502_addressing_zeropage:
            ADDR8 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            OPERAND = membus_read_addr(opcode_context->memory, ADDR8);
            cycle_count = 2;
            break;
        case isa_6502_addressing_absolute:
            ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
            OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
            cycle_count = 3;
            break;
    }
    opcode_context->cycle_count += cycle_count;
    A_AND_M = (opcode_context->registers->A & OPERAND);
    opcode_context->registers->SR &= ~(register_SR_Bit_V | register_SR_Bit_Z | register_SR_Bit_N);
    opcode_context->registers->SR |= (OPERAND & 0xC0) | (A_AND_M ? 0 : register_SR_Bit_Z);
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(BIT)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t                     value, operand1, operand2;
    const char                  *out_fmt = NULL;
    
    switch ( opcode_context->addressing_mode ) {
    
        case isa_6502_addressing_zeropage:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            return snprintf(buffer, buffer_len, "BIT $%02hhX {$%02hhX TEST $%02hhX => %hhu%hhu----%hhu-}",
                        operand1, opcode_context->registers->A, value,
                        registers_SR_get_bit(opcode_context->registers, register_SR_Bit_N),
                        registers_SR_get_bit(opcode_context->registers, register_SR_Bit_V),
                        registers_SR_get_bit(opcode_context->registers, register_SR_Bit_Z));
            break;
        case isa_6502_addressing_absolute:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            return snprintf(buffer, buffer_len, "BIT $%02hhX%02hhX {$%02hhX TEST $%02hhX => %hhu%hhu----%hhu-}",
                        operand1, operand2, opcode_context->registers->A, value,
                        registers_SR_get_bit(opcode_context->registers, register_SR_Bit_N),
                        registers_SR_get_bit(opcode_context->registers, register_SR_Bit_V),
                        registers_SR_get_bit(opcode_context->registers, register_SR_Bit_Z));
            break;
    }
    return 0;
#else
    return 0;
#endif
}
