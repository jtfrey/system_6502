
isa_6502_instr_stage_t
__isa_6502_BRK(
    isa_6502_instr_context_t    *opcode_context,
    isa_6502_instr_stage_t      at_stage
)
{
    static uint16_t PC;
    static uint8_t *PC_ptr;
    static uint8_t SR;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            /* Add 2 to PC */
            PC = opcode_context->registers->PC + 2;
#ifdef ISA_6502_HOST_IS_LE
            PC_ptr = ((uint8_t*)&PC) + 1;
#else
            PC_ptr = ((uint8_t*)&PC);
#endif
            /* Push PC[hi] */
            __isa_6502_push(opcode_context->registers, opcode_context->memory, *PC_ptr);    
#ifdef ISA_6502_HOST_IS_LE
            PC_ptr--;
#else
            PC_ptr++;
#endif
            break;
        case 2:
            /* Push PC[lo] */
            __isa_6502_push(opcode_context->registers, opcode_context->memory, *PC_ptr);
            break;
        case 3:
            /* Push SR */
            __isa_6502_push(opcode_context->registers, opcode_context->memory, opcode_context->registers->SR | register_SR_Bit_B);
            break;
        case 4:
            /* Set Interrupt-disable flag */
            registers_SR_set_bit(opcode_context->registers, register_SR_Bit_I, 1);
            break;
        case 5:
            /* Fetch program counter from NMI vector: */
#ifdef ISA_6502_HOST_IS_LE
            PC_ptr = ((uint8_t*)&opcode_context->registers->PC) + 1;
#else
            PC_ptr = ((uint8_t*)&opcode_context->registers->PC);
#endif
            *PC_ptr = membus_read_addr(opcode_context->memory, MEMORY_ADDR_NMI_VECTOR);
#ifdef ISA_6502_HOST_IS_LE
            PC_ptr--;
#else
            PC_ptr++;
#endif
            break;
        case 6:
            *PC_ptr = membus_read_addr(opcode_context->memory, MEMORY_ADDR_NMI_VECTOR + 1);
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    return at_stage;
}

int
__isa_6502_disasm_BRK(
    isa_6502_instr_context_t    *opcode_context,
    char                        *buffer,
    int                         buffer_len
)
{
#ifdef ENABLE_DISASSEMBLY
    return snprintf(buffer, buffer_len, "BRK (=> $%04hX)", opcode_context->registers->PC);
#else
    return 0;
#endif
}
