
isa_6502_instr_stage_t
__isa_6502_CPX(
    isa_6502_instr_context_t    *opcode_context,
    isa_6502_instr_stage_t      at_stage
)
{
    static uint16_t ADDR = 0x0000;
    static uint8_t  *ADDR_ptr;
    static uint16_t ALU;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            ADDR = 0x0000;
            if ( opcode_context->addressing_mode == isa_6502_addressing_immediate ) {
                ALU = memory_read(opcode_context->memory, opcode_context->registers->PC++);
                at_stage = isa_6502_instr_stage_end;
            } else {
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
            }
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
        uint8_t     flag_bits = 0x00;
        
        if ( opcode_context->registers->X > ALU )
            flag_bits |= register_SR_Bit_C;
        else if ( opcode_context->registers->X == ALU )
            flag_bits |= register_SR_Bit_C | register_SR_Bit_Z;
        else
            flag_bits |= register_SR_Bit_N;
        opcode_context->registers->SR = (opcode_context->registers->SR & ~(register_SR_Bit_C | register_SR_Bit_Z | register_SR_Bit_N)) | flag_bits;
    }
    return at_stage;
}

int
__isa_6502_disasm_CPX(
    isa_6502_instr_context_t    *opcode_context,
    char                        *buffer,
    int                         buffer_len
)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t                     value, operand1, operand2;
    const char                  *out_fmt = NULL;
    
    switch ( opcode_context->addressing_mode ) {
    
        case isa_6502_addressing_immediate:
            value = memory_rcache_pop(opcode_context->memory);      /* Value */
            out_fmt = "CPX #$%7$02hhX {$%4$02hhX == $%3$02hhX}";
            break;
        case isa_6502_addressing_zeropage:
            value = memory_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = memory_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "CPX $%1$02hhX {$%4$02hhX == $%3$02hhX}";
            break;
        case isa_6502_addressing_absolute:
            value = memory_rcache_pop(opcode_context->memory);      /* Value */
            operand1 = memory_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = memory_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "CPX $%1$02hhX%2$02hhX {$%4$02hhX == $%3$02hhX}";
            break;
    }
    return snprintf(buffer, buffer_len, out_fmt, operand1, operand2, opcode_context->registers->X, value);
#else
    return 0;
#endif
}
