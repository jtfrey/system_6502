
ISA_65C02_INSTR(TSB)
{
    static uint16_t ADDR = 0x0000;
    static uint8_t  *ADDR_ptr;
    static uint8_t  ALU;
    bool            is_penultimate = false;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            ADDR = 0x0000;
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr = ((uint8_t*)&ADDR);
#else
            ADDR_ptr = ((uint8_t*)&ADDR) + 1;
#endif
            *ADDR_ptr = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr++;
#else
            ADDR_ptr--;
#endif
            break;
        
        case 2:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage:
                    ALU = membus_read_addr(opcode_context->memory, ADDR);
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_absolute:
                    *ADDR_ptr = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
                    break;
            }
            break;
        
        case 3:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage:
                    is_penultimate = true;
                    break;
                case isa_6502_addressing_absolute:
                    ALU = membus_read_addr(opcode_context->memory, ADDR);
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
        uint8_t     A_and_M = (opcode_context->registers->A & ALU);
        
        opcode_context->registers->SR &= ~register_SR_Bit_Z;
        opcode_context->registers->SR |= (A_and_M ? 0 : register_SR_Bit_Z);
    }
    if ( at_stage == isa_6502_instr_stage_end ) {
        membus_write_addr(opcode_context->memory, ADDR, ALU | opcode_context->registers->A);
    }
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
