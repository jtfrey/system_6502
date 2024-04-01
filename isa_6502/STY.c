
isa_6502_instr_stage_t
__isa_6502_STY(
    isa_6502_instr_context_t    *opcode_context,
    isa_6502_instr_stage_t      at_stage
)
{
    static uint16_t ADDR = 0x0000;
    static uint8_t  *ADDR_ptr;
    
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
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_zeropage_x_indexed:
                    ADDR = (ADDR + opcode_context->registers->X) & 0x00FF;
                    break;
                case isa_6502_addressing_absolute:
                    *ADDR_ptr = memory_read(opcode_context->memory, opcode_context->registers->PC++);
                    break;
            }
            break;
        
        case 3:
            at_stage = isa_6502_instr_stage_end;
            break;
            
    }
    if ( at_stage == isa_6502_instr_stage_end) {        
        memory_write(opcode_context->memory, ADDR, opcode_context->registers->Y);
    }
    return at_stage;
}

int
__isa_6502_disasm_STY(
    isa_6502_instr_context_t    *opcode_context,
    char                        *buffer,
    int                         buffer_len
)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t                     operand1, operand2;
    const char                  *out_fmt = NULL;
    
    switch ( opcode_context->addressing_mode ) {
    
        case isa_6502_addressing_zeropage:
            operand1 = memory_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "STY $%1$02hhX {<= Y=$%4$02hhX}";
            break;
        case isa_6502_addressing_zeropage_x_indexed:
            operand1 = memory_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "STY $%1$02hhX,X[$%3$02hhX] {<= Y=$%4$02hhX}";
            break;
        case isa_6502_addressing_absolute:
            operand1 = memory_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = memory_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "STY $%1$02hhX%2$02hhX {<= Y=$%4$02hhX}";
            break;
    }
    return snprintf(buffer, buffer_len, out_fmt, operand1, operand2, opcode_context->registers->X, opcode_context->registers->Y);
#else
    return 0;
#endif
}
