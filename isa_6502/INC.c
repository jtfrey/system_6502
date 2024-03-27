
isa_6502_instr_stage_t
__isa_6502_INC(
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
                    ADDR += opcode_context->registers->X;
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
        ALU++;
        memory_write(opcode_context->memory, ADDR, ALU & 0x00FF);
        registers_status_with_value(opcode_context->registers, ALU & 0x00FF);
    }
    return at_stage;
}
