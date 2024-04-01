
isa_6502_instr_stage_t
__isa_6502_PHP(
    isa_6502_instr_context_t    *opcode_context,
    isa_6502_instr_stage_t      at_stage
)
{
    static uint8_t SR;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            SR = opcode_context->registers->SR | register_SR_Bit_B | register_SR_Bit_IGN;
            break;
        case 2:
            __isa_6502_push(opcode_context->registers, opcode_context->memory, SR);
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}

int
__isa_6502_disasm_PHP(
    isa_6502_instr_context_t    *opcode_context,
    char                        *buffer,
    int                         buffer_len
)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "PHP");
#else
    return 0;
#endif
}
