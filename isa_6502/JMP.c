
ISA_6502_INSTR(JMP)
{
    static uint16_t ADDR;
    static uint8_t *ADDR_ptr;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr = ((uint8_t*)&ADDR);
#else
            ADDR_ptr = ((uint8_t*)&ADDR) + 1;
#endif
            /* Read low byte: */
            *ADDR_ptr = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr++;
#else
            ADDR_ptr--;
#endif
            break;
        case 2:
            /* Read high byte: */
            *ADDR_ptr = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            if ( opcode_context->addressing_mode == isa_6502_addressing_absolute ) {
                opcode_context->registers->PC = ADDR;
                at_stage = isa_6502_instr_stage_end;
            }
            break;
        case 3:
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr = ((uint8_t*)&opcode_context->registers->PC);
#else
            ADDR_ptr = ((uint8_t*)&opcode_context->registers->PC) + 1;
#endif
            /* Read low byte: */
            *ADDR_ptr = membus_read_addr(opcode_context->memory, ADDR);
#ifndef DISABLE_JMP_PAGE_BOUNDARY_BUG
            if ( (ADDR & 0x00FF) == 0x00FF ) {
                ADDR &= 0xFF00;
            } else {
                ADDR++;
            }
#else
            ADDR++;
#endif
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr++;
#else
            ADDR_ptr--;
#endif

            at_stage = isa_6502_instr_stage_next_cycle;
            break;
        case 4:
            /* Read high byte: */
            *ADDR_ptr = membus_read_addr(opcode_context->memory, ADDR);
            opcode_context->registers->PC--;
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}

ISA_6502_DISASM(JMP)
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
            out_fmt = "JMP ($%3$02hhX%4$02hhX[$%1$02hhX%2$02hhX])";
            break;
    }
    return snprintf(buffer, buffer_len, out_fmt, operand1, operand2, operand3, operand4);
#else
    return 0;
#endif
}
