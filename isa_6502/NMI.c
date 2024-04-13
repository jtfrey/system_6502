
ISA_6502_INSTR(NMI)
{
    static uint16_t     ADDR;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        
        case 1:
            ADDR = 0x0000;
            __isa_6502_push(opcode_context->registers, opcode_context->memory, ((opcode_context->registers->PC & 0xFF00) >> 8));
            break;
        
        case 2:
            __isa_6502_push(opcode_context->registers, opcode_context->memory, (opcode_context->registers->PC & 0x00FF));
            break;
        
        case 3:
            __isa_6502_push(opcode_context->registers, opcode_context->memory, opcode_context->registers->SR | register_SR_Bit_B);
            break;
        
        case 4:
            ADDR = opcode_context->memory->nmi_vector;
            break;
        
        case 5:
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    if ( at_stage == isa_6502_instr_stage_end) {
        opcode_context->registers->PC = ADDR;
    }
    return at_stage;
}

ISA_6502_DISASM(NMI)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "NMI");
#else
    return 0;
#endif
}