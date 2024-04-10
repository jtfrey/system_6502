
isa_6502_instr_stage_t
__isa_6502_STA(
    isa_6502_instr_context_t    *opcode_context,
    isa_6502_instr_stage_t      at_stage
)
{
    static uint16_t ADDR = 0x0000, ADDR2;
    static uint8_t  *ADDR_ptr, *ADDR2_ptr;
    
    at_stage = isa_6502_instr_stage_next_cycle;
    
    switch ( opcode_context->cycle_count ) {
        case 1:
            ADDR = 0x000;
#ifdef ISA_6502_HOST_IS_LE
            ADDR_ptr = ((uint8_t*)&ADDR);
#else
            ADDR_ptr = ((uint8_t*)&ADDR) + 1;
#endif
            *ADDR_ptr = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
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
                case isa_6502_addressing_absolute_x_indexed:
                case isa_6502_addressing_absolute_y_indexed:
                    *ADDR_ptr = membus_read_addr(opcode_context->memory, opcode_context->registers->PC++);
                    break;
                case isa_6502_addressing_x_indexed_indirect:
                case isa_6502_addressing_indirect_y_indexed:
                    break;
            }
            break;
        
        case 3:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage_x_indexed:
                case isa_6502_addressing_absolute:
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_absolute_x_indexed:
                    ADDR += opcode_context->registers->X;
                    break;
                case isa_6502_addressing_absolute_y_indexed:
                    ADDR += opcode_context->registers->Y;
                    break;
                case isa_6502_addressing_x_indexed_indirect:
                    ADDR += opcode_context->registers->X;
                case isa_6502_addressing_indirect_y_indexed:
                    ADDR2 = 0x0000;
#ifdef ISA_6502_HOST_IS_LE
                    ADDR2_ptr = ((uint8_t*)&ADDR2);
#else
                    ADDR2_ptr = ((uint8_t*)&ADDR2) + 1;
#endif
                    *ADDR2_ptr = membus_read_addr(opcode_context->memory, ADDR++);
#ifdef ISA_6502_HOST_IS_LE
                    ADDR2_ptr++;
#else
                    ADDR2_ptr--;
#endif
                    break;
            }
            break;
        
        case 4:
            switch ( opcode_context->addressing_mode ) {\
                case isa_6502_addressing_absolute_x_indexed:
                case isa_6502_addressing_absolute_y_indexed:
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_x_indexed_indirect:
                    *ADDR2_ptr = membus_read_addr(opcode_context->memory, ADDR);
                    break;
                case isa_6502_addressing_indirect_y_indexed:
                    *ADDR2_ptr = membus_read_addr(opcode_context->memory, ADDR);
                    ADDR2 += opcode_context->registers->Y;
                    break;
            }
            break;
        
        case 5:
            ADDR = ADDR2;
            at_stage = isa_6502_instr_stage_end;
            break;
    }
    if ( at_stage == isa_6502_instr_stage_end) {
        membus_write_addr(opcode_context->memory, ADDR, opcode_context->registers->A);
    }
    return at_stage;
}

int
__isa_6502_disasm_STA(
    isa_6502_instr_context_t    *opcode_context,
    char                        *buffer,
    int                         buffer_len
)
{
#ifdef ENABLE_DISASSEMBLY
    uint8_t                     value, operand1, operand2, operand3;
    const char                  *out_fmt = NULL;
    
    switch ( opcode_context->addressing_mode ) {
    
        case isa_6502_addressing_zeropage:
            value = membus_wcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "STA $%1$02hhX {<= $%6$02hhX}";
            break;
        case isa_6502_addressing_zeropage_x_indexed:
            value = membus_wcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "STA $%1$02hhX,X {<= $%6$02hhX}";
            break;
        case isa_6502_addressing_absolute:
            value = membus_wcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "STA $%1$02hhX%2$02hhX {<= $%6$02hhX}";
            break;
        case isa_6502_addressing_absolute_x_indexed:
            value = membus_wcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "STA $%1$02hhX%2$02hhX,X[$%4$02hhX] {<= $%6$02hhX}";
            break;
        case isa_6502_addressing_absolute_y_indexed:
            value = membus_wcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            out_fmt = "STA $%1$02hhX%2$02hhX,Y[$%5$02hhX] {<= $%6$02hhX}";
            break;
        case isa_6502_addressing_x_indexed_indirect:
            value = membus_wcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            operand3 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "STA ($%3$02hhX=$%1$02hhX%2$02hhX,X[$%4$02hhX]) {<= $%6$02hhX}";
            break;
        case isa_6502_addressing_indirect_y_indexed:
            value = membus_wcache_pop(opcode_context->memory);      /* Value */
            operand1 = membus_rcache_pop(opcode_context->memory);   /* Target addr, high */
            operand2 = membus_rcache_pop(opcode_context->memory);   /* Target addr, low */
            operand3 = membus_rcache_pop(opcode_context->memory);   /* Zero-page addr */
            out_fmt = "STA ($%3$02hhX[$%1$02hhX%2$02hhX]),Y[$%5$02hhX] {<= $%6$02hhX}";
            break;
    }
    return snprintf(buffer, buffer_len, out_fmt, operand1, operand2, operand3, opcode_context->registers->X, opcode_context->registers->Y, value);
#else
    return 0;
#endif
}
