
isa_6502_instr_stage_t
__isa_6502_LDX(
    isa_6502_instr_context_t    *opcode_context,
    isa_6502_instr_stage_t      at_stage
)
{
    static uint16_t ADDR = 0x0000, ADDR_pre_index;
    static uint8_t  *ADDR_ptr, *ALU_ptr;
    static uint16_t ALU, ALU_pre_index;
    
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
                case isa_6502_addressing_zeropage_y_indexed:
                    ADDR += opcode_context->registers->Y;
                    break;
                case isa_6502_addressing_absolute:
                case isa_6502_addressing_absolute_y_indexed:
                    *ADDR_ptr = memory_read(opcode_context->memory, opcode_context->registers->PC++);
                    break;
            }
            break;
        
        case 3:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_zeropage_y_indexed:
                case isa_6502_addressing_absolute:
                    ALU = memory_read(opcode_context->memory, ADDR);
                    at_stage = isa_6502_instr_stage_end;
                    break;
                case isa_6502_addressing_absolute_y_indexed:
                    ADDR_pre_index = ADDR;
                    ADDR += opcode_context->registers->Y;
                    if ( (ADDR_pre_index & 0xFF00) == (ADDR & 0xFF00) ) {
                        ALU = memory_read(opcode_context->memory, ADDR);
                        at_stage = isa_6502_instr_stage_end;
                    }
                    break;
            }
            break;
        
        case 4:
            switch ( opcode_context->addressing_mode ) {
                case isa_6502_addressing_absolute_y_indexed:
                    ALU = memory_read(opcode_context->memory, ADDR);
                    at_stage = isa_6502_instr_stage_end;
                    break;
            }
            break;
    }
    if ( at_stage == isa_6502_instr_stage_end) {
        opcode_context->registers->X = ALU;
        registers_did_set_X(opcode_context->registers, registers_Carry_ignore);
    }
    return at_stage;
}
