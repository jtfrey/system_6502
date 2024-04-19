
ISA_6502_INSTR(NMI)
{
    static uint16_t     ADDR16;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        
        case 1:
            __isa_6502_push(opcode_context->registers, opcode_context->memory, ((opcode_context->registers->PC & 0xFF00) >> 8));
            break;
        
        case 2:
            __isa_6502_push(opcode_context->registers, opcode_context->memory, (opcode_context->registers->PC & 0x00FF));
            break;
        
        case 3:
            __isa_6502_push(opcode_context->registers, opcode_context->memory, opcode_context->registers->SR);
            registers_SR_set_bit(opcode_context->registers, register_SR_Bit_I, 1);
            break;
        
        case 4:
            ADDR16 = membus_read_addr(opcode_context->memory, MEMORY_ADDR_NMI_VECTOR);
            break;
        
        case 5:
            ADDR16 |= membus_read_addr(opcode_context->memory, MEMORY_ADDR_NMI_VECTOR+1) << 8;
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    if ( at_stage == isa_6502_instr_stage_end) {
        opcode_context->registers->PC = ADDR16;
    }
    return at_stage;
}

ISA_6502_STATIC_INSTR(NMI)
{
    __isa_6502_push(opcode_context->registers, opcode_context->memory, ((opcode_context->registers->PC & 0xFF00) >> 8));
    __isa_6502_push(opcode_context->registers, opcode_context->memory, (opcode_context->registers->PC & 0x00FF));
    __isa_6502_push(opcode_context->registers, opcode_context->memory, opcode_context->registers->SR);
    registers_SR_set_bit(opcode_context->registers, register_SR_Bit_I, 1);
    opcode_context->registers->PC = membus_read_addr(opcode_context->memory, MEMORY_ADDR_NMI_VECTOR) | (membus_read_addr(opcode_context->memory, MEMORY_ADDR_NMI_VECTOR+1) << 8);
    opcode_context->cycle_count += 5;
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(NMI)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "NMI");
#else
    return 0;
#endif
}