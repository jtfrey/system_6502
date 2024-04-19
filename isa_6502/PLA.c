
ISA_6502_INSTR(PLA)
{
    static uint8_t A;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            /* Pop A */
            A = __isa_6502_pop(opcode_context->registers, opcode_context->memory);
            at_stage = isa_6502_instr_stage_next_cycle;
            break;
        case 2:
            opcode_context->registers->A = A;
            break;
        case 3:
            registers_did_set_A(opcode_context->registers, registers_Carry_ignore);
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}

ISA_6502_STATIC_INSTR(PLA)
{
    opcode_context->registers->A = __isa_6502_pop(opcode_context->registers, opcode_context->memory);
    registers_did_set_A(opcode_context->registers, registers_Carry_ignore);
    opcode_context->cycle_count += 3;
    return isa_6502_instr_stage_end;
}

ISA_6502_DISASM(PLA)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "PLA");
#else
    return 0;
#endif
}
