
ISA_6502_INSTR(JSR)
{
    static uint16_t ADDR;
    static uint8_t *ADDR_ptr;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            /* Read low byte: */
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
            /* Read high byte: */
            *ADDR_ptr = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            break;
        case 3:
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr = ((uint8_t*)&opcode_context->registers->PC) + 1;
#else
            ADDR_ptr = ((uint8_t*)&opcode_context->registers->PC);
#endif
            /* Push PC[hi] */
            __isa_6502_push(opcode_context->registers, opcode_context->memory, *ADDR_ptr);
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr--;
#else
            ADDR_ptr++;
#endif
            break;
        case 4:
            /* Push PC[lo] */
            __isa_6502_push(opcode_context->registers, opcode_context->memory, *ADDR_ptr);
            break;
        case 5:
            /* Set PC to target address: */
            opcode_context->registers->PC = ADDR;
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}

ISA_6502_DISASM(JSR)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t                     operand1, operand2, operand3, operand4;
    const char                  *out_fmt = NULL;
    
    switch ( opcode_context->addressing_mode ) {
    
        case isa_6502_addressing_absolute:
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "JSR $%1$02hhX%2$02hhX";
            break;
    }
    return snprintf(buffer, buffer_len, out_fmt, operand1, operand2, operand3, operand4);
#else
    return 0;
#endif
}
