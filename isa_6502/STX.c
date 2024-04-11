
ISA_6502_INSTR(STX)
{
    static uint16_t ADDR = 0x0000;
    static uint8_t  *ADDR_ptr;
    
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
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_zeropage_y_indexed:
                    ADDR = (ADDR + opcode_context->registers->Y) & 0x00FF;
                    break;
                case isa_6502_addressing_absolute:
                    *ADDR_ptr = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
                    break;
            }
            break;
        
        case 3:
            at_stage = isa_6502_instr_stage_end;
            break;
            
    }
    if ( at_stage == isa_6502_instr_stage_end) {        
        membus_write_addr(opcode_context->memory, ADDR, opcode_context->registers->X);
    }
    return at_stage;
}

ISA_6502_DISASM(STX)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t                     operand1, operand2;
    const char                  *out_fmt = NULL;
    
    switch ( opcode_context->addressing_mode ) {
    
        case isa_6502_addressing_zeropage:
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "STX $%1$02hhX {<= X=$%4$02hhX}";
            break;
        case isa_6502_addressing_zeropage_y_indexed:
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "STX $%1$02hhX,Y[$%3$02hhX] {<= X=$%4$02hhX}";
            break;
        case isa_6502_addressing_absolute:
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "STX $%1$02hhX%2$02hhX {<= X=$%4$02hhX}";
            break;
    }
    return snprintf(buffer, buffer_len, out_fmt, operand1, operand2, opcode_context->registers->Y, opcode_context->registers->X);
#else
    return 0;
#endif
}
