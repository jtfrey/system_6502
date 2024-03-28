
isa_6502_instr_stage_t
__isa_6502_RTI(
    isa_6502_instr_context_t    *opcode_context,
    isa_6502_instr_stage_t      at_stage
)
{
    static uint8_t  *PC_ptr;
    static uint8_t  ALU;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            /* Pop the SR */
            ALU = __isa_6502_pop(opcode_context->registers, opcode_context->memory);
            break;
        case 2:
            /* Clear the B/IGN flags */
            ALU &= ~(register_SR_Bit_B | register_SR_Bit_IGN);
            break;
        case 3:
            opcode_context->registers->SR = ALU;
            break;
        case 4:
            /* Prep for read: */
#ifdef ISA_6502_HOST_IS_LE
            PC_ptr = ((uint8_t*)&opcode_context->registers->PC);
#else
            PC_ptr = ((uint8_t*)&opcode_context->registers->PC) + 1;
#endif
            /* Pop low-byte of PC */
            *PC_ptr = __isa_6502_pop(opcode_context->registers, opcode_context->memory);
#ifdef ISA_6502_HOST_IS_LE
            PC_ptr++;
#else
            PC_ptr--;
#endif
            break;
        case 5:
            /* Pop high-byte of PC */
            *PC_ptr = __isa_6502_pop(opcode_context->registers, opcode_context->memory);
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}
