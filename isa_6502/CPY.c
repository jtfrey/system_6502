
ISA_6502_INSTR(CPY)
{
    static uint16_t ADDR16;
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
        uint8_t     flag_bits = 0x00;
        
        if ( opcode_context->registers->Y > OPERAND )
            flag_bits |= register_SR_Bit_C;
        else if ( opcode_context->registers->Y == OPERAND )
            flag_bits |= register_SR_Bit_C | register_SR_Bit_Z;
        else
            flag_bits |= register_SR_Bit_N;
        opcode_context->registers->SR = (opcode_context->registers->SR & ~(register_SR_Bit_C | register_SR_Bit_Z | register_SR_Bit_N)) | flag_bits;
    }
    return at_stage;
}

ISA_6502_STATIC_INSTR(CPY)
{
    uint16_t    OPERAND;
    uint8_t     cycle_count, flag_bits = 0x00;
    
    switch ( opcode_context->addressing_mode ) {
        case isa_6502_addressing_immediate:
            OPERAND = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            cycle_count = 1;
            break;
        case isa_6502_addressing_zeropage: {
            uint8_t     ADDR8 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            OPERAND = membus_read_addr(opcode_context->memory, ADDR8);
            cycle_count = 2;
            break;
        }
        case isa_6502_addressing_absolute: {
            uint16_t    ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
            OPERAND = membus_read_addr(opcode_context->memory, ADDR16);
            cycle_count = 3;
            break;
        }
    }
    opcode_context->cycle_count += cycle_count;
    if ( opcode_context->registers->Y > OPERAND )
        flag_bits |= register_SR_Bit_C;
    else if ( opcode_context->registers->Y == OPERAND )
        flag_bits |= register_SR_Bit_C | register_SR_Bit_Z;
    else
        flag_bits |= register_SR_Bit_N;
    opcode_context->registers->SR = (opcode_context->registers->SR & ~(register_SR_Bit_C | register_SR_Bit_Z | register_SR_Bit_N)) | flag_bits;
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(CPY)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t                     value, operand1, operand2;
    const char                  *out_fmt = NULL;
    
    switch ( opcode_context->addressing_mode ) {
    
        case isa_6502_addressing_immediate:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            out_fmt = "CPY #$%4$02hhX {$%4$02hhX == $%3$02hhX}";
            break;
        case isa_6502_addressing_zeropage:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "CPY $%1$02hhX {$%4$02hhX == $%3$02hhX}";
            break;
        case isa_6502_addressing_absolute:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "CPY $%1$02hhX%2$02hhX {$%4$02hhX == $%3$02hhX}";
            break;
    }
    return snprintf(buffer, buffer_len, out_fmt, operand1, operand2, opcode_context->registers->Y, value);
#else
    return 0;
#endif
}
