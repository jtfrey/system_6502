
ISA_65C02_INSTR(PLY)
{
    static uint8_t Y;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            /* Pop Y */
            Y = __isa_6502_pop(opcode_context->registers, opcode_context->memory);
            at_stage = isa_6502_instr_stage_next_cycle;
            break;
        case 2:
            opcode_context->registers->Y = Y;
            break;
        case 3:
            registers_did_set_Y(opcode_context->registers, registers_Carry_ignore);
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}

ISA_65C02_STATIC_INSTR(PLY)
{
    opcode_context->registers->Y = __isa_6502_pop(opcode_context->registers, opcode_context->memory);
    registers_did_set_Y(opcode_context->registers, registers_Carry_ignore);
    opcode_context->cycle_count += 3;
    return isa_6502_instr_stage_end;
}

ISA_65C02_DISASM(PLY)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "PLY");
#else
    return 0;
#endif
}
