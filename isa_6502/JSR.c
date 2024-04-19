
ISA_6502_INSTR(JSR)
{
    static uint16_t ADDR16;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            /* Read low byte: */
            ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            break;
        case 2:
            /* Read high byte: */
            ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
            break;
        case 3:
            /* Push high byte of return address: */
            __isa_6502_push(opcode_context->registers, opcode_context->memory, (opcode_context->registers->PC & 0xFF00) >> 8);
            break;
        case 4:
            /* Push low byte of return address: */
            __isa_6502_push(opcode_context->registers, opcode_context->memory, (opcode_context->registers->PC & 0x00FF));
            break;
        case 5:
            /* Set PC: */
            opcode_context->registers->PC = ADDR16;
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}

ISA_6502_STATIC_INSTR(JSR)
{
    static uint16_t ADDR16;
    
    ADDR16 = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
    ADDR16 |= membus_read_addr(opcode_context->memory, opcode_context->registers->PC++) << 8;
    __isa_6502_push(opcode_context->registers, opcode_context->memory, (opcode_context->registers->PC & 0xFF00) >> 8);
    __isa_6502_push(opcode_context->registers, opcode_context->memory, (opcode_context->registers->PC & 0x00FF));
    opcode_context->registers->PC = ADDR16;
    opcode_context->cycle_count += 5;
    return isa_6502_instr_stage_end;
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
