
ISA_65C02_INSTR(JMP)
{
    static uint16_t ADDR16, INDIRECT;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            /* Read low byte: */
            ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            break;
        case 2:
            /* Read high byte: */
            ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
            if ( opcode_context->addressing_mode == isa_6502_addressing_absolute ) {
                at_stage = isa_6502_instr_stage_end;
            }
            break;
        case 3:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_indirect:
                    /* Read low byte: */
                    INDIRECT = membus_read_addr(opcode_context->memory, ADDR16++);
                    break;
                case isa_6502_addressing_absolute_x_indexed_indirect:
                    ADDR16 += opcode_context->registers->X;
                    break;
            }
            break;
        case 4:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_indirect:
                    /* Read high byte: */
                    ADDR16 = (membus_read_addr(opcode_context->memory, ADDR16) << 8) | INDIRECT;
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_absolute_x_indexed_indirect:
                    /* Read low byte: */
                    INDIRECT = membus_read_addr(opcode_context->memory, ADDR16++);
                    break;
            }
            break;
        case 5:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_absolute_x_indexed_indirect:
                    /* Read high byte: */
                    ADDR16 = (membus_read_addr(opcode_context->memory, ADDR16) << 8) | INDIRECT;
                    at_stage = isa_6502_instr_stage_end;
                    break;
            }
            break;
    }
    if ( at_stage == isa_6502_instr_stage_end ) opcode_context->registers->PC = ADDR16;
    return at_stage;
}

ISA_65C02_STATIC_INSTR(JMP)
{
    uint16_t    ADDR16;
    uint8_t     cycle_count;
    
    switch ( opcode_context->addressing_mode ) {
        case isa_6502_addressing_absolute:
            ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
            cycle_count = 2;
            break;
        case isa_6502_addressing_indirect: {
            uint16_t    INDIRECT;
            
            ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
            INDIRECT = membus_read_addr(opcode_context->memory, ADDR16++);
            ADDR16 = (membus_read_addr(opcode_context->memory, ADDR16) << 8) | INDIRECT;
            cycle_count = 4;
            break;
        }
        
        case isa_6502_addressing_absolute_x_indexed_indirect: {
            uint16_t    INDIRECT;
            
            ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
            ADDR16 += opcode_context->registers->X;
            INDIRECT = membus_read_addr(opcode_context->memory, ADDR16++);
            ADDR16 = (membus_read_addr(opcode_context->memory, ADDR16) << 8) | INDIRECT;
            cycle_count = 5;
            break;
        }
    }
    opcode_context->cycle_count += cycle_count;
    opcode_context->registers->PC = ADDR16;
    return isa_6502_instr_stage_end;
}

ISA_65C02_DISASM(JMP)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t                     operand1, operand2, operand3, operand4;
    const char                  *out_fmt = NULL;
    
    switch ( opcode_context->addressing_mode ) {
    
        case isa_6502_addressing_absolute:
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "JMP $%1$02hhX%2$02hhX";
            break;
        case isa_6502_addressing_indirect:
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            operand3 = membus_rcache_pop(opcode_context->memory);   /* Indirect addr, high */
            operand4 = membus_rcache_pop(opcode_context->memory);   /* Indirect addr, low */
            out_fmt = "JMP ($%3$02hhX%4$02hhX)[$%1$02hhX%2$02hhX]";
            break;
        case isa_6502_addressing_absolute_x_indexed_indirect:
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            operand3 = membus_rcache_pop(opcode_context->memory);   /* Indirect addr, high */
            operand4 = membus_rcache_pop(opcode_context->memory);   /* Indirect addr, low */
            out_fmt = "JMP ($%3$02hhX%4$02hhX,X[$%5$02hhX])[$%1$02hhX%2$02hhX]";
            break;
    }
    return snprintf(buffer, buffer_len, out_fmt, operand1, operand2, operand3, operand4, opcode_context->registers->X);
#else
    return 0;
#endif
}
