
isa_6502_instr_stage_t
__isa_6502_BIT(
    isa_6502_instr_context_t    *opcode_context,
    isa_6502_instr_stage_t      at_stage
)
{
    static uint16_t ADDR = 0x0000;
    static uint8_t  *ADDR_ptr;
    static uint8_t ALU;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            ADDR = 0x0000;
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr = ((uint8_t*)&ADDR);
#else
            ADDR_ptr = ((uint8_t*)&ADDR) + 1;
#endif
            *ADDR_ptr = memory_read(opcode_context->memory, opcode_context->registers->PC++);
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr++;
#else
            ADDR_ptr--;
#endif
            break;
        
        case 2:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage:
                    ALU = memory_read(opcode_context->memory, ADDR);
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_absolute:
                    *ADDR_ptr = memory_read(opcode_context->memory, opcode_context->registers->PC++);
                    break;
            }
            break;
        
        case 3:
            ALU = memory_read(opcode_context->memory, ADDR);
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    if ( at_stage == isa_6502_instr_stage_end) {
        uint8_t     A_and_M = (opcode_context->registers->A & ALU);
        
        opcode_context->registers->SR &= ~(register_SR_Bit_V | register_SR_Bit_Z | register_SR_Bit_N);
        opcode_context->registers->SR |= (ALU & 0xC0) | (A_and_M ? 0 : register_SR_Bit_Z);
    }
    return at_stage;
}

int
__isa_6502_disasm_BIT(
    isa_6502_instr_context_t    *opcode_context,
    char                        *buffer,
    int                         buffer_len
)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t                     value, operand1, operand2;
    const char                  *out_fmt = NULL;
    
    switch ( opcode_context->addressing_mode ) {
    
        case isa_6502_addressing_zeropage:
            value = memory_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = memory_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            return snprintf(buffer, buffer_len, "BIT $%02hhX {$%02hhX TEST $%02hhX => %hhu%hhu----%hhu-}",
                        operand1, opcode_context->registers->A, value,
                        registers_SR_get_bit(opcode_context->registers, register_SR_Bit_N),
                        registers_SR_get_bit(opcode_context->registers, register_SR_Bit_V),
                        registers_SR_get_bit(opcode_context->registers, register_SR_Bit_Z));
            break;
        case isa_6502_addressing_absolute:
            value = memory_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = memory_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = memory_rcache_pop(opcode_context->memory);   /* Target addr, low */
            return snprintf(buffer, buffer_len, "BIT $%02hhX%02hhX {$%02hhX TEST $%02hhX => %hhu%hhu----%hhu-}",
                        operand1, operand2, opcode_context->registers->A, value,
                        registers_SR_get_bit(opcode_context->registers, register_SR_Bit_N),
                        registers_SR_get_bit(opcode_context->registers, register_SR_Bit_V),
                        registers_SR_get_bit(opcode_context->registers, register_SR_Bit_Z));
            break;
    }
    return 0;
#else
    return 0;
#endif
}
