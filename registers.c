
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
    the_registers->SR.BYTE = 0x00;
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
        "A:$%02hhX  X:$%02hhX  Y:$%02hhX  SP:$01%02hhX  PC:$%04hX  SR:$%02hhX [N:%d V:%d B:%d D:%d I:%d Z:%d C:%d]",
        the_registers->A,
        the_registers->X,
        the_registers->Y,
        the_registers->SP,
        the_registers->PC,
        the_registers->SR.BYTE,
        the_registers->SR.FIELDS.N, the_registers->SR.FIELDS.V, the_registers->SR.FIELDS.B, the_registers->SR.FIELDS.D, the_registers->SR.FIELDS.I, the_registers->SR.FIELDS.Z, the_registers->SR.FIELDS.C
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
        "A:$%02hhX  X:$%02hhX  Y:$%02hhX  SP:$01%02hhX  PC:$%04hX  SR:$%02hhX [N:%d V:%d B:%d D:%d I:%d Z:%d C:%d]\n",
        the_registers->A,
        the_registers->X,
        the_registers->Y,
        the_registers->SP,
        the_registers->PC,
        the_registers->SR.BYTE,
        the_registers->SR.FIELDS.N, the_registers->SR.FIELDS.V, the_registers->SR.FIELDS.B, the_registers->SR.FIELDS.D, the_registers->SR.FIELDS.I, the_registers->SR.FIELDS.Z, the_registers->SR.FIELDS.C
    );
}

//

#ifdef REGISTERS_DID_SET_ARE_FUNCTIONS

void registers_did_set_A(
    registers_t     *the_registers,
    int             carry_status
)
{
    the_registers->SR.FIELDS.Z = ( the_registers->A == 0 );
    the_registers->SR.FIELDS.N = ( (the_registers->A & 0x80) == 0x80 );
    if ( carry_status < registers_Carry_ignore ) the_registers->SR.FIELDS.C = carry_status;
}

//

void registers_did_set_X(
    registers_t     *the_registers,
    int             carry_status
)
{
    the_registers->SR.FIELDS.Z = ( the_registers->X == 0 );
    the_registers->SR.FIELDS.N = ( (the_registers->X & 0x80) == 0x80 );
    if ( carry_status < registers_Carry_ignore ) the_registers->SR.FIELDS.C = carry_status;
}

//

void registers_did_set_Y(
    registers_t     *the_registers,
    int             carry_status
)
{
    the_registers->SR.FIELDS.Z = ( the_registers->Y == 0 );
    the_registers->SR.FIELDS.N = ( (the_registers->Y & 0x80) == 0x80 );
    if ( carry_status < registers_Carry_ignore ) the_registers->SR.FIELDS.C = carry_status;
}

void
registers_status_with_value(
    registers_t     *the_registers,
    uint8_t         value
)
{
    the_registers->SR.FIELDS.Z = ( value == 0 );
    the_registers->SR.FIELDS.N = ( (value & 0x80) == 0x80 );
}

#endif

//

#ifdef ENABLE_REGISTERS_TEST

int
main()
{
    registers_t     cpu_registers;
    
    registers_init(&cpu_registers);
    registers_fprintf(&cpu_registers, stdout);
    
    cpu_registers.A = 0x00;
    registers_did_set_A(&cpu_registers, 0);
    registers_fprintf(&cpu_registers, stdout);
    
    cpu_registers.X = 0xFF;
    registers_did_set_X(&cpu_registers, 1);
    registers_fprintf(&cpu_registers, stdout);
    
    return 0;
}

#endif /* HAVE_REGISTERS_TEST */
