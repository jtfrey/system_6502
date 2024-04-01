
isa_6502_instr_stage_t
__isa_6502_DEC(
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
            break;
        
        case 2:
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
        
        case 3:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage:
                    ALU = memory_read(opcode_context->memory, ADDR);
                    break;
                case isa_6502_addressing_zeropage_x_indexed:
                    ADDR = (ADDR + opcode_context->registers->X) & 0x00FF;
                    break;
                case isa_6502_addressing_absolute:
                case isa_6502_addressing_absolute_x_indexed:
                    *ADDR_ptr = memory_read(opcode_context->memory, opcode_context->registers->PC++);
                    break;
            }
            break;
        
        case 4:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage:
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_zeropage_x_indexed:
                case isa_6502_addressing_absolute:
                    ALU = memory_read(opcode_context->memory, ADDR);
                    break;
                case isa_6502_addressing_absolute_x_indexed:
                    ADDR += opcode_context->registers->X;
                    break;
            }
            break;
        
        case 5:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage_x_indexed:
                case isa_6502_addressing_absolute:
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_absolute_x_indexed:
                    ALU = memory_read(opcode_context->memory, ADDR);
                    break;
            }
            break;
        
        case 6:
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    if ( at_stage == isa_6502_instr_stage_end) {
        ALU--;
        memory_write(opcode_context->memory, ADDR, ALU & 0x00FF);
        registers_status_with_value(opcode_context->registers, ALU & 0x00FF, registers_Carry_ignore);
    }
    return at_stage;
}

int
__isa_6502_disasm_DEC(
    isa_6502_instr_context_t    *opcode_context,
    char                        *buffer,
    int                         buffer_len
)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t                     value1, value2, operand1, operand2;
    const char                  *out_fmt = NULL;
    
    switch ( opcode_context->addressing_mode ) {
    
        case isa_6502_addressing_zeropage:
            value1 = memory_rcache_pop(opcode_context->memory);     /* Value in */
            value2 = memory_wcache_pop(opcode_context->memory);     /* Value out */
            operand1 = memory_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "DEC $%1$02hhX {$%5$02hhX - 1 = $%6$02hhX}";
            break;
        case isa_6502_addressing_zeropage_x_indexed:
            value1 = memory_rcache_pop(opcode_context->memory);     /* Value in */
            value2 = memory_wcache_pop(opcode_context->memory);     /* Value out */
            operand1 = memory_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "DEC $%1$02hhX,X[$%4$02hhX] {$%5$02hhX - 1 = $%6$02hhX}";
            break;
        case isa_6502_addressing_absolute:
            value1 = memory_rcache_pop(opcode_context->memory);     /* Value in */
            value2 = memory_wcache_pop(opcode_context->memory);     /* Value out */
            operand1 = memory_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = memory_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "DEC $%1$02hhX%2$02hhX {$%5$02hhX - 1 = $%6$02hhX}";
            break;
        case isa_6502_addressing_absolute_x_indexed:
            value1 = memory_rcache_pop(opcode_context->memory);     /* Value in */
            value2 = memory_wcache_pop(opcode_context->memory);     /* Value out */
            operand1 = memory_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = memory_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "DEC $%1$02hhX%2$02hhX,X[$%4$02hhX] {$%5$02hhX - 1 = $%6$02hhX}";
            break;
    }
    return snprintf(buffer, buffer_len, out_fmt, operand1, operand2, opcode_context->registers->A, opcode_context->registers->X,  value1, value2);
#else
    return 0;
#endif
}
