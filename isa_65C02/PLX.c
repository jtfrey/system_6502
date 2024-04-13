
ISA_65C02_INSTR(PLX)
{
    static uint8_t X;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            /* Pop X */
            X = __isa_6502_pop(opcode_context->registers, opcode_context->memory);
            at_stage = isa_6502_instr_stage_next_cycle;
            break;
        case 2:
            opcode_context->registers->X = X;
            break;
        case 3:
            registers_did_set_X(opcode_context->registers, registers_Carry_ignore);
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}

ISA_65C02_DISASM(PLX)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "PLX");
#else
    return 0;
#endif
}
