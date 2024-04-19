
ISA_65C02_INSTR(BRK)
{
    static uint8_t  DUMMY;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            /* BRK is essentially a zeropage instruction: */
            DUMMY = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
            /* Push PC[hi] */
            __isa_6502_push(opcode_context->registers, opcode_context->memory, (opcode_context->registers->PC & 0xFF00) >> 8);
            break;
        case 2:
            /* Push PC[lo] */
            __isa_6502_push(opcode_context->registers, opcode_context->memory, (opcode_context->registers->PC & 0x00FF));
            break;
        case 3:
            /* Push SR */
            __isa_6502_push(opcode_context->registers, opcode_context->memory, opcode_context->registers->SR | register_SR_Bit_B);
            break;
        case 4:
            /* Set Interrupt-disable flag */
            registers_SR_set_bit(opcode_context->registers, register_SR_Bit_I, 1);
            /* Clear BCD flag */
            registers_SR_set_bit(opcode_context->registers, register_SR_Bit_D, 0);
            break;
        case 5:
            /* Fetch program counter from NMI vector: */
            opcode_context->registers->PC = membus_read_addr(opcode_context->memory, MEMORY_ADDR_NMI_VECTOR);
            break;
        case 6:
            opcode_context->registers->PC |= membus_read_addr(opcode_context->memory, MEMORY_ADDR_NMI_VECTOR+1) << 8;
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}

ISA_65C02_STATIC_INSTR(BRK)
{
    uint8_t     DUMMY = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
    
    /* Push PC[hi] */
    __isa_6502_push(opcode_context->registers, opcode_context->memory, (opcode_context->registers->PC & 0xFF00) >> 8);
    /* Push PC[lo] */
    __isa_6502_push(opcode_context->registers, opcode_context->memory, (opcode_context->registers->PC & 0x00FF));
    /* Push SR */
    __isa_6502_push(opcode_context->registers, opcode_context->memory, opcode_context->registers->SR | register_SR_Bit_B);
    /* Set Interrupt-disable flag */
    registers_SR_set_bit(opcode_context->registers, register_SR_Bit_I, 1);
    /* Clear BCD flag */
    registers_SR_set_bit(opcode_context->registers, register_SR_Bit_D, 0);
    /* Fetch program counter from NMI vector: */
    opcode_context->registers->PC = membus_read_addr(opcode_context->memory, MEMORY_ADDR_NMI_VECTOR);
    opcode_context->registers->PC |= membus_read_addr(opcode_context->memory, MEMORY_ADDR_NMI_VECTOR+1) << 8;
    opcode_context->cycle_count += 6;
    return isa_6502_instr_stage_end;
}

ISA_65C02_DISASM(BRK)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t     value = membus_rcache_pop(opcode_context->memory);
    return snprintf(buffer, buffer_len, "BRK (=> $%04hX, MARKER=$%02hhX)", opcode_context->registers->PC, value);
#else
    return 0;
#endif
}
