
ISA_6502_INSTR(ADC)
{
    static uint16_t ADDR16, ADDR16_pre_index;
    static uint8_t  ADDR8;
    static uint16_t OPERAND;
    static bool     is_BCD;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            is_BCD = registers_SR_get_bit(opcode_context->registers, register_SR_Bit_D);
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
        uint16_t        ALU;
        uint8_t         flag_bits = 0x00;
        
        if ( is_BCD ) {
            ALU = (OPERAND & 0x000F) + (uint16_t)(opcode_context->registers->A & 0x0F) + registers_SR_get_bit(opcode_context->registers, register_SR_Bit_C);
            if ( ALU > 0x0009 ) ALU += 0x0006;
            ALU = (OPERAND & 0x00F0) + (uint16_t)(opcode_context->registers->A & 0xF0) + ((ALU > 0x000F) ? 0x0010 : 0x0000) + (ALU & 0x000F);
        } else {
            ALU = OPERAND + (uint16_t)opcode_context->registers->A + registers_SR_get_bit(opcode_context->registers, register_SR_Bit_C);
        }
        
        // Set overflow:
        if ( ~((uint16_t)opcode_context->registers->A ^ OPERAND) & ((uint16_t)opcode_context->registers->A ^ ALU) & 0x0080 )
            flag_bits |= register_SR_Bit_V;
        
        // Final BCD adjustment:
        if ( is_BCD && (ALU > 0x009F) ) ALU += 0x0060;
        
        // Zero?
        if ( ! (ALU & 0x00FF) ) flag_bits |= register_SR_Bit_Z;
        
        // Negative?
        if ( ALU & 0x0080 ) flag_bits |= register_SR_Bit_N;
        
        // Carry?
        if ( ALU & 0xFF00 ) flag_bits |= register_SR_Bit_C;
        
        // Set bits:
        opcode_context->registers->SR = (opcode_context->registers->SR & ~(register_SR_Bit_C | register_SR_Bit_Z | register_SR_Bit_V | register_SR_Bit_C | register_SR_Bit_Z | register_SR_Bit_N)) | flag_bits;
        
        // Set accumulator:
        opcode_context->registers->A = (ALU & 0x00FF);
    }
    return at_stage;
}

ISA_6502_STATIC_INSTR(ADC)
{
    uint16_t        ADDR16, ADDR16_pre_index;
    uint8_t         ADDR8;
    uint16_t        OPERAND, ALU;
    uint8_t         flag_bits;
    int             cycle_count = 0;
    bool            is_BCD = registers_SR_get_bit(opcode_context->registers, register_SR_Bit_D);
    
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
    
    if ( is_BCD ) {
        ALU = (OPERAND & 0x000F) + (uint16_t)(opcode_context->registers->A & 0x0F) + registers_SR_get_bit(opcode_context->registers, register_SR_Bit_C);
        if ( ALU > 0x0009 ) ALU += 0x0006;
        ALU = (OPERAND & 0x00F0) + (uint16_t)(opcode_context->registers->A & 0xF0) + ((ALU > 0x000F) ? 0x0010 : 0x0000) + (ALU & 0x000F);
    } else {
        ALU = OPERAND + (uint16_t)opcode_context->registers->A + registers_SR_get_bit(opcode_context->registers, register_SR_Bit_C);
    }
    
    // Set overflow:
    if ( ~((uint16_t)opcode_context->registers->A ^ OPERAND) & ((uint16_t)opcode_context->registers->A ^ ALU) & 0x0080 )
        flag_bits |= register_SR_Bit_V;
    
    // Final BCD adjustment:
    if ( is_BCD && (ALU > 0x009F) ) ALU += 0x0060;
    
    // Zero?
    if ( ! (ALU & 0x00FF) ) flag_bits |= register_SR_Bit_Z;
    
    // Negative?
    if ( ALU & 0x0080 ) flag_bits |= register_SR_Bit_N;
    
    // Carry?
    if ( ALU & 0xFF00 ) flag_bits |= register_SR_Bit_C;
    
    // Set bits:
    opcode_context->registers->SR = (opcode_context->registers->SR & ~(register_SR_Bit_C | register_SR_Bit_Z | register_SR_Bit_V | register_SR_Bit_C | register_SR_Bit_Z | register_SR_Bit_N)) | flag_bits;
    
    // Set accumulator:
    opcode_context->registers->A = (ALU & 0x00FF);
    
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(ADC)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t                     value, operand1, operand2, operand3;
    const char                  *out_fmt = NULL;
    
    switch ( opcode_context->addressing_mode ) {
    
        case isa_6502_addressing_immediate:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            out_fmt = "ADC #$%7$02hhX {A + $%7$02hhX = $%4$02hhX}";
            break;
        case isa_6502_addressing_zeropage:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "ADC $%1$02hhX {A + $%7$02hhX = $%4$02hhX}";
            break;
        case isa_6502_addressing_zeropage_x_indexed:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "ADC $%1$02hhX,X {A + $%7$02hhX = $%4$02hhX}";
            break;
        case isa_6502_addressing_absolute:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "ADC $%1$02hhX%2$02hhX {A + $%7$02hhX = $%4$02hhX}";
            break;
        case isa_6502_addressing_absolute_x_indexed:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "ADC $%1$02hhX%2$02hhX,X[$%5$02hhX] {A + $%7$02hhX = $%4$02hhX}";
            break;
        case isa_6502_addressing_absolute_y_indexed:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "ADC $%1$02hhX%2$02hhX,Y[$%6$02hhX] {A + $%7$02hhX = $%4$02hhX}";
            break;
        case isa_6502_addressing_x_indexed_indirect:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            operand3 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "ADC ($%3$02hhX=$%1$02hhX%2$02hhX,X[$%5$02hhX]) {A + $%7$02hhX = $%4$02hhX}";
            break;
        case isa_6502_addressing_indirect_y_indexed:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            operand3 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "ADC ($%3$02hhX[$%1$02hhX%2$02hhX]),Y[$%6$02hhX] {A + $%7$02hhX = $%4$02hhX}";
            break;
    }
    return snprintf(buffer, buffer_len, out_fmt, operand1, operand2, operand3, opcode_context->registers->A, opcode_context->registers->X,  opcode_context->registers->Y, value);
#else
    return 0;
#endif
}
