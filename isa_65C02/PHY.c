
ISA_65C02_INSTR(PHY)
{
    static uint8_t Y;
    
    at_stage = isa_6502_instr_stage_next_cycle;
            
    switch ( opcode_context->cycle_count ) {
        case 1:
            Y = opcode_context->registers->Y;
            break;
        case 2:
            __isa_6502_push(opcode_context->registers, opcode_context->memory, Y);
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}

ISA_65C02_STATIC_INSTR(PHY)
{
    __isa_6502_push(opcode_context->registers, opcode_context->memory, opcode_context->registers->Y);
    opcode_context->cycle_count += 2;
    return isa_6502_instr_stage_end;
}

ISA_65C02_DISASM(PHY)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "PHY");
#else
    return 0;
#endif
}
