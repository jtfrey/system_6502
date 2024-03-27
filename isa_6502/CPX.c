
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
        opcode_context->registers->SR.BYTE = (opcode_context->registers->SR.BYTE & ~(register_SR_Bit_C | register_SR_Bit_Z | register_SR_Bit_N)) | flag_bits;
    }
    return at_stage;
}
