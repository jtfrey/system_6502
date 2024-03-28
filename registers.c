
#include "registers.h"

registers_t*
registers_alloc(void)
{
    return (registers_t*)malloc(sizeof(registers_t));
}

/**/

registers_t*
registers_init(
    registers_t     *the_registers
)
{
    if ( ! the_registers ) the_registers = registers_alloc();
    if ( the_registers ) registers_reset(the_registers);
    return the_registers;
}

//

void
registers_free(
    registers_t     *the_registers
)
{
    free((void*)the_registers);
}

//

void
registers_reset(
    registers_t     *the_registers
)
{
    the_registers->A = the_registers->X = the_registers->Y = 0x00;
    the_registers->SP = 0x00;
    the_registers->PC = 0xFFFC;
    the_registers->SR = 0x00;
}

//

int
registers_snprintf(
    registers_t     *the_registers,
    char            *p,
    size_t          p_len
)
{
    return snprintf(p, p_len,
        "A:$%02hhX  X:$%02hhX  Y:$%02hhX  SP:$01%02hhX  PC:$%04hX  SR:[N:%d V:%d B:%d D:%d I:%d Z:%d C:%d]",
        the_registers->A,
        the_registers->X,
        the_registers->Y,
        the_registers->SP,
        the_registers->PC,
        registers_SR_get_bit(the_registers, register_SR_Bit_N),
        registers_SR_get_bit(the_registers, register_SR_Bit_V),
        registers_SR_get_bit(the_registers, register_SR_Bit_B),
        registers_SR_get_bit(the_registers, register_SR_Bit_D),
        registers_SR_get_bit(the_registers, register_SR_Bit_IGN),
        registers_SR_get_bit(the_registers, register_SR_Bit_Z),
        registers_SR_get_bit(the_registers, register_SR_Bit_C)
    );
}

//

int
registers_fprintf(
    registers_t     *the_registers,
    FILE            *stream
)
{
    return fprintf(stream,
        "A:$%02hhX  X:$%02hhX  Y:$%02hhX  SP:$01%02hhX  PC:$%04hX  SR:[N:%d V:%d B:%d D:%d I:%d Z:%d C:%d]\n",
        the_registers->A,
        the_registers->X,
        the_registers->Y,
        the_registers->SP,
        the_registers->PC,
        registers_SR_get_bit(the_registers, register_SR_Bit_N),
        registers_SR_get_bit(the_registers, register_SR_Bit_V),
        registers_SR_get_bit(the_registers, register_SR_Bit_B),
        registers_SR_get_bit(the_registers, register_SR_Bit_D),
        registers_SR_get_bit(the_registers, register_SR_Bit_IGN),
        registers_SR_get_bit(the_registers, register_SR_Bit_Z),
        registers_SR_get_bit(the_registers, register_SR_Bit_C)
    );
}

//

#ifdef ENABLE_REGISTERS_TEST

int
main()
{
    registers_t     cpu_registers;
    
    registers_init(&cpu_registers);
    registers_fprintf(&cpu_registers, stdout);
    
    cpu_registers.A = 0x00;
    registers_did_set_A(&cpu_registers, registers_Carry_clear);
    registers_fprintf(&cpu_registers, stdout);
    
    cpu_registers.X = 0xFF;
    registers_did_set_X(&cpu_registers, registers_Carry_set);
    registers_fprintf(&cpu_registers, stdout);
    
    uint16_t        A = 0x0040;
    
    registers_SR_set_bit(&cpu_registers, register_SR_Bit_C, 0);
    
    A = A << 1;
    cpu_registers.A = A;
    registers_did_set_A(&cpu_registers, (((A & 0xFF00) != 0) ? registers_Carry_set : registers_Carry_clear));
    registers_fprintf(&cpu_registers, stdout);
    
    
    return 0;
}

#endif /* HAVE_REGISTERS_TEST */
