
ISA_65C02_INSTR(ADC)
{
    static uint16_t ADDR, ADDR_pre_index;
    static uint8_t  *ADDR_ptr, *ALU_ptr;
    static uint16_t ALU, ALU_pre_index;
    static bool     IS_BCD, BCD_SHOULD_DELAY, BCD_SHOULD_EXIT;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            ADDR = 0x0000;
            IS_BCD = registers_SR_get_bit(opcode_context->registers, register_SR_Bit_D);
            BCD_SHOULD_DELAY = IS_BCD;
            BCD_SHOULD_EXIT = false;
            if ( opcode_context->addressing_mode == isa_6502_addressing_immediate ) {
                ALU = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
                at_stage = isa_6502_instr_stage_end;
            } else {
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
            }
            break;
        
        case 2:
            /* Catch the delay cycle for BCD: */
            if ( BCD_SHOULD_EXIT ) return isa_6502_instr_stage_end;
            
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage:
                    ALU = membus_read_addr(opcode_context->memory, ADDR);
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_zeropage_indirect:
                    /* Zero page address is complete, load the low word of target: */
                    ALU = membus_read_addr(opcode_context->memory, ADDR);
                    ADDR = (ADDR + 1) & 0x00FF;
                    break;
                case isa_6502_addressing_zeropage_x_indexed:
                    ADDR = (ADDR + opcode_context->registers->X) & 0x00FF;
                    break;
                case isa_6502_addressing_absolute:
                case isa_6502_addressing_absolute_x_indexed:
                case isa_6502_addressing_absolute_y_indexed:
                case isa_6502_addressing_x_indexed_indirect:
                case isa_6502_addressing_indirect_y_indexed:
                    *ADDR_ptr = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
                    break;
            }
            break;
        
        case 3:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage_indirect:
                    /* Zero page address is complete, load the high word of target: */
                    ALU |= membus_read_addr(opcode_context->memory, ADDR) << 8;
                    break;
                case isa_6502_addressing_zeropage_x_indexed:
                case isa_6502_addressing_absolute:
                    ALU = membus_read_addr(opcode_context->memory, ADDR);
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_absolute_x_indexed:
                    ADDR_pre_index = ADDR;
                    ADDR += opcode_context->registers->X;
                    if ( (ADDR_pre_index & 0xFF00) == (ADDR & 0xFF00) ) {
                        ALU = membus_read_addr(opcode_context->memory, ADDR);
                        at_stage = isa_6502_instr_stage_end;
                    }
                    break;
                case isa_6502_addressing_absolute_y_indexed:
                    ADDR_pre_index = ADDR;
                    ADDR += opcode_context->registers->Y;
                    if ( (ADDR_pre_index & 0xFF00) == (ADDR & 0xFF00) ) {
                        ALU = membus_read_addr(opcode_context->memory, ADDR);
                        at_stage = isa_6502_instr_stage_end;
                    }
                    break;
                case isa_6502_addressing_x_indexed_indirect:
                    ADDR += opcode_context->registers->X;
                case isa_6502_addressing_indirect_y_indexed:
#ifdef ISA_6502_HOST_IS_LE
                    ALU_ptr = ((uint8_t*)&ALU);
#else
                    ALU_ptr = ((uint8_t*)&ALU) + 1;
#endif
                    *ALU_ptr = membus_read_addr(opcode_context->memory, ADDR++);
#ifdef ISA_6502_HOST_IS_LE
                    ALU_ptr++;
#else
                    ALU_ptr--;
#endif
                    break;
            }
            break;
        
        case 4:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage_indirect:
                    /* Target address is in ALU now, read byte: */
                    ALU = membus_read_addr(opcode_context->memory, ALU);
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_absolute_x_indexed:
                case isa_6502_addressing_absolute_y_indexed:
                    ALU = membus_read_addr(opcode_context->memory, ADDR);
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_x_indexed_indirect:
                    *ALU_ptr = membus_read_addr(opcode_context->memory, ADDR);
                    break;
                case isa_6502_addressing_indirect_y_indexed:
                    *ALU_ptr = membus_read_addr(opcode_context->memory, ADDR);
                    ALU_pre_index = ALU;
                    ALU += opcode_context->registers->Y;
                    if ( (ALU_pre_index & 0xFF00) == (ALU & 0xFF00) ) {
                        ALU = membus_read_addr(opcode_context->memory, ALU);
                        at_stage = isa_6502_instr_stage_end;
                    }
                    break;
            }
            break;
        
        case 5:
            ALU = membus_read_addr(opcode_context->memory, ALU);
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    if ( at_stage == isa_6502_instr_stage_end) {
        uint16_t    SUM = ALU + (uint16_t)opcode_context->registers->A + registers_SR_get_bit(opcode_context->registers, register_SR_Bit_C);
        uint8_t     flag_bits = (SUM) ? 0 : register_SR_Bit_Z;
        
        if ( IS_BCD && ((SUM & 0x0010) || ((SUM & 0x000F) > 0x0009)) ) SUM += 0x0006;
    
        if ( SUM & 0xFF00 ) flag_bits |= register_SR_Bit_C;
        if ( ~((uint16_t)opcode_context->registers->A ^ ALU) & ((uint16_t)opcode_context->registers->A ^ SUM) & 0x80 ) flag_bits |= register_SR_Bit_V;
        opcode_context->registers->SR = (opcode_context->registers->SR & ~(register_SR_Bit_C | register_SR_Bit_Z | register_SR_Bit_V)) | flag_bits;
        
        if ( IS_BCD && ((SUM & 0x0100) || ((SUM & 0x00FF) > 0x009F)) ) SUM += 0x0060;
        
        opcode_context->registers->A = SUM;
        
        if ( BCD_SHOULD_DELAY ) {
            at_stage = isa_6502_instr_stage_next_cycle;
            BCD_SHOULD_DELAY = false;
            BCD_SHOULD_EXIT = true;
        }
    }
    return at_stage;
}

ISA_65C02_DISASM(ADC)
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
        case isa_6502_addressing_zeropage_indirect:
            value = membus_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            operand3 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "ADC ($%3$02hhX[$%1$02hhX%2$02hhX]) {A + $%7$02hhX = $%4$02hhX}";
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